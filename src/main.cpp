#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QLoggingCategory>
#include <QProcessEnvironment>
#include <QSettings>

#include "fitbit.h"
#include "polar.h"

void configureLogging(const QCommandLineParser &parser);

int main(int argc, char *argv[])
{
    // Configure the offscreen renderer. To take affect, this need to be done before QApplication
    // is initialised.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        bool showOnScreen = false;
        for (int i=1;(!showOnScreen) && (i<argc); ++i) {
            if (strcmp(argv[i], "--show") == 0) {
                showOnScreen = true;
            }
        }
        if ((!showOnScreen) && (!qputenv("QT_QPA_PLATFORM", "offscreen"))) {
            qWarning() << "Failed to set QT_QPA_PLATFORM to offscreen";
        }
    }

    // Initialise our Qt application.
    QApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral("0.0.1"));

    // Parse the command line options.
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Get Office 365 tenant admin consent"));
    parser.addHelpOption();
    parser.addOptions({
        {{QStringLiteral("c"), QStringLiteral("credentials")},
          QStringLiteral("Read credentials from filename"),  QStringLiteral("filename")},
        {{QStringLiteral("d"), QStringLiteral("debug")}, QStringLiteral("Enable debug output")},
        { QStringLiteral("no-color"), QStringLiteral("Do not color the output")},
        { QStringLiteral("show"), QStringLiteral("Show the web view on screen")},
    });
    parser.addVersionOption();
    parser.process(app);
    configureLogging(parser);

    // Fetch the credentials.
    #define FETCH_ENV(var, name) \
        QString var = QProcessEnvironment::systemEnvironment().value(QLatin1String(#name))
    FETCH_ENV(fitbitUser, FITBIT_USERNAME);
    FETCH_ENV(fitbitPass, FITBIT_PASSWORD);
    FETCH_ENV(polarUser, POLAR_USERNAME);
    FETCH_ENV(polarPass, POLAR_PASSWORD);
    qDebug() << parser.isSet(QStringLiteral("credentials"));
    if (parser.isSet(QStringLiteral("credentials"))) {
        qDebug() << parser.value(QStringLiteral("credentials"));
        QSettings settings(parser.value(QStringLiteral("credentials")), QSettings::IniFormat);
        qDebug() << settings.allKeys();
        #define FETCH_SETTING(var, name) \
            if (settings.contains(QLatin1String(#name))) \
                var = settings.value(QLatin1String(#name)).toString()
        FETCH_SETTING(fitbitUser, Fitbit/username);
        FETCH_SETTING(fitbitPass, Fitbit/password);
        FETCH_SETTING(polarUser, Polar/username);
        FETCH_SETTING(polarPass, Polar/password);
    }
    #define REQUIRE_SETTING(var, name) \
        if (var.isEmpty()) { \
            qCritical().noquote() << "Option is required:" << QStringLiteral(#name); \
            parser.showHelp(EXIT_FAILURE); \
        }
    REQUIRE_SETTING(fitbitUser, Fitbit/username)
    REQUIRE_SETTING(fitbitPass, Fitbit/password)
    REQUIRE_SETTING(polarUser, Polar/username)
    REQUIRE_SETTING(polarPass, Polar/password)

    // Do it.
    Fitbit fitbit(fitbitUser, fitbitPass);
    Polar polar(polarUser, polarPass);
    QObject::connect(&fitbit,&Fitbit::weightFound,&polar,&Polar::setWeight);
    fitbit.fetchWeight();
    return app.exec();
}

/*!
 * Configure application logging based on the command line \a parser
 */
void configureLogging(const QCommandLineParser &parser)
{
    // Start with the Qt default message pattern (see qtbase:::qlogging.cpp:defaultPattern)
    QString messagePattern = QStringLiteral("%{if-category}%{category}: %{endif}%{message}");

    if (parser.isSet(QStringLiteral("debug"))) {
        messagePattern.prepend(QStringLiteral("%{time process} %{type} %{function} "));
        QLoggingCategory::setFilterRules(QStringLiteral("js=true\n*.debug=false"));
        QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
    } else {
        // Note, due to limitations in the Chromium API, console debug messages come through as
        // info. So we don't enable js.info logging unless in debug mode.
        QLoggingCategory::setFilterRules(QStringLiteral("js=true\njs.info=false\n*.debug=false"));
    }

    if (!parser.isSet(QStringLiteral("no-color"))) {
        messagePattern.prepend(QStringLiteral(
        "%{if-debug}\x1b[90m%{endif}"        // Gray
        "%{if-info}\x1b[34m%{endif}"         // Blue
        "%{if-warning}\x1b[35m%{endif}"      // Magenta
        "%{if-critical}\x1b[31m%{endif}"     // Red
        "%{if-fatal}\x1b[1;31;1m%{endif}")); // Red and bold
        messagePattern.append(QStringLiteral("\x1b[0m"));
    }

    qSetMessagePattern(messagePattern);
}
