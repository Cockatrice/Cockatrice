#include "main.h"

#include "oraclewizard.h"
#include "settingscache.h"
#include "thememanager.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>

QTranslator *translator, *qtTranslator;
ThemeManager *themeManager;

const QString translationPrefix = "oracle";
QString translationPath;
bool isSpoilersOnly;

void installNewTranslator()
{
    QString lang = SettingsCache::instance().getLang();

    const auto okLoad1 = qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (!okLoad1) {
        qDebug() << "Error Loading";
    }
    qApp->installTranslator(qtTranslator);
    const auto okLoad2 = translator->load(translationPrefix + "_" + lang, translationPath);
    if (!okLoad2) {
        qDebug() << "Error Loading 2";
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
    QCommandLineOption showProgressOption("s", QCoreApplication::translate("main", "Only run in spoiler mode"));
    parser.addOption(showProgressOption);
    parser.process(app);
    isSpoilersOnly = parser.isSet(showProgressOption);

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    // set name of the app desktop file; used by wayland to load the window icon
    QGuiApplication::setDesktopFileName("oracle");
#endif

    wizard.show();

    return app.exec();
}
