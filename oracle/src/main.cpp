#include <QApplication>
#include <QTextCodec>
#include "window_main.h"
#include "settingscache.h"

SettingsCache *settingsCache;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	
	QCoreApplication::setOrganizationName("Cockatrice");
	QCoreApplication::setOrganizationDomain("cockatrice.de");
	QCoreApplication::setApplicationName("Cockatrice");
	
	settingsCache = new SettingsCache;
	
	WindowMain wnd;
	wnd.show();
	
	return app.exec();
}
