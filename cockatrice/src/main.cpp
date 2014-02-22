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
#include <QTextCodec>
#include <QtPlugin>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDateTime>
#include <QSettings>
#include <QIcon>
#include <QDir>
#include <QDesktopServices>
#include <stdio.h>

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

const QString translationPrefix = "cockatrice";
#ifdef TRANSLATION_PATH
QString translationPath = TRANSLATION_PATH;
#else
QString translationPath = QString();
#endif

void myMessageOutput(QtMsgType /*type*/, const char *msg)
{
    static FILE *f = NULL;
    if (!f)
        f = fopen("qdebug.txt", "w");
    fprintf(f, "%s\n", msg);
    fflush(f);
}

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
    
    if (app.arguments().contains("--debug-output"))
        qInstallMsgHandler(myMessageOutput);
#ifdef Q_OS_MAC
    QDir baseDir(app.applicationDirPath());
    baseDir.cdUp();
    baseDir.cdUp();
    baseDir.cdUp();
    QDir pluginsDir = baseDir;
    pluginsDir.cd("PlugIns");
    app.addLibraryPath(pluginsDir.absolutePath());
#endif
#ifdef Q_OS_WIN
    app.addLibraryPath(app.applicationDirPath() + "/plugins");
#endif
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice.de");
    QCoreApplication::setApplicationName("Cockatrice");
    
    if (translationPath.isEmpty()) {
#ifdef Q_OS_MAC
        QDir translationsDir = baseDir;
        translationsDir.cd("translations");
        translationPath = translationsDir.absolutePath();
#endif
#ifdef Q_OS_WIN
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
    
    bool startMainProgram = true;
    const QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    if (!db->getLoadSuccess())
        if (db->loadCardDatabase(dataDir + "/cards.xml"))
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
    if (!db->getLoadSuccess() || !QDir(settingsCache->getDeckPath()).exists() || settingsCache->getDeckPath().isEmpty() || settingsCache->getPicsPath().isEmpty() || !QDir(settingsCache->getPicsPath()).exists()) {
        DlgSettings dlgSettings;
        dlgSettings.show();
        app.exec();
        startMainProgram = (db->getLoadSuccess() && QDir(settingsCache->getDeckPath()).exists() && !settingsCache->getDeckPath().isEmpty() && QDir(settingsCache->getPicsPath()).exists() && !settingsCache->getPicsPath().isEmpty());
    }
    
    if (startMainProgram) {
        qDebug("main(): starting main program");
        soundEngine = new SoundEngine;
        qDebug("main(): SoundEngine constructor finished");

        MainWindow ui;
        qDebug("main(): MainWindow constructor finished");
        
        QIcon icon(":/resources/appicon.svg");
        ui.setWindowIcon(icon);
        
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
