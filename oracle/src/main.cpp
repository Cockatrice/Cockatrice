#include <QApplication>
#include <QTextCodec>
#include <QIcon>
#include <QTranslator>
#include <QLibraryInfo>

#include "main.h"
#include "oraclewizard.h"
#include "settingscache.h"

QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;

const QString translationPrefix = "oracle";
#ifdef TRANSLATION_PATH
QString translationPath = TRANSLATION_PATH;
#else
QString translationPath = QString();
#endif

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

    if (translationPath.isEmpty()) {
#ifdef Q_OS_MAC
        translationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#elif defined(Q_OS_WIN)
        translationPath = app.applicationDirPath() + "/translations";
#endif
    }

	settingsCache = new SettingsCache;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

	OracleWizard wizard;

    QIcon icon(":/resources/appicon.svg");
    wizard.setWindowIcon(icon);

	wizard.show();

	return app.exec();
}
