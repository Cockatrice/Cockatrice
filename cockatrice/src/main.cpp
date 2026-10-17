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

#include "client/crash/crash_reporter.h"
#include "client/network/update/card_spoiler/spoiler_background_updater.h"
#include "client/settings/cache_settings.h"
#include "client/sound_engine.h"
#include "client/url_scheme_event_filter.h"
#include "database/interface/settings_card_preference_provider.h"
#include "interface/intents/intent.h"
#include "interface/intents/intent_open_local_deck.h"
#include "interface/logger.h"
#include "interface/pixel_map_generator.h"
#include "interface/theme_manager.h"
#include "interface/widgets/dialogs/dlg_crash_report.h"
#include "interface/window_main.h"
#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/rng/rng_abstract.h"
#include "single_instance_manager.h"
#include "version_string.h"

#include <QApplication>
#include <QByteArray>
#include <QByteArrayView>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QList>
#include <QLocale>
#include <QMessageBox>
#include <QMessageLogger>
#include <QNetworkInterface>
#include <QObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QTranslator>
#include <QUrl>
#include <QtGlobal>
#include <algorithm>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/rng/rng_sfmt.h>
#include <libcockatrice/settings/card_database_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/network_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/utility/cryptoutil.h>
#include <libcockatrice/utility/translation_loader.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qsystemdetection.h>

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

void installNewTranslator()
{
    QString lang = SettingsCache::instance().personal().getLang();

    // Qt's own strings and ours need a translator each: QTranslator discards its previous contents
    // on every load, so loading both files into one translator silently drops the first of them.
    TranslationLoader::loadQt(*qtTranslator, lang);
    qApp->installTranslator(qtTranslator);

    TranslationLoader::loadApplication(*translator, translationPrefix, lang,
                                       TranslationLoader::applicationTranslationPaths(translationPath));
    qApp->installTranslator(translator);
}

QString const generateClientID()
{
    QString macList;
    for (const QNetworkInterface &networkInterface : QNetworkInterface::allInterfaces()) {
        if (networkInterface.hardwareAddress() != "") {
            if (networkInterface.hardwareAddress() != "00:00:00:00:00:00:00:E0") {
                macList += networkInterface.hardwareAddress() + ".";
            }
        }
    }
    QString strClientID = QCryptographicHash::hash(macList.toUtf8(), QCryptographicHash::Sha1).toHex().right(15);
    return strClientID;
}

static QString redactActivationUrl(const QString &url)
{
    // Activation URLs carry secrets in their query string (e.g. the deck share
    // token); log only the scheme and the action (cockatrice://opendeck), never
    // the parameters.
    if (!url.startsWith(QStringLiteral("cockatrice://"))) {
        return url;
    }
    const QUrl parsed(url);
    return parsed.scheme() + "://" + parsed.host();
}

/**
 * @brief Run the startup crash-report flow for an unclean exit (or --simulate-crash).
 *
 * With a configured DSN the SDK owns capture and upload (consent-gated); with an
 * empty DSN the flow falls back to a local mock file. AlwaysSend uploads silently
 * (the log was folded into the pending native envelope(s) at startup); Ask offers
 * the dialog; NeverSend skips silently.
 *
 * @param attachedEnvelopes number of pending native envelopes that already carry the
 *                          redacted log, so an AlwaysSend run only sends a standalone
 *                          log event when no crash event existed.
 */
static void runCrashReportFlow(CrashReporter &crashReporter, QWidget *parent, int attachedEnvelopes)
{
    const PersonalSettings::CrashReportingMode crashMode = SettingsCache::instance().personal().getCrashReportingMode();
    const bool crashReady = crashReporter.isReady();
    if (crashMode == PersonalSettings::CrashReportingMode::AlwaysSend) {
        if (crashReady) {
            // Silent path: the redacted log was folded into the pending native crash
            // envelope(s) at startup and consent already released them. Only when no
            // pending crash event existed (e.g. --simulate-crash) does a standalone
            // log event still need to go out here.
            if (attachedEnvelopes == 0) {
                const QString crashPreview = DlgCrashReport::buildRedactedPreview();
                if (!crashReporter.sendReport(crashPreview)) {
                    const QString savedPath = DlgCrashReport::writeMockReport(crashPreview);
                    qCWarning(MainLog) << "Failed to send crash report, saved locally to" << savedPath;
                }
            }
        } else {
            DlgCrashReport::writeMockReport(DlgCrashReport::buildRedactedPreview());
        }
    } else if (crashMode == PersonalSettings::CrashReportingMode::Ask) {
        // Real mode: Send gives consent + captures the event, Don't send revokes
        // consent; the dialog owns both via the reporter.
        DlgCrashReport crashDialog(parent, crashReady, crashReady ? &crashReporter : nullptr);
        crashDialog.exec();
    }
    // NeverSend: skip silently (no dialog, nothing written or sent).
}

int main(int argc, char *argv[])
{
    // No custom unhandled-exception filter: sentry owns native crash capture on all
    // platforms. (The legacy MiniDumpWithFullMemory writer was removed as a privacy
    // liability; unclean exits are detected via CrashReporter::beginSession below.)

    // Logging setup
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

#ifdef Q_OS_MAC
    UrlSchemeEventFilter cockatriceFilter(QStringList{QStringLiteral("cockatrice")});

    QStringList pendingMacUrls;

    const auto cocoaBufferConn =
        QObject::connect(&cockatriceFilter, &UrlSchemeEventFilter::urlReceived,
                         [&pendingMacUrls](const QString &url) { pendingMacUrls.append(url); });

    app.installEventFilter(&cockatriceFilter);
#endif

    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QApplication::quit);

    qInstallMessageHandler(CockatriceLogger);

#ifdef Q_OS_WIN
    app.addLibraryPath(app.applicationDirPath() + "/plugins");
#endif

    // These values are only used by the settings loader/saver
    // Wrong or outdated values are kept to not break things
    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice.github.io");
    QCoreApplication::setApplicationName("Cockatrice");
    QCoreApplication::setApplicationVersion(VERSION_STRING);

#ifdef Q_OS_MAC
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    // Persistent session log: keeps this run's logs on disk so the next-startup crash dialog can
    // attach the crashed session's tail (the in-memory buffer is lost on crash). Opened in Append
    // mode with a per-launch session marker; read back via Logger::getPreviousSessionTail().
    // Declared before crash-reporter init: the startup attach below needs the log tail while the
    // file still holds the crashed session's lines.
    const QString persistentLogPath =
        SettingsCache::instance().getSettingsPath() + QStringLiteral("logs/cockatrice.log");
    Logger::getInstance().setPersistentLogPath(persistentLogPath);

    // Crash reporting: init the SDK early so a crash anywhere below is captured into the
    // local database. Uploads stay consent-gated (require_user_consent, armed in CrashReporter::init):
    // AlwaysSend gives consent here so cached envelopes flush at startup; any other mode starts
    // unconsented and the dialog below drives give/revoke. An empty DSN skips the SDK entirely and
    // the mock fallback further below behaves exactly as before.
    CrashReporter crashReporter;
    const QString crashDatabasePath = SettingsCache::instance().getSettingsPath() + QStringLiteral("sentry");
    // Envelopes that received the redacted log at startup (AlwaysSend path); 0 means the log was
    // NOT folded into a pending crash event and a standalone log event is still needed below.
    int crashLogAttachedEnvelopes = 0;
    {
        QString crashDsn = QString::fromUtf8(qgetenv("COCKATRICE_SENTRY_DSN"));
        const bool dsnFromEnvironment = !crashDsn.isEmpty();
        if (!dsnFromEnvironment) {
            crashDsn = QString::fromUtf8(COCKATRICE_SENTRY_DSN);
        }
        if (crashDsn.isEmpty()) {
            // Nothing is armed without a DSN, so no crash is ever captured and nothing is ever sent.
            // It is the one disabled configuration a tester cannot see from the UI, so say it here.
            qCWarning(CrashReportLog) << "crash reporting disabled: no DSN configured (COCKATRICE_SENTRY_DSN is "
                                         "not set and this build has no DSN baked in)";
        } else if (!CrashReporter::isDsnWellFormed(crashDsn)) {
            // Same rule the installer applies (cmake/CrashReporting.cmake): a malformed DSN disables
            // reporting rather than silently falling back to the project baked into the build, which
            // would upload where the tester never chose. The SDK would otherwise refuse to build a
            // request at all and report nothing about it.
            qCWarning(CrashReportLog) << "crash reporting disabled:"
                                      << (dsnFromEnvironment ? "COCKATRICE_SENTRY_DSN is set but malformed"
                                                             : "the DSN baked into this build is malformed")
                                      << "(expected <scheme>://<key>@<host>/<project-id>)";
            crashDsn.clear();
        } else if (dsnFromEnvironment) {
            // Destination and mode are logged by CrashReporter::init() once it resolves them.
            qCInfo(CrashReportLog) << "using COCKATRICE_SENTRY_DSN from the environment";
        }
        const PersonalSettings::CrashReportingMode crashMode =
            SettingsCache::instance().personal().getCrashReportingMode();
        // AlwaysSend folds the crashed session's redacted log into the pending crash
        // envelope(s) BEFORE init: sentry_init replays the crashed run's *.run
        // envelopes immediately when consent is already persisted, so attaching any
        // later would be too late. Consent given below then releases them, and dump,
        // stack and log upload as a single Sentry event. Ask mode attaches at the
        // dialog instead — its cache-held envelopes stay untouched until giveConsent().
        if (crashMode == PersonalSettings::CrashReportingMode::AlwaysSend &&
            CrashReporter::isReportingEnabled(crashDsn, crashMode)) {
            crashLogAttachedEnvelopes = CrashReporter::attachToPendingCrashEnvelopes(
                crashDatabasePath, DlgCrashReport::buildRedactedPreview(), QString());
        }
        crashReporter.init(crashDsn, QString::fromUtf8(COCKATRICE_SENTRY_RELEASE), crashDatabasePath, crashMode);
        if (crashReporter.isReady() && crashMode == PersonalSettings::CrashReportingMode::AlwaysSend) {
            // Silent opt-in: cached native crash envelopes flush automatically once consent is given.
            crashReporter.giveConsent();
        }
        // Mid-session mode changes must reach the SDK: without this, switching from AlwaysSend to
        // NeverSend (or Ask) leaves consent given until restart, so a later crash in this session
        // would still upload after the user opted out. crashReporter outlives main(), so capturing
        // it here is safe.
        QObject::connect(&SettingsCache::instance().personal(), &PersonalSettings::crashReportingModeChanged,
                         [&crashReporter](PersonalSettings::CrashReportingMode mode) {
                             if (mode == PersonalSettings::CrashReportingMode::AlwaysSend) {
                                 crashReporter.giveConsent();
                             } else {
                                 crashReporter.revokeConsent();
                             }
                         });
    }

    // Translations
#ifdef Q_OS_MAC
    translationPath = qApp->applicationDirPath() + "/../Resources/translations";
#elif defined(Q_OS_WIN)
    translationPath = qApp->applicationDirPath() + "/translations";
#else // linux
    translationPath = qApp->applicationDirPath() + "/../share/cockatrice/translations";
#endif

    // Command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Cockatrice Client");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions(
        {{{"c", "connect"}, QCoreApplication::translate("main", "Connect on startup"), "user:pass@host:port"},
         {{"d", "debug-output"}, QCoreApplication::translate("main", "Debug to file")},
         {"simulate-crash", QCoreApplication::translate(
                                "main", "Show the crash-report dialog (saves a mock file when no DSN is configured)")},
         {"simulate-native-crash",
          QCoreApplication::translate("main", "Crash the app on purpose to test native crash capture (only captured "
                                              "with a DSN configured and crash reporting not off)")}});

    parser.process(app);

    if (parser.isSet("debug-output")) {
        Logger::getInstance().logToFile(true);
    }

    // --- Handle files or URLs passed at startup ---
    // Only positional arguments are treated as files/URLs, so options like
    // --connect are never handed off to another instance.
    const QStringList startupFiles = parser.positionalArguments();
    const bool hasActivationFiles = !startupFiles.isEmpty();

    SingleInstanceManager instance;

    if (hasActivationFiles) {
        QStringList redactedFiles;
        redactedFiles.reserve(startupFiles.size());
        for (const QString &file : startupFiles) {
            redactedFiles.append(redactActivationUrl(file));
        }
        qCInfo(MainLog) << "Activation launch, files:" << redactedFiles;
        // Activation launch: hand off to the primary instance if one is
        // running, otherwise become the primary ourselves. Do this before
        // constructing the main window so a hand-off exits cheaply.
        if (!instance.tryRun(startupFiles)) {
            qInfo() << "Handed off to a running instance, exiting";
            // Sent successfully → exit
            return 0;
        }
        // No primary instance → become server
        qInfo() << "No existing instance found, becoming primary instance";
    } else {
        // Plain launch: if another instance is running, run independently
        // instead of handing off and exiting.
        if (!instance.tryRun(QStringList())) {
            // Another instance is already running → just run independently
            qInfo() << "Another instance exists, running independently";
        } else {
            qInfo() << "No existing instance found, starting server";
        }
    }

    // Unclean-exit detection (cross-platform QLockFile session tracking, replacing the
    // legacy Windows-only .dmp scan): a leftover session lock means the previous run died
    // without reaching endSession(). Our own lock is held until the event loop finishes.
    // Deliberately placed after parser.process() (--help/--version exit inside it and must
    // not leak a lock) and after the instance hand-off (a handed-off secondary exits
    // immediately and must never take a lock). Note this counts any death without
    // endSession() as a crash, including SIGTERM at logout, Task Manager kills and
    // session-end shutdowns; in AlwaysSend those send a report too.
    const bool uncleanExit = crashReporter.beginSession(crashDatabasePath);

    // Deliberate native fault to verify crash capture (breakpad via sentry-native) end to
    // end. Fires here, after the SDK is armed and the session lock is claimed, so the
    // backend captures a real minidump and the leftover lock survives (the fault skips
    // unwinding, so ~CrashReporter never runs to release it). Run WITHOUT a debugger
    // attached: a debugger swallows the first-chance exception before the in-process
    // handler sees it. The volatile write defeats optimizer elision of the null dereference
    // — and, just as usefully, GCC's -Wnull-dereference, which fires on the plain form at
    // -O1+ and would break the -Werror build.
    //
    // What the leftover lock actually does depends on the mode: Ask (the default) shows the
    // crash dialog on the next launch as described, AlwaysSend flushes the cached envelope
    // silently at init instead, and NeverSend ignores the crash entirely. Not usable with
    // ENABLE_SANITIZERS either: CrashReporting.cmake forces the backend to "none" there and
    // ASan intercepts the signal first. Under AlwaysSend the report lands in the real
    // project with nothing marking it as deliberate. Takes precedence over
    // --simulate-crash: this fault happens here, so the dialog block further down is
    // never reached.
    if (parser.isSet("simulate-native-crash")) {
        qCWarning(MainLog) << "simulate-native-crash: raising an intentional access violation";
        if (!crashReporter.isReady()) {
            // Necessary but not sufficient: ready is true with ENABLE_SANITIZERS, where the
            // backend is "none". Best the log can do is keep the no-op from being silent.
            qCWarning(MainLog) << "simulate-native-crash: no crash backend armed (empty DSN, "
                                  "crash reporting off, or built without sentry-native) - the fault "
                                  "below cannot be captured";
        }
        volatile int *nullPointer = nullptr;
        *nullPointer = 1;
    }

    rng = new RNG_SFMT(CryptoUtil::randomUInt64());
    themeManager = new ThemeManager;
    soundEngine = new SoundEngine;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    QLocale::setDefault(QLocale::English);

    // Dependency Injections
    CardDatabaseManager::setCardPreferenceProvider(new SettingsCardPreferenceProvider());
    CardDatabaseManager::setCardDatabasePathProvider(&SettingsCache::instance());
    CardDatabaseManager::setCardSetPriorityController(&SettingsCache::instance().cardDatabase());

    qCInfo(MainLog) << "Starting main program";

    // Front-load the card database before constructing the main window. The
    // binary-cache read is cheap when uncontended (~1s); doing it here -- while no
    // GUI work yet competes for CPU -- avoids the multi-second, GUI-freezing
    // contention that happens when the load runs alongside window construction.
    // The CardDatabaseModel populates from the already-loaded data in its
    // constructor, so the window appears fully populated with no startup lag.
    // Note: checkUnknownSets() is deliberately not run for this front-loaded
    // parse. It would run before MainWindow exists, so
    // cardDatabaseNewSetsFound / cardDatabaseAllNewSetsEnabled would have no
    // receivers, and enabling the sets up-front would suppress the first-run
    // experience MainWindow::startupConfigCheck() and the onboarding wizard
    // provide. Callers that load after startup own the check.
    CardDatabaseManager::getInstance()->loadCardDatabases();

    MainWindow ui;

    // A URL launch must own the connection: the intent chain triggered by the
    // URL connects to the server named in the URL, so the window's own startup
    // auto-connect must not race against it (two connectToServer calls tear
    // each other down via doDisconnectFromServer).
    bool hasUrlActivation = std::any_of(startupFiles.begin(), startupFiles.end(), [](const QString &file) {
        return file.startsWith(QStringLiteral("cockatrice://"));
    });
#ifdef Q_OS_MAC
    // On macOS the launch can arrive through the URL scheme instead of as a
    // positional argument (captured in pendingMacUrls); count those too or the
    // window would auto-connect into the link's own connection attempt.
    hasUrlActivation = hasUrlActivation ||
                       std::any_of(pendingMacUrls.cbegin(), pendingMacUrls.cend(),
                                   [](const QString &url) { return url.startsWith(QStringLiteral("cockatrice://")); });
#endif
    ui.setSkipStartupAutoConnect(hasUrlActivation);

    auto handleActivation = [&ui](const QString &file) {
        if (file.startsWith("cockatrice://")) {
            qCInfo(MainLog) << "Handling URL activation:" << redactActivationUrl(file);
            // Route through the window's persistent url parser: it serializes
            // link chains so activations handed over while another chain is
            // still connecting do not connect concurrently.
            ui.handleCockatriceLink(file);
        } else if (QFileInfo(file).exists()) {
            auto openDeckIntent = new IntentOpenLocalDeck(ui.getTabSupervisor(), file);
            QObject::connect(openDeckIntent, &Intent::failed, &ui, [&ui](const QString &reason) {
                QMessageBox::warning(&ui, QObject::tr("Open deck"), reason);
            });
            openDeckIntent->execute();
        }
    };

#ifdef Q_OS_MAC
    QObject::disconnect(cocoaBufferConn);

    QObject::connect(&cockatriceFilter, &UrlSchemeEventFilter::urlReceived,
                     [&handleActivation](const QString &url) { handleActivation(url); });
#endif
    if (parser.isSet("connect")) {
        ui.setConnectTo(parser.value("connect"));
    }
    qCInfo(MainLog) << "MainWindow constructor finished";

    ui.setWindowIcon(themePixmap(QStringLiteral("cockatrice")));
    // Base name of the desktop file installed next to the binary, without the ".desktop"
    // extension. Wayland reads the window icon from it, and xdg-desktop-portal looks up
    // org.freedesktop.host.portal.Registry by this app ID, so it has to match the file name
    // exactly, down to the case.
    QGuiApplication::setDesktopFileName("cockatrice");

    SettingsCache::instance().network().setClientID(generateClientID());

    // If spoiler mode is enabled, we will download the spoilers
    // then reload the DB. otherwise just reload the DB
    SpoilerBackgroundUpdater spoilerBackgroundUpdater;

    ui.show();
    qCInfo(MainLog) << "ui.show() finished";

    // Crash-report flow: the trigger is --simulate-crash or an unclean previous exit
    // (leftover session lock; the old %LOCALAPPDATA%/CrashDumps .dmp scan is gone with
    // the exception filter). See runCrashReportFlow() for the per-mode handling.
    if (parser.isSet("simulate-crash") || uncleanExit) {
        runCrashReportFlow(crashReporter, &ui, crashLogAttachedEnvelopes);
    }

    // force shortcuts to be shown/hidden in right-click menus, regardless of system defaults
    qApp->setAttribute(Qt::AA_DontShowShortcutsInContextMenus,
                       !SettingsCache::instance().userInterface().getShowShortcuts());

#ifdef Q_OS_MAC
    for (const QString &url : pendingMacUrls) {
        handleActivation(url);
    }
    pendingMacUrls.clear();
#endif

    for (const QString &file : startupFiles) {
        handleActivation(file);
    }

    // Connect to future file/URL events from other instances
    QObject::connect(&instance, &SingleInstanceManager::filesReceived, [&handleActivation](const QStringList &files) {
        for (const QString &file : files) {
            handleActivation(file);
        }
    });

    int ret = app.exec();

    qCInfo(MainLog) << "Event loop finished, terminating...";
    delete rng;
    PingPixmapGenerator::clear();
    CountryPixmapGenerator::clear();
    UserLevelPixmapGenerator::clear();
    ManaSymbolPixmapGenerator::clear();

    // Released last: crashes anywhere in the teardown above (a common place for Qt apps to
    // crash) are still captured by the SDK and detected as unclean exits on the next launch.
    crashReporter.shutdown();
    crashReporter.endSession();

    return ret;
}
