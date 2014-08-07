#include <QApplication>
#include <QTextCodec>
#include <QIcon>
#include "oraclewizard.h"
#include "settingscache.h"

SettingsCache *settingsCache;

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

	settingsCache = new SettingsCache;

	OracleWizard wizard;

    QIcon icon(":/resources/appicon.svg");
    wizard.setWindowIcon(icon);

	wizard.show();

	return app.exec();
}
