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
#include "client/network/spoiler_background_updater.h"
#include "client/sound_engine.h"
#include "client/ui/pixel_map_generator.h"
#include "client/ui/theme_manager.h"
#include "client/ui/window_main.h"
#include "dialogs/dlg_settings.h"
#include "featureset.h"
#include "game/cards/card_database.h"
#include "rng_sfmt.h"
#include "settings/cache_settings.h"
#include "utility/logger.h"
#include "version_string.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QTranslator>
#include <QtPlugin>

QTranslator *translator, *qtTranslator;
RNG_Abstract *rng;
SoundEngine *soundEngine;
QSystemTrayIcon *trayIcon;
ThemeManager *themeManager;

const QString translationPrefix = "cockatrice";
QString translationPath;

static void CockatriceLogger(QtMsgType type, const QMessageLogContext &ctx, const QString &message)
{
    QString logMessage = qFormatLogMessage(type, ctx, message);

    // Regular expression to match the full path in the square brackets and extract only the filename and line number
    QRegularExpression regex(R"(\[(?:.:)?[\/\\].*[\/\\]([^\/\\]+\:\d+)\])");
    QRegularExpressionMatch match = regex.match(logMessage);

    if (match.hasMatch()) {
        // Extract the filename and line number (e.g., "main.cpp:211")
        QString filenameLine = match.captured(1);

        // Replace the full path in square brackets with just the filename and line number
        logMessage.replace(match.captured(0), QString("[%1]").arg(filenameLine));
    }

    Logger::getInstance().log(type, ctx, logMessage);
}

#ifdef Q_OS_WIN
// clang-format off
#include <Windows.h>
#include <DbgHelp.h>
#include <tchar.h>
#pragma comment(lib, "DbgHelp.lib") // Link the DbgHelp library
// clang-format on

LONG WINAPI CockatriceUnhandledExceptionFilter(EXCEPTION_POINTERS *pExceptionPointers)
{
    HANDLE hDumpFile =
        CreateFile(_T("cockatrice.crash.dmp"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDumpFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pExceptionPointers;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithFullMemory, &dumpInfo,
                          NULL, NULL);
        CloseHandle(hDumpFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void installNewTranslator()
{
    QString lang = SettingsCache::instance().getLang();

    QString qtNameHint = "qt_" + lang;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QString qtTranslationPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    QString qtTranslationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    bool qtTranslationLoaded = qtTranslator->load(qtNameHint, qtTranslationPath);
    if (!qtTranslationLoaded) {
        qCDebug(QtTranslatorDebug) << "Unable to load qt translation" << qtNameHint << "at" << qtTranslationPath;
    } else {
        qCDebug(QtTranslatorDebug) << "Loaded qt translation" << qtNameHint << "at" << qtTranslationPath;
    }
    qApp->installTranslator(qtTranslator);

    QString appNameHint = translationPrefix + "_" + lang;
    bool appTranslationLoaded = qtTranslator->load(appNameHint, translationPath);
    if (!appTranslationLoaded) {
        qCDebug(QtTranslatorDebug) << "Unable to load" << translationPrefix << "translation" << appNameHint << "at"
                                   << translationPath;
    } else {
        qCDebug(QtTranslatorDebug) << "Loaded" << translationPrefix << "translation" << appNameHint << "at"
                                   << translationPath;
    }
    qApp->installTranslator(translator);
}

QString const generateClientID()
{
    QString macList;
    for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
        if (networkInterface.hardwareAddress() != "")
            if (networkInterface.hardwareAddress() != "00:00:00:00:00:00:00:E0")
                macList += networkInterface.hardwareAddress() + ".";
    }
    QString strClientID = QCryptographicHash::hash(macList.toUtf8(), QCryptographicHash::Sha1).toHex().right(15);
    return strClientID;
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(CockatriceUnhandledExceptionFilter);
#endif

    // Set the QT_LOGGING_CONF environment variable
    qputenv("QT_LOGGING_CONF", "./qtlogging.ini");
    qSetMessagePattern(
        "\033[0m[%{time yyyy-MM-dd h:mm:ss.zzz} "
        "%{if-debug}\033[36mD%{endif}%{if-info}\033[32mI%{endif}%{if-warning}\033[33mW%{endif}%{if-critical}\033[31mC%{"
        "endif}%{if-fatal}\033[1;31mF%{endif}\033[0m] [%{function}] - %{message} [%{file}:%{line}]");
    QApplication app(argc, argv);

    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QApplication::quit);

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
    themeManager = new ThemeManager;
    soundEngine = new SoundEngine;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    QLocale::setDefault(QLocale::English);

    qCDebug(MainLog) << "Starting main program";

    MainWindow ui;
    if (parser.isSet("connect")) {
        ui.setConnectTo(parser.value("connect"));
    }
    qCDebug(MainLog) << "MainWindow constructor finished";

    ui.setWindowIcon(QPixmap("theme:cockatrice"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    // set name of the app desktop file; used by wayland to load the window icon
    QGuiApplication::setDesktopFileName("cockatrice");
#endif

    SettingsCache::instance().setClientID(generateClientID());

    // If spoiler mode is enabled, we will download the spoilers
    // then reload the DB. otherwise just reload the DB
    SpoilerBackgroundUpdater spoilerBackgroundUpdater;

    ui.show();
    qCDebug(MainLog) << "ui.show() finished";

    // force shortcuts to be shown/hidden in right-click menus, regardless of system defaults
    qApp->setAttribute(Qt::AA_DontShowShortcutsInContextMenus, !SettingsCache::instance().getShowShortcuts());

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    app.exec();

    qDebug("Event loop finished, terminating...");
    delete rng;
    PingPixmapGenerator::clear();
    CountryPixmapGenerator::clear();
    UserLevelPixmapGenerator::clear();

    return 0;
}
