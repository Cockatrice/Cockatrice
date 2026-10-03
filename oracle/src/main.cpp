#include "main.h"

#include "interface/theme_manager.h"
#include "oraclewizard.h"

#include <../../cockatrice/src/client/settings/cache_settings.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QTimer>
#include <QTranslator>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/utility/translation_loader.h>

QTranslator *translator, *qtTranslator;
ThemeManager *themeManager;

const QString translationPrefix = "oracle";
QString translationPath;
bool isSpoilersOnly;
bool isBackgrounded;

void installNewTranslator()
{
    QString lang = SettingsCache::instance().personal().getLang();

    // Qt's own strings and ours need a translator each: QTranslator discards its previous contents
    // on every load, so loading both files into one translator silently drops the first of them.
    TranslationLoader::loadQt(*qtTranslator, lang);
    qApp->installTranslator(qtTranslator);

    TranslationLoader::loadApplication(*translator, translationPrefix, lang,
                                       TranslationLoader::applicationTranslationPaths(translationPath));
    qApp->installTranslator(translator);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("Cockatrice");
    // This can't be changed, as it influences the default save path for cards.xml
    QCoreApplication::setApplicationName("Cockatrice");

    // If the program is opened with the -s flag, it will only do spoilers. Otherwise it will do MTGJSON/Tokens
    QCommandLineParser parser;
    QCommandLineOption spoilersOnlyOption("s", QCoreApplication::translate("main", "Only run in spoiler mode"));
    QCommandLineOption backgroundOption("b", QCoreApplication::translate("main", "Run in no-confirm background mode"));
    parser.addOption(spoilersOnlyOption);
    parser.addOption(backgroundOption);
    parser.process(app);
    isSpoilersOnly = parser.isSet(spoilersOnlyOption);
    isBackgrounded = parser.isSet(backgroundOption);

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
    // Base name of the installed oracle.desktop; wayland reads the window icon from it and
    // xdg-desktop-portal registers it as the app ID, so the case has to match the file name.
    QGuiApplication::setDesktopFileName("oracle");

    wizard.show();

    if (isBackgrounded) {
        QTimer::singleShot(0, &wizard, [&wizard]() { wizard.runInBackground(); });
    }

    return app.exec();
}
