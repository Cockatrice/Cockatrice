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
#include <QDir>
#include <QDebug>
#include <QSystemTrayIcon>
#include "QtNetwork/QNetworkInterface"
#include <QCryptographicHash>
#include "main.h"
#include "window_main.h"
#include "dlg_settings.h"
#include "carddatabase.h"
#include "settingscache.h"
#include "thememanager.h"
#include "pixmapgenerator.h"
#include "rng_sfmt.h"
#include "soundengine.h"
#include "featureset.h"
#include "logger.h"

CardDatabase *db;
QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
RNG_Abstract *rng;
SoundEngine *soundEngine;
QSystemTrayIcon *trayIcon;
ThemeManager *themeManager;

const QString translationPrefix = "cockatrice";
QString translationPath;

static void CockatriceLogger(QtMsgType type, const QMessageLogContext &ctx, const QString &message) {
    Logger::getInstance().log(type, ctx, message);
}

void installNewTranslator()
{
    QString lang = settingsCache->getLang();

    qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qtTranslator);
    translator->load(translationPrefix + "_" + lang, translationPath);
    qApp->installTranslator(translator);
}

QString const generateClientID()
{
    QString macList;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
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

    qInstallMessageHandler(CockatriceLogger);
    if (app.arguments().contains("--debug-output"))
        Logger::getInstance().logToFile(true);

#ifdef Q_OS_WIN
    app.addLibraryPath(app.applicationDirPath() + "/plugins");
#endif

    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice.de");
    QCoreApplication::setApplicationName("Cockatrice");

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

    rng = new RNG_SFMT;
    settingsCache = new SettingsCache;
    themeManager = new ThemeManager;
    soundEngine = new SoundEngine;
    db = new CardDatabase;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    qsrand(QDateTime::currentDateTime().toTime_t());
    qDebug("main(): starting main program");

    MainWindow ui;
    qDebug("main(): MainWindow constructor finished");

    ui.setWindowIcon(QPixmap("theme:cockatrice"));
    
    settingsCache->setClientID(generateClientID());

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
