#include <QApplication>
#include <QTextCodec>
#include "oraclewizard.h"
#include "settingscache.h"

SettingsCache *settingsCache;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	
	QCoreApplication::setOrganizationName("Cockatrice");
	QCoreApplication::setOrganizationDomain("cockatrice");
	// this can't be changed, as it influences the default savepath for cards.xml
	QCoreApplication::setApplicationName("Cockatrice");
	
	settingsCache = new SettingsCache;

	OracleWizard wizard;
	wizard.show();

	return app.exec();
}
