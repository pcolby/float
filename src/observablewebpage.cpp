#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUuid>
#include <QWebEngineScript>

#include "observablewebpage.h"

ObservableWebPage::ObservableWebPage(QObject * parent) : NonInteractiveWebPage(parent),
    observerVarName(QStringLiteral("observer_%1_").arg(QUuid::createUuid().toString(QUuid::Id128)))
{

}

ObservableWebPage::ObservableWebPage(QWebEngineProfile * profile, QObject * parent)
    : NonInteractiveWebPage(profile, parent),
      observerVarName(QStringLiteral("observer_%1_").arg(QUuid::createUuid().toString(QUuid::Id128)))
{

}

ObservableWebPage::~ObservableWebPage()
{

}

void ObservableWebPage::observe(const QString &target, const ObserveOptions &options)
{
    QJsonObject optionsJson {
        { QStringLiteral("childList"), options.childList },
        { QStringLiteral("attributes"), options.attributes },
        { QStringLiteral("characterData"), options.characterData },
        { QStringLiteral("subtree"), options.subtree },
        { QStringLiteral("attributeOldValue"), options.attributeOldValue },
        { QStringLiteral("characterDataOldValue"), options.characterDataOldValue },
    };
    if (!options.attributeFilter.isEmpty()) {
        optionsJson.insert(QStringLiteral("attributeFilter"),
                           QJsonArray::fromStringList(options.attributeFilter));
    }
    const QString optionsString =
        QString::fromUtf8(QJsonDocument(optionsJson).toJson(QJsonDocument::Compact));
    observe(target, optionsString);
}

void ObservableWebPage::observe(const QString &target, const QString &options)
{
    // Setup the observer (if not already), and begin observing the target.
    const QString script = QStringLiteral(R"JS(
        try {
            if (typeof %1 === 'undefined') {
                console.debug('Constructing mutation observer "%1"');
                %1 = new MutationObserver(function (mutationList, observer) {
                    mutationList.forEach((mutation) => {
                        var object = {
                            type: mutation.type,
                            target: mutation.target,
                            addedNodes: mutation.addedNodes,
                            removedNodes: mutation.removedNodes,
                            previousSibling: mutation.previousSibling,
                            nextSibling: mutation.nextSibling,
                            oldValue: mutation.oldValue
                        }
                        console.log('%1' + JSON.stringify(object));
                    })
                });
            }
            console.log('Observing ' + %2);
            %1.observe(%2%3%4);
        } catch(error) {
            console.info(error.toString());
            const result = { error: { name: error.name, message: error.message } };
            result;
        }
    )JS").arg(observerVarName, target, QString::fromLatin1(options.isEmpty() ? "" : ", "), options);

    // Execute the script.
    qDebug().noquote() << "Running JavaScript" << script;
    runJavaScript(script, QWebEngineScript::ApplicationWorld, [](const QVariant &result) {
        qDebug() << "Observation request result" << result;
        const QVariantMap error = result.toMap().value(QStringLiteral("error")).toMap();
        if (!error.isEmpty()) {
            qWarning().noquote() << error.value(QLatin1String("name")).toString()
                                 << error.value(QLatin1String("message")).toString();
        }
        /// @todo We could emit a signal with the result.
    });
}

void ObservableWebPage::observeById(const QString &Id, const ObserveOptions &options)
{
    qDebug() << "observing" << Id;
    observe(QStringLiteral("document.getElementById(%1)").arg(Id), options);
}

void ObservableWebPage::observeByClassName(const QString &className, const ObserveOptions &options)
{
    observe(QStringLiteral("document.getElementsByClassName(%1)[0]").arg(className), options);
}

void ObservableWebPage::observeByName(const QString &name, const ObserveOptions &options)
{
    observe(QStringLiteral("document.getElementsByClassName(%1)[0]").arg(name), options);
}

void ObservableWebPage::observeBySelector(const QString &selector, const ObserveOptions &options)
{
    observe(QStringLiteral("document.querySelector(%1)").arg(selector), options);
}

void ObservableWebPage::observeByTagName(const QString &tagName, const ObserveOptions &options)
{
    observe(QStringLiteral("document.getElementsByTagName(%1)[0]").arg(tagName), options);
}

void ObservableWebPage::observeByTagNameNS(const QString &namespaceUri, const QString &localName,
                                           const ObserveOptions &options)
{
    observe(QStringLiteral("document.getElementsByTagNameNS(%1, %2)[0]")
            .arg(namespaceUri, localName), options);
}

// Base class overrides.

void ObservableWebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                                 const QString &message, int lineNumber,
                                                 const QString &sourceID)
{
    qDebug() << level << sourceID << lineNumber << message;

    if (message.startsWith(observerVarName)) {
        QJsonParseError error;
        const QJsonDocument mutation =
            QJsonDocument::fromJson(message.mid(observerVarName.length()).toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning().noquote() << "Failed to parse mutation object from" << message;
            qInfo() << error.errorString();
            return;
        }
        qDebug().noquote() << mutation.toJson();
        emit mutationObserved(mutation.object());
        return;
    }

    NonInteractiveWebPage::javaScriptConsoleMessage(level, message, lineNumber, sourceID);
}
