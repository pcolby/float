#include <QWebEnginePage>

class NonInteractiveWebPage : public QWebEnginePage
{

public:
    explicit NonInteractiveWebPage(QObject * parent = Q_NULLPTR);
    NonInteractiveWebPage(QWebEngineProfile * profile, QObject * parent = Q_NULLPTR);
    ~NonInteractiveWebPage() override;

protected:

    // Base class overrides.
    QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles,
                            const QStringList &acceptedMimeTypes) override;
    void javaScriptAlert(const QUrl &securityOrigin, const QString &msg) override;
    bool javaScriptConfirm(const QUrl &securityOrigin, const QString &msg) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                  int lineNumber, const QString &sourceID) override;
    bool javaScriptPrompt(const QUrl &securityOrigin, const QString &msg,
                          const QString &defaultValue, QString *result) override;

};
