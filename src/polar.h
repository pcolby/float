#include <QObject>
#include <QUrl>
#include <QWebEnginePage>

#define USE_WEB_ENGINE_VIEW

#ifdef USE_WEB_ENGINE_VIEW
#include <QWebEngineView>
#endif

class NonInteractiveWebPage;

class Polar : public QObject
{
    Q_OBJECT

public:
    explicit Polar(const QString &username, const QString &password, QObject * parent = Q_NULLPTR);
    virtual ~Polar();

public slots:
    void setWeight(const double mass);

protected:
    static QString javaScriptLiteral(QString string, QChar quote = QChar());

protected slots:
    void onLoadFinshed(const bool ok);

private:
    NonInteractiveWebPage * page;
#ifdef USE_WEB_ENGINE_VIEW
    QWebEngineView * view;
#endif
    QString username;
    QString password;
    double mass;

};
