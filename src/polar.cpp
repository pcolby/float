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

#include "polar.h"
#include "noninteractivewebpage.h"

#define FLOW_SETTINGS_URL QStringLiteral("https://flow.polar.com/settings")

Polar::Polar(const QString &username, const QString &password, QObject * parent)
    :  QObject(parent), username(username), password(password)
{
    // Create a new web page, with an 'anonymous' profile (unshared, in-memory cookies, etc).
    page = new NonInteractiveWebPage(new QWebEngineProfile, this);
    page->profile()->setParent(this); // Done post-construction to ensure deletion *after* page.
    Q_ASSERT(page->profile()->isOffTheRecord());

#ifdef USE_WEB_ENGINE_VIEW
    // Create a web engine view (if we're using one), and assign our custom web page.
    view = new QWebEngineView();
    view->setPage(page);
#endif

    connect(page, &NonInteractiveWebPage::loadFinished, this, &Polar::onLoadFinshed);
}

Polar::~Polar()
{
#ifdef USE_WEB_ENGINE_VIEW
    delete view;
#endif
    delete page;
}

// Public Slots

void Polar::setWeight(const double mass)
{
    qDebug() << "Setting weight to" << mass << "kg";

    // Sanity check my weight range (yes, this is just for me ;)
    if ((60 >= mass) || (mass >= 90)) {
        qWarning() << "Invalid mass:" << mass;
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }
    this->mass = mass;

#ifdef USE_WEB_ENGINE_VIEW
    Q_ASSERT(view);
    view->load(FLOW_SETTINGS_URL);
    view->show();
#else
    page->load(FLOW_SETTINGS_URL);
#endif
}

// Protected Methods

QString Polar::javaScriptLiteral(QString string, QChar quote)
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

void Polar::onLoadFinshed(const bool ok)
{
    qDebug() << "Finished loading" << page->url().toString() << ok;

    // Check the webpage was loaded successfully.
    if (!ok) {
        qWarning() << "Failed to load" << page->url().toString();
        return;
    }

    // Login and/or update the weight.
    page->runJavaScript(
        QStringLiteral(R"JS(
            function nextStep() {
                const loginForm = document.getElementById('loginForm');
                if (loginForm) {
                    console.info('QtInfoMsg: Logging into Polar Flow');
                    const email = document.getElementById('email');
                    email.value = %1;
                    const pass = document.getElementById('password');
                    pass.value = %2;
                    const login = document.getElementById('login');
                    login.click();
                    return true; // Keep running.
                }

                const weight = document.getElementById('weight');
                if (weight) {
                    if (weight.value == %3) {
                        console.info(`QtInfoMsg: Weight is already ${weight.value} (%3)`);
                        return false; // Time to exit the app.
                    }
                    console.info(`QtInfoMsg: Updating weight from ${weight.value} to %3`);
                    weight.value = %3;
                    document.getElementById('save-account-btn').click();
                }
            }
            try {
                nextStep();
            } catch(error) {
               console.debug(error.toString());
               const result = { error: { name: error.name, message: error.message } };
               result;
            }
        )JS").arg(javaScriptLiteral(username), javaScriptLiteral(password)).arg(mass),
        QWebEngineScript::ApplicationWorld, [](const QVariant &result) {
            qDebug() << "JavaScript result" << result;

            // Stop on errors.
            const QVariantMap error = result.toMap().value(QStringLiteral("error")).toMap();
            if (!error.isEmpty()) {
                qCritical().noquote() << error.value(QLatin1String("name")).toString()
                                      << error.value(QLatin1String("message")).toString();
                QCoreApplication::exit(EXIT_FAILURE);
                return;
            }

            // Stop on 'false'.
            if ((result.type() == QVariant::Bool) && (!result.toBool())) {
                QCoreApplication::exit(EXIT_SUCCESS); // We're done :)
            }
        }
    );
}
