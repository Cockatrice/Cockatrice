#include "settingscache.h"
#include <QCoreApplication>
#include <QFile>
#if QT_VERSION >= 0x050000
    #include <QStandardPaths>
#else
    #include <QDesktopServices>
#endif

QString SettingsCache::guessConfigurationPath(QString & specificPath)
{
	const QString fileName="servatrice.ini";
	QString guessFileName;

	// specific path
	if(!specificPath.isEmpty() && QFile::exists(specificPath))
		return specificPath;

	// application directory path
	guessFileName = QCoreApplication::applicationDirPath() + "/" + fileName;
	if(QFile::exists(guessFileName))
		return guessFileName;

#ifdef Q_OS_UNIX
	// /etc
	guessFileName = "/etc/" + fileName;
	if(QFile::exists(guessFileName))
		return guessFileName;
#endif

#if QT_VERSION >= 0x050000
    guessFileName =  QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + fileName;
#else
    guessFileName =  QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + fileName;
#endif
    return guessFileName;
}