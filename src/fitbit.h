#include <QObject>
#include <QUrl>
#include <QWebEnginePage>

#define USE_WEB_ENGINE_VIEW

#ifdef USE_WEB_ENGINE_VIEW
#include <QWebEngineView>
#endif

class ObservableWebPage;

class Fitbit : public QObject
{
    Q_OBJECT

public:
    explicit Fitbit(const QString &username, const QString &password, QObject * parent = Q_NULLPTR);
    virtual ~Fitbit();

public slots:
    void fetchWeight();

protected:
    static QString javaScriptLiteral(QString string, QChar quote = QChar());
    static float parseBodyFat(const QString &string);
    static QDateTime parseDate(const QString &string);
    static float parseWeigth(const QString &string);

protected slots:
    void nextStep();
    void onLoadFinshed(const bool ok);
    void onMutation(const QJsonObject &mutation);

private:
    ObservableWebPage * page;
#ifdef USE_WEB_ENGINE_VIEW
    QWebEngineView * view;
#endif
    QString username;
    QString password;
    bool scriptBusy;

signals:
    void weightFound(const float massInKgs);

};
