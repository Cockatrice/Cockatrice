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

#include "main.h"
#include "QtNetwork/QNetworkInterface"
#include "carddatabase.h"
#include "dlg_settings.h"
#include "featureset.h"
#include "logger.h"
#include "pixmapgenerator.h"
#include "rng_sfmt.h"
#include "settingscache.h"
#include "soundengine.h"
#include "spoilerbackgroundupdater.h"
#include "thememanager.h"
#include "version_string.h"
#include "window_main.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QSystemTrayIcon>
#include <QTextCodec>
#include <QTextStream>
#include <QTranslator>
#include <QtPlugin>

CardDatabase *db;
QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
RNG_Abstract *rng;
SoundEngine *soundEngine;
QSystemTrayIcon *trayIcon;
ThemeManager *themeManager;

const QString translationPrefix = "cockatrice";
QString translationPath;

static void CockatriceLogger(QtMsgType type, const QMessageLogContext &ctx, const QString &message)
{
    Logger::getInstance().log(type, ctx, message);
}

void installNewTranslator()
{
    QString lang = settingsCache->getLang();

    qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qtTranslator);
    translator->load(translationPrefix + "_" + lang, translationPath);
    qApp->installTranslator(translator);
    qDebug() << "Language changed:" << lang;
}

QString const generateClientID()
{
    QString macList;
    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (interface.hardwareAddress() != "")
            if (interface.hardwareAddress() != "00:00:00:00:00:00:00:E0")
                macList += interface.hardwareAddress() + ".";
    }
    QString strClientID = QCryptographicHash::hash(macList.toUtf8(), QCryptographicHash::Sha1).toHex().right(15);
    return strClientID;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    qInstallMessageHandler(CockatriceLogger);
#ifdef Q_OS_WIN
    app.addLibraryPath(app.applicationDirPath() + "/plugins");
#endif

    // These values are only used by the settings loader/saver
    // Wrong or outdated values are kept to not break things
    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice.de");
    QCoreApplication::setApplicationName("Cockatrice");
    QCoreApplication::setApplicationVersion(VERSION_STRING);

#ifdef Q_OS_MAC
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

#ifdef Q_OS_MAC
    translationPath = qApp->applicationDirPath() + "/../Resources/translations";
#elif defined(Q_OS_WIN)
    translationPath = qApp->applicationDirPath() + "/translations";
#else // linux
    translationPath = qApp->applicationDirPath() + "/../share/cockatrice/translations";
#endif

    QCommandLineParser parser;
    parser.setApplicationDescription("Cockatrice");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions(
        {{{"c", "connect"}, QCoreApplication::translate("main", "Connect on startup"), "user:pass@host:port"},
         {{"d", "debug-output"}, QCoreApplication::translate("main", "Debug to file")}});

    parser.process(app);

    if (parser.isSet("debug-output")) {
        Logger::getInstance().logToFile(true);
    }

    rng = new RNG_SFMT;
    settingsCache = new SettingsCache;
    themeManager = new ThemeManager;
    soundEngine = new SoundEngine;
    db = new CardDatabase;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    QLocale::setDefault(QLocale::English);

    qsrand(QDateTime::currentDateTime().toTime_t());
    qDebug("main(): starting main program");

    MainWindow ui;
    if (parser.isSet("connect")) {
        ui.setConnectTo(parser.value("connect"));
    }
    qDebug("main(): MainWindow constructor finished");

    ui.setWindowIcon(QPixmap("theme:cockatrice"));

    settingsCache->setClientID(generateClientID());

    // If spoiler mode is enabled, we will download the spoilers
    // then reload the DB. otherwise just reload the DB
    SpoilerBackgroundUpdater spoilerBackgroundUpdater;

    ui.show();
    qDebug("main(): ui.show() finished");

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.exec();

    qDebug("Event loop finished, terminating...");
    delete db;
    delete settingsCache;
    delete rng;
    PingPixmapGenerator::clear();
    CountryPixmapGenerator::clear();
    UserLevelPixmapGenerator::clear();

    return 0;
}
