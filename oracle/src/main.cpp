#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>

#include "main.h"
#include "oraclewizard.h"
#include "settingscache.h"
#include "thememanager.h"

QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
ThemeManager *themeManager;

const QString translationPrefix = "oracle";
QString translationPath;
bool isSpoilersOnly;

void installNewTranslator()
{
    QString lang = settingsCache->getLang();

    qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qtTranslator);
    translator->load(translationPrefix + "_" + lang, translationPath);
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
    QCommandLineOption showProgressOption("s", QCoreApplication::translate("main", "Only run in spoiler mode"));
    parser.addOption(showProgressOption);
    parser.process(app);
    isSpoilersOnly = parser.isSet(showProgressOption);

#ifdef Q_OS_MAC
    translationPath = qApp->applicationDirPath() + "/../Resources/translations";
#elif defined(Q_OS_WIN)
    translationPath = qApp->applicationDirPath() + "/translations";
#else // linux
    translationPath = qApp->applicationDirPath() + "/../share/cockatrice/translations";
#endif

    settingsCache = new SettingsCache;
    themeManager = new ThemeManager;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    OracleWizard wizard;

    QIcon icon("theme:appicon.svg");
    wizard.setWindowIcon(icon);

    wizard.show();

    return app.exec();
}
