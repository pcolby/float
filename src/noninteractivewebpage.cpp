#include <QDebug>

#include "noninteractivewebpage.h"

NonInteractiveWebPage::NonInteractiveWebPage(QObject * parent) : QWebEnginePage(parent)
{

}

NonInteractiveWebPage::NonInteractiveWebPage(QWebEngineProfile * profile, QObject * parent)
    : QWebEnginePage(profile, parent)
{

}

NonInteractiveWebPage::~NonInteractiveWebPage()
{

}

// Base class overrides.

QStringList NonInteractiveWebPage::chooseFiles(FileSelectionMode mode, const QStringList &oldFiles,
                        const QStringList &acceptedMimeTypes)
{
    // Base implementation would invoke QFileDialog::get...() according to the value of mode.
    qWarning() << "Refusing JavaScript file selection request";
    qDebug() << mode << oldFiles << acceptedMimeTypes;
    return QStringList(); // Don't select any files.
}

void NonInteractiveWebPage::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    // Base implementation would invoke QMessageBox::information().
    qWarning() << "JavaScript alert" << msg;
    qDebug() << securityOrigin;
}

bool NonInteractiveWebPage::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
{
    // Base implementation would invoke QMessageBox::information().
    qWarning() << "Refusing JavaScript confirm" << msg;
    qDebug() << securityOrigin;
    return false; // We do not confirm whatever JavaScript confirm() call wants.
}

void NonInteractiveWebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                                 const QString &message, int lineNumber,
                                                 const QString &sourceID)
{
    QWebEnginePage::javaScriptConsoleMessage(level, message, lineNumber, sourceID);
    // A small hack to allow info messages to be logged, since Chromium's API doesn't allow Qt to
    // differentiate between debug and info messages in JavaScript (both are 'Info' level).
    if (message.startsWith(QStringLiteral("QtInfoMsg:"))) {
        qInfo().noquote() << message.mid(10).trimmed();
    }
}

bool NonInteractiveWebPage::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg,
                      const QString &defaultValue, QString *result)
{
    // Base implementation would invoke QInputDialog::getText().
    Q_UNUSED(result)
    qWarning() << "Refusing JavaScript prompt" << msg << defaultValue;
    qDebug() << securityOrigin;
    return false; // Return false to indicate that the 'user' cancelled the JavaScript prompt.
}
