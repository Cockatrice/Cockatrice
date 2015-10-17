#include <QApplication>
#include <QTextCodec>
#include <QIcon>
#include <QTranslator>
#include <QLibraryInfo>

#include "main.h"
#include "oraclewizard.h"
#include "settingscache.h"
#include "thememanager.h"

QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
ThemeManager *themeManager;

const QString translationPrefix = "oracle";
QString translationPath;

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

#if QT_VERSION < 0x050000
	// gone in Qt5, all source files _MUST_ be utf8-encoded
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	QCoreApplication::setOrganizationName("Cockatrice");
	QCoreApplication::setOrganizationDomain("cockatrice");
	// this can't be changed, as it influences the default savepath for cards.xml
	QCoreApplication::setApplicationName("Cockatrice");

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
