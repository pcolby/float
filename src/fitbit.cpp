/*
    Copyright 2019 Paul Colby <git@colby.id.au>

    This file is part of Float.

    Float is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Float is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Float.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QDebug>
#include <QWebEngineProfile>
#include <QWebEngineScript>

#include <QTimer>

#include "fitbit.h"
#include "observablewebpage.h"

#define FITBIT_WEIGHT_URL QStringLiteral("https://www.fitbit.com/weight")

Fitbit::Fitbit(const QString &username, const QString &password, QObject * parent)
    :  QObject(parent), username(username), password(password), scriptBusy(false)
{
    // Create a new web page, with an 'anonymous' profile (unshared, in-memory cookies, etc).
    page = new ObservableWebPage(new QWebEngineProfile, this);
    page->profile()->setParent(this); // Done post-construction to ensure deletion *after* page.
    Q_ASSERT(page->profile()->isOffTheRecord());

#ifdef USE_WEB_ENGINE_VIEW
    // Create a web engine view (if we're using one), and assign our custom web page.
    view = new QWebEngineView();
    view->setPage(page);
#endif

    connect(page, &ObservableWebPage::loadFinished, this, &Fitbit::onLoadFinshed);
    connect(page, &ObservableWebPage::mutationObserved, this, &Fitbit::onMutation);
}

Fitbit::~Fitbit()
{
#ifdef USE_WEB_ENGINE_VIEW
    delete view;
#endif
    delete page;
}

// Public Slots

void Fitbit::fetchWeight()
{
#ifdef USE_WEB_ENGINE_VIEW
    Q_ASSERT(view);
    view->load(FITBIT_WEIGHT_URL);
    view->show();
#else
    page->load(FITBIT_WEIGHT_URL);
#endif
}

// Protected Methods

float Fitbit::parseBodyFat(const QString &string)
{
    // Fitbit.com examples: "23% Fat", "22.6% Fat".
    return string.split(QLatin1Char('%')).first().toFloat();
}

QDateTime Fitbit::parseDate(const QString &string)
{
    // Fitbit.com examples:
    // "Today - 8:41 AM"
    // "Mon - 9:15 PM"
    // "19/09/2019 - 8:31 AM"
    // "09/05/2019 - 7:08 PM"

    // Handle 'Today - h:mm (A|P)M' format.
    const QTime time = QTime::fromString(string, QStringLiteral("'Today - 'h:mm AP"));
    if (time.isValid()) {
        qDebug() << time << "isValid";
        return QDateTime(QDate::currentDate(), time);
    }

    // Handle '(Mon|Tue|Wed|Thu|Fri|Sat|Sun) - h:mm (A|P)M' format.
    const QStringList parts = string.split(QStringLiteral(" - "));
    if ((parts.length() == 2) && (parts[0].length() == 3)) {
        return QDateTime(
            QDate::currentDate().addDays(
                QDate::fromString(parts[0], QStringLiteral("ddd")).dayOfWeek() -
                QDate::currentDate().dayOfWeek()
            ),
            QTime::fromString(parts[1].trimmed(), QStringLiteral("h:mm AP"))
        );
    }

    // Handle 'dd/MM/yyyy - h:mm AP' format.
    const QDateTime date = QDateTime::fromString(string, QStringLiteral("dd/MM/yyyy - h:mm AP"));
    if (date.isValid()) {
        return date;
    }

    qWarning() << "Failed to parse date string:" << string;
    return QDateTime();
}

float Fitbit::parseWeigth(const QString &string)
{
    // Fitbit.com examples: "79.3 kg", "80.4 kg", "78 kg"
    return string.split(QLatin1Char(' ')).first().toFloat();
}

QString Fitbit::javaScriptLiteral(QString string, QChar quote)
{
    // If not specified, choose the most efficient quote character.
    if (quote.isNull()) {
        quote = (string.count(QLatin1Char('\'')) <= string.count(QLatin1Char('"')))
            ? QLatin1Char('\'') : QLatin1Char('"');
    }
    Q_ASSERT((quote == QLatin1Char('"')) || (quote == QLatin1Char('\'')));

    // Note, we could escape a lot here (indeed, every char if we wanted to), but ES only requires
    // backslash, quote (the one being used), and line-terminators (which ES defines as CR, LF,
    // LS and PS).
    return quote + QString(string)
        .replace(QLatin1Char('\\'),         QStringLiteral("\\\\"))
        .replace(QChar::CarriageReturn,     QStringLiteral("\\")+QChar::CarriageReturn)
        .replace(QChar::LineFeed,           QStringLiteral("\\")+QChar::LineFeed)
        .replace(QChar::LineSeparator,      QStringLiteral("\\")+QChar::LineSeparator)
        .replace(QChar::ParagraphSeparator, QStringLiteral("\\")+QChar::ParagraphSeparator)
        .replace(quote,                     QStringLiteral("\\")+quote)
        + quote;
}

// Protected Slots

void Fitbit::nextStep()
{
    if (scriptBusy) {
        qDebug() << "JavasScript is busy; will try again soon";
        return;
    }

    scriptBusy = true;
    page->runJavaScript(
        QStringLiteral(R"JS(
            function nextStep() {
                const pageLoadingScreen = document.getElementById('pageLoadingScreen');
                if ((pageLoadingScreen) && (pageLoadingScreen.classList.contains('loading'))) {
                    console.trace('Page still loading');
                    return false;
                }

                const loginForm = document.getElementById('loginForm');
                if (loginForm) {
                    console.info('QtInfoMsg: Logging into Fitbit');
                    const email = document.querySelector('#email-input input');
                    email.value = %1;
                    email.dispatchEvent(new CustomEvent('blur'));
                    const pass = document.querySelector('#password-input input');
                    pass.value = %2;
                    pass.dispatchEvent(new CustomEvent('blur'));
                    document.querySelector('#loginForm button').click();
                    return true;
                }

                const weightListItem = document.querySelector('.weight-list-item');
                if (weightListItem) {
                    console.trace('Reading weight item');
                    result = {
                        date: weightListItem.querySelector('.weight-list-item-date-text').innerText,
                        bodyFat: weightListItem.querySelector('.weight-list-item-stats .body-fat-text').innerText,
                        weight: weightListItem.querySelector('.weight-list-item-stats.body-weight').innerText,
                    }
                    console.debug(JSON.stringify(result));
                    return result;
                } else {
                    console.trace("Didn't find anything useful; will wait");
                }
            }
            try {
                nextStep();
            } catch(error) {
               console.debug(error.toString());
               const result = { error: { name: error.name, message: error.message } };
               result;
            }
        )JS").arg(javaScriptLiteral(username), javaScriptLiteral(password)),
        QWebEngineScript::ApplicationWorld, [this](const QVariant &result) {
            qDebug() << "JavaScript result" << result;

            // Stop on errors.
            const QVariantMap error = result.toMap().value(QStringLiteral("error")).toMap();
            if (!error.isEmpty()) {
                qCritical().noquote() << error.value(QLatin1String("name")).toString()
                                      << error.value(QLatin1String("message")).toString();
                QCoreApplication::exit(EXIT_FAILURE);
                return;
            }

            const QVariantMap map = result.toMap();
            if (!map.isEmpty()) {
                const QDateTime date = parseDate(map.value(QLatin1String("date")).toString());
                const float bodyFat = parseBodyFat(map.value(QLatin1String("bodyFat")).toString());
                const float weight = parseWeigth(map.value(QLatin1String("weight")).toString());
                qDebug() << "Found weight:" << date << bodyFat << weight;
                if (date.daysTo(QDateTime::currentDateTime()) > 7) {
                    qWarning() << "Weight date is too old:" << date;
                    QCoreApplication::exit(EXIT_FAILURE);
                }
                emit weightFound(weight);
            }

            scriptBusy = false;
        }
    );
}

void Fitbit::onLoadFinshed(const bool ok)
{
    qDebug() << "Finished loading" << page->url().toString() << ok;

    // Check the web page was loaded successfully.
    if (!ok) {
        qWarning() << "Failed to load" << page->url().toString();
        return;
    }

    // Begin observing changes to the page's body (only top-level children).
    ObservableWebPage::ObserveOptions options;
    options.childList = true;
    page->observeByTagName(QStringLiteral("'body'"), options);

    // Move the browser to the next step.
    nextStep();
}

void Fitbit::onMutation(const QJsonObject &mutation)
{
    Q_UNUSED(mutation)
    nextStep();
}
