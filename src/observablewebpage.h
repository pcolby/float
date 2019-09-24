#include <QJsonObject>
#include <QWebEnginePage>

#include "noninteractivewebpage.h"

class ObservableWebPage : public NonInteractiveWebPage
{
    Q_OBJECT

public:
    struct ObserveOptions {
        bool childList;
        bool attributes;
        bool characterData;
        bool subtree;
        bool attributeOldValue;
        bool characterDataOldValue;
        QStringList attributeFilter;
        ObserveOptions() {
            childList = attributes = characterData = subtree =
                    attributeOldValue = characterDataOldValue = false;
        }
    };

    explicit ObservableWebPage(QObject * parent = Q_NULLPTR);
    ObservableWebPage(QWebEngineProfile * profile, QObject * parent = Q_NULLPTR);
    ~ObservableWebPage() override;

    void observe(const QString &target, const ObserveOptions &options);
    void observe(const QString &target, const QString &options);

    // Convenience methods that simply call the above observe methods.
    void observeByClassName(const QString &className, const ObserveOptions &options = ObserveOptions());
    void observeById(const QString &Id, const ObserveOptions &options = ObserveOptions());
    void observeByName(const QString &name, const ObserveOptions &options = ObserveOptions());
    void observeBySelector(const QString &selector, const ObserveOptions &options = ObserveOptions());
    void observeByTagName(const QString &tagName, const ObserveOptions &options = ObserveOptions());
    void observeByTagNameNS(const QString &namespaceUri, const QString &localName,
                                   const ObserveOptions &options = ObserveOptions());

protected:
    // Base class overrides.

    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                  int lineNumber, const QString &sourceID) override;

private:
    const QString observerVarName;

signals:
    void mutationObserved(const QJsonObject &mutation);

};
