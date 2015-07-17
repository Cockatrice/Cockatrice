/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@gmx.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QtPlugin>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDateTime>
#include <QSettings>
#include <QIcon>
#include <QDir>
#include <QDesktopServices>
#include <QDebug>
#include <QSystemTrayIcon>
#include "QtNetwork/QNetworkInterface"
#include <QCryptographicHash>


#include "main.h"
#include "window_main.h"
#include "dlg_settings.h"
#include "carddatabase.h"
#include "settingscache.h"
#include "pixmapgenerator.h"
#include "rng_sfmt.h"
#include "soundengine.h"

//Q_IMPORT_PLUGIN(qjpeg)

CardDatabase *db;
QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
RNG_Abstract *rng;
SoundEngine *soundEngine;
QSystemTrayIcon *trayIcon;


const QString translationPrefix = "cockatrice";
#ifdef TRANSLATION_PATH
QString translationPath = TRANSLATION_PATH;
#else
QString translationPath = QString();
#endif

#if QT_VERSION < 0x050000
static void myMessageOutput(QtMsgType /*type*/, const char *msg)
{
    QFile file("qdebug.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream out(&file);
    out << msg << endl;
    file.close();
}
#else
static void myMessageOutput(QtMsgType /*type*/, const QMessageLogContext &, const QString &msg)
{
    QFile file("qdebug.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream out(&file);
    out << msg << endl;
    file.close();
}
#endif

void installNewTranslator()
{
    QString lang = settingsCache->getLang();

    qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qtTranslator);
    translator->load(translationPrefix + "_" + lang, translationPath);
    qApp->installTranslator(translator);
}

bool settingsValid()
{
    return QDir(settingsCache->getDeckPath()).exists() &&
        !settingsCache->getDeckPath().isEmpty() &&
        QDir(settingsCache->getPicsPath()).exists() &&
        !settingsCache->getPicsPath().isEmpty();
}

void generateClientID()
{
	QString macList;
	foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
	{
		if (interface.hardwareAddress() != "")
			if (interface.hardwareAddress() != "00:00:00:00:00:00:00:E0")
				macList += interface.hardwareAddress() + ".";
	}
    QString strClientID = QCryptographicHash::hash(macList.toUtf8(), QCryptographicHash::Sha1).toHex().right(15);
    settingsCache->setClientID(strClientID);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (app.arguments().contains("--debug-output"))
    {
#if QT_VERSION < 0x050000
        qInstallMsgHandler(myMessageOutput);
#else
        qInstallMessageHandler(myMessageOutput);
#endif
    }
#ifdef Q_OS_WIN
    app.addLibraryPath(app.applicationDirPath() + "/plugins");
#endif

#if QT_VERSION < 0x050000
    // gone in Qt5, all source files _MUST_ be utf8-encoded
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice.de");
    QCoreApplication::setApplicationName("Cockatrice");

    if (translationPath.isEmpty()) {
#ifdef Q_OS_MAC
        translationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#elif defined(Q_OS_WIN)
        translationPath = app.applicationDirPath() + "/translations";
#endif
    }

    rng = new RNG_SFMT;
    settingsCache = new SettingsCache;
    db = new CardDatabase;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    qsrand(QDateTime::currentDateTime().toTime_t());
    
#if QT_VERSION < 0x050000
    const QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    const QString dataDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
#endif
    if (!db->getLoadSuccess())
        if (!db->loadCardDatabase(dataDir + "/cards.xml"))
            settingsCache->setCardDatabasePath(dataDir + "/cards.xml");
    if (settingsCache->getTokenDatabasePath().isEmpty())
        settingsCache->setTokenDatabasePath(dataDir + "/tokens.xml");
    if (!QDir(settingsCache->getDeckPath()).exists() || settingsCache->getDeckPath().isEmpty()) {
        QDir().mkpath(dataDir + "/decks");
        settingsCache->setDeckPath(dataDir + "/decks");
    }
    if (!QDir(settingsCache->getReplaysPath()).exists() || settingsCache->getReplaysPath().isEmpty()) {
        QDir().mkpath(dataDir + "/replays");
        settingsCache->setReplaysPath(dataDir + "/replays");
    }
    if (!QDir(settingsCache->getPicsPath()).exists() || settingsCache->getPicsPath().isEmpty()) {
        QDir().mkpath(dataDir + "/pics");
        settingsCache->setPicsPath(dataDir + "/pics");
    }
    if (!QDir().mkpath(settingsCache->getPicsPath() + "/CUSTOM"))
        qDebug() << "Could not create " + settingsCache->getPicsPath().toUtf8() + "/CUSTOM. Will fall back on default card images.";
    if (QDir().mkpath(dataDir + "/customsets"))
    {
        // if the dir exists (or has just been created)
        db->loadCustomCardDatabases(dataDir + "/customsets");
    } else {
        qDebug() << "Could not create " + dataDir + "/customsets folder.";
    }

    if(settingsCache->getSoundPath().isEmpty() || !QDir(settingsCache->getSoundPath()).exists())
    {
        QDir tmpDir;
        
#ifdef Q_OS_MAC
        tmpDir = app.applicationDirPath() + "/../Resources/sounds";
#elif defined(Q_OS_WIN)
         tmpDir = app.applicationDirPath() + "/sounds";
#else // linux
        tmpDir = app.applicationDirPath() + "/../share/cockatrice/sounds/";
#endif
        settingsCache->setSoundPath(tmpDir.canonicalPath());
    }

    if (!settingsValid() || db->getLoadStatus() != Ok) {
        qDebug("main(): invalid settings or load status");
        DlgSettings dlgSettings;
        dlgSettings.show();
        app.exec();
    }

    if (settingsValid()) {
        qDebug("main(): starting main program");
        soundEngine = new SoundEngine;
        qDebug("main(): SoundEngine constructor finished");

        MainWindow ui;
        qDebug("main(): MainWindow constructor finished");

        QIcon icon(":/resources/appicon.svg");
        ui.setWindowIcon(icon);
        
        generateClientID();    //generate the users client id
        qDebug() << "ClientID In Cache: " << settingsCache->getClientID();

        ui.show();
        qDebug("main(): ui.show() finished");

        app.exec();
    }

    qDebug("Event loop finished, terminating...");
    delete db;
    delete settingsCache;
    delete rng;
    PingPixmapGenerator::clear();
    CountryPixmapGenerator::clear();
    UserLevelPixmapGenerator::clear();

    return 0;
}
