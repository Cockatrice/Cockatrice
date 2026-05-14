#include "main.h"

#include "interface/theme_manager.h"
#include "oraclewizard.h"

#include <../../cockatrice/src/client/settings/cache_settings.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QTimer>
#include <QTranslator>
#include <libcockatrice/utility/url_utils.h>
#include <libcockatrice/utility_gui/url_scheme_event_filter.h>

QTranslator *translator, *qtTranslator;
ThemeManager *themeManager;

const QString translationPrefix = "oracle";
QString translationPath;
bool isSpoilersOnly;
bool isBackgrounded;

void installNewTranslator()
{
    QString lang = SettingsCache::instance().getLang();

    QString qtNameHint = "qt_" + lang;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QString qtTranslationPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    QString qtTranslationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    bool qtTranslationLoaded = qtTranslator->load(qtNameHint, qtTranslationPath);
    if (!qtTranslationLoaded) {
        qDebug() << "Unable to load qt translation" << qtNameHint << "at" << qtTranslationPath;
    } else {
        qDebug() << "Loaded qt translation" << qtNameHint << "at" << qtTranslationPath;
    }
    qApp->installTranslator(qtTranslator);

    QString appNameHint = translationPrefix + "_" + lang;
    bool appTranslationLoaded = qtTranslator->load(appNameHint, translationPath);
    if (!appTranslationLoaded) {
        qDebug() << "Unable to load" << translationPrefix << "translation" << appNameHint << "at" << translationPath;
    } else {
        qDebug() << "Loaded" << translationPrefix << "translation" << appNameHint << "at" << translationPath;
    }
    qApp->installTranslator(translator);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice");
    // this can't be changed, as it influences the default save path for cards.xml
    QCoreApplication::setApplicationName("Cockatrice");

    // If the program is opened with the -s flag, it will only do spoilers. Otherwise it will do MTGJSON/Tokens
    QCommandLineParser parser;
    QCommandLineOption spoilersOnlyOption("s", QCoreApplication::translate("main", "Only run in spoiler mode"));
    QCommandLineOption backgroundOption("b", QCoreApplication::translate("main", "Run in no-confirm background mode"));
    parser.addOption(spoilersOnlyOption);
    parser.addOption(backgroundOption);
    parser.addPositionalArgument(
        "url", QCoreApplication::translate("main", "Optional cockatrice-oracle:// URL to handle"), "[url]");
    parser.process(app);
    isSpoilersOnly = parser.isSet(spoilersOnlyOption);
    isBackgrounded = parser.isSet(backgroundOption);

    // Handle cockatrice-oracle:// URL passed via the OS URL scheme handler
    const QString oracleUrl =
        UrlUtils::findUrlArgument(parser.positionalArguments(), QStringLiteral("cockatrice-oracle://"));
    if (!oracleUrl.isEmpty()) {
        const auto action = UrlUtils::parseOracleUrl(oracleUrl);
        if (action.isUpdate) {
            isBackgrounded = true;
            if (action.spoilersOnly)
                isSpoilersOnly = true;
        } else {
            qDebug() << "Oracle: ignoring unknown cockatrice-oracle:// URL:" << oracleUrl;
        }
    }

#ifdef Q_OS_MAC
    translationPath = qApp->applicationDirPath() + "/../Resources/translations";
#elif defined(Q_OS_WIN)
    translationPath = qApp->applicationDirPath() + "/translations";
#else // linux
    translationPath = qApp->applicationDirPath() + "/../share/oracle/translations";
#endif

    themeManager = new ThemeManager;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    OracleWizard wizard;

    QIcon icon("theme:appicon.svg");
    wizard.setWindowIcon(icon);
    // set name of the app desktop file; used by wayland to load the window icon
    QGuiApplication::setDesktopFileName("oracle");

#ifdef Q_OS_MAC
    // On macOS the OS delivers a registered URL scheme via QFileOpenEvent,
    // dispatched on the first event-loop spin.  Oracle has no nested event
    // loop before app.exec(), so installing the filter here (after wizard
    // construction but before app.exec()) is sufficient — the cold-start URL
    // event sits in the queue until app.exec() dispatches it, by which point
    // both the filter and wizard exist.
    UrlSchemeEventFilter oracleFilter(QStringLiteral("cockatrice-oracle://"));
    QObject::connect(&oracleFilter, &UrlSchemeEventFilter::urlReceived, &wizard, [&wizard](const QString &url) {
        const auto action = UrlUtils::parseOracleUrl(url);
        if (!action.isUpdate) {
            qDebug() << "Oracle: ignoring unknown cockatrice-oracle:// URL:" << url;
            return;
        }
        if (action.spoilersOnly)
            isSpoilersOnly = true;
        QTimer::singleShot(0, &wizard, [&wizard]() { wizard.runInBackground(); });
    });
    app.installEventFilter(&oracleFilter);
#endif

    wizard.show();

    if (isBackgrounded) {
        QTimer::singleShot(0, &wizard, [&wizard]() { wizard.runInBackground(); });
    }

    return app.exec();
}
