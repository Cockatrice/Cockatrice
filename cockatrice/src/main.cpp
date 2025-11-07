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
#include "client/network/update/card_spoiler/spoiler_background_updater.h"
#include "client/settings/cache_settings.h"
#include "client/sound_engine.h"
#include "database/interface/settings_card_preference_provider.h"
#include "interface/logger.h"
#include "interface/pixel_map_generator.h"
#include "interface/theme_manager.h"
#include "interface/widgets/dialogs/dlg_settings.h"
#include "interface/window_main.h"
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
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/protocol/featureset.h>
#include <libcockatrice/rng/rng_sfmt.h>

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
#include <ShlObj.h>
#include <ctime>
#include <filesystem>
#pragma comment(lib, "DbgHelp.lib") // Link the DbgHelp library
// clang-format on

LONG WINAPI CockatriceUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionPointers)
{
    std::filesystem::path path;

    // Find %LOCALAPPDATA% (or cheat at finding it)
    wchar_t *localAppDataFolder;
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) != S_OK) {
        path = std::filesystem::temp_directory_path().parent_path().parent_path();
    } else {
        path = std::filesystem::path(localAppDataFolder);
    }

    // Plan on writing crash files into %LOCALAPPDATA%/CrashDumps/Cockatrice/cockatrice.crash.*.dmp
    path /= "CrashDumps";
    path /= "Cockatrice";
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    path /= "cockatrice.crash." + std::to_string(std::time(0)) + ".dmp";

    // Create and write crash files
#ifdef UNICODE
    HANDLE hDumpFile =
        CreateFile(path.wstring().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
    HANDLE hDumpFile =
        CreateFile(path.string().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ExceptionPointers = exceptionPointers;
    mei.ThreadId = GetCurrentThreadId();
    mei.ClientPointers = 1;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithFullMemory, &mei, nullptr,
                      nullptr);

    CloseHandle(hDumpFile);

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
        qCWarning(QtTranslatorDebug) << "Unable to load qt translation" << qtNameHint << "at" << qtTranslationPath;
    } else {
        qCInfo(QtTranslatorDebug) << "Loaded qt translation" << qtNameHint << "at" << qtTranslationPath;
    }
    qApp->installTranslator(qtTranslator);

    QString appNameHint = translationPrefix + "_" + lang;
    bool appTranslationLoaded = qtTranslator->load(appNameHint, translationPath);
    if (!appTranslationLoaded) {
        qCWarning(QtTranslatorDebug) << "Unable to load" << translationPrefix << "translation" << appNameHint << "at"
                                     << translationPath;
    } else {
        qCInfo(QtTranslatorDebug) << "Loaded" << translationPrefix << "translation" << appNameHint << "at"
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

#ifdef Q_OS_APPLE
    // <build>/cockatrice/cockatrice.app/Contents/MacOS/cockatrice
    const QByteArray configPath = "../../../qtlogging.ini";
#elif defined(Q_OS_UNIX)
    // <build>/cockatrice/cockatrice
    const QByteArray configPath = "./qtlogging.ini";
#elif defined(Q_OS_WIN)
    // <build>/cockatrice/Debug/cockatrice.exe
    const QByteArray configPath = "../qtlogging.ini";
#else
    const QByteArray configPath = "";
#endif

    if (!qEnvironmentVariableIsSet(("QT_LOGGING_CONF"))) {
        // Set the QT_LOGGING_CONF environment variable
        qputenv("QT_LOGGING_CONF", configPath);
    }
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

    // Dependency Injections
    CardDatabaseManager::setCardPreferenceProvider(new SettingsCardPreferenceProvider());
    CardDatabaseManager::setCardDatabasePathProvider(&SettingsCache::instance());
    CardDatabaseManager::setCardSetPriorityController(SettingsCache::instance().cardDatabase());

    qCInfo(MainLog) << "Starting main program";

    MainWindow ui;
    if (parser.isSet("connect")) {
        ui.setConnectTo(parser.value("connect"));
    }
    qCInfo(MainLog) << "MainWindow constructor finished";

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
    qCInfo(MainLog) << "ui.show() finished";

    // force shortcuts to be shown/hidden in right-click menus, regardless of system defaults
    qApp->setAttribute(Qt::AA_DontShowShortcutsInContextMenus, !SettingsCache::instance().getShowShortcuts());

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    app.exec();

    qCInfo(MainLog) << "Event loop finished, terminating...";
    delete rng;
    PingPixmapGenerator::clear();
    CountryPixmapGenerator::clear();
    UserLevelPixmapGenerator::clear();

    return 0;
}
