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

    const auto fileName1 = "qt_" + lang;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const auto dir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const auto dir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    const auto fileLoaded1 = qtTranslator->load(fileName1, dir);
    if (!fileLoaded1) {
        qDebug() << "(1) Unable to load translation file" << QFileInfo(dir, fileName1).absoluteFilePath();
    } else {
        qDebug() << "(1) Loaded translation file" << QFileInfo(dir, fileName1).absoluteFilePath();
    }
    qApp->installTranslator(qtTranslator);

    const auto fileName2 = translationPrefix + "_" + lang;
    const auto fileLoaded2 = qtTranslator->load(fileName2, translationPath);
    if (!fileLoaded2) {
        qDebug() << "(2) Unable to load translation file" << QFileInfo(translationPath, fileName2).absoluteFilePath();
    } else {
        qDebug() << "(2) Loaded translation file" << QFileInfo(translationPath, fileName2).absoluteFilePath();
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
    // set name of the app desktop file; used by wayland to load the window icon
    QGuiApplication::setDesktopFileName("oracle");

    wizard.show();

    return app.exec();
}
