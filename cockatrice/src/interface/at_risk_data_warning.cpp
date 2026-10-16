#include "at_risk_data_warning.h"

#include "../client/settings/cache_settings.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QWidget>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/utility/user_data_backup.h>

inline Q_LOGGING_CATEGORY(AtRiskDataWarningLog, "main_window");

static bool isDeckFileSuffix(const QString &suffix)
{
    return suffix == QStringLiteral("cod") || suffix == QStringLiteral("dec") || suffix == QStringLiteral("dek") ||
           suffix == QStringLiteral("mwdeck");
}

static bool isInsideAnyDir(const QString &path, const QStringList &bases)
{
    for (const QString &base : bases) {
        if (PathsSettings::isInsideDir(path, base)) {
            return true;
        }
    }
    return false;
}

// Merges an at-risk offender into the backup source list: offenders already covered by a
// collected source are skipped, while an offender that is a parent of collected sources
// supersedes them.
static void mergeCoveringSource(QStringList &sources, const QString &offender)
{
    if (isInsideAnyDir(offender, sources)) {
        return;
    }
    for (int i = sources.size() - 1; i >= 0; --i) {
        if (PathsSettings::isInsideDir(sources.at(i), offender)) {
            sources.removeAt(i);
        }
    }
    sources.append(offender);
}

static bool hasLooseDeckFile(const QStringList &offenders)
{
    for (const QString &offender : offenders) {
        if (isDeckFileSuffix(QFileInfo(offender).suffix().toLower())) {
            return true;
        }
    }
    return false;
}

static QString resolveBackupRoot(const QString &appDir)
{
    QString backupRoot = UserDataBackup::defaultBackupRoot();
    if (PathsSettings::isInsideDir(backupRoot, appDir)) {
        backupRoot = QDir::temp().absoluteFilePath(QStringLiteral("CockatriceBackups"));
    }
    return backupRoot;
}

// True when a portable offender must stay out of the safety backup: replays (size),
// downloaded pictures (large and re-downloadable - everything under pics/ except the
// seeded CUSTOM dir) and the regenerable redirect/cache dir.
static bool isPortableBackupExcluded(const QString &offender)
{
    auto &paths = SettingsCache::instance().paths();
    const QString replaysPath = QDir::cleanPath(paths.getReplaysPath());
    const QString picsPath = QDir::cleanPath(paths.getPicsPath());
    const QString customPicsPath = QDir::cleanPath(paths.getCustomPicsPath());
    const QString redirectsPath = QDir::cleanPath(paths.getRedirectCachePath());
    const QString cacheDir = QDir::cleanPath(SettingsCache::instance().getCachePath());

    if (!replaysPath.isEmpty() && PathsSettings::isInsideDir(offender, replaysPath)) {
        return true;
    }
    if (!picsPath.isEmpty() && PathsSettings::isInsideDir(offender, picsPath) &&
        (customPicsPath.isEmpty() || !PathsSettings::isInsideDir(offender, customPicsPath))) {
        return true;
    }
    if (!redirectsPath.isEmpty() && PathsSettings::isInsideDir(offender, redirectsPath)) {
        return true;
    }
    if (!cacheDir.isEmpty() && PathsSettings::isInsideDir(offender, cacheDir)) {
        return true;
    }
    return false;
}

AtRiskDataWarning::AtRiskPaths AtRiskDataWarning::collectPortableAtRiskPaths(const QString &appDir)
{
    auto &paths = SettingsCache::instance().paths();
    AtRiskPaths atRisk;

    // Portable data lives inside the install directory by design, so there is
    // nothing to reconfigure. Back up the small irreplaceable data instead.
    // Downloaded pictures are deliberately excluded: they are large and can
    // always be re-downloaded (the installer's pre-uninstall backup drops
    // them for the same reason). Replays are excluded here for size; the
    // installer backup covers them during installer-driven upgrades, but a
    // manual run of the old uninstall.exe is not covered - hence the note
    // in the warning text below.
    const QString dataDir = SettingsCache::instance().getDataPath();
    for (const QString &sub :
         QStringList{QStringLiteral("decks"), QStringLiteral("customsets"), QStringLiteral("settings"),
                     QStringLiteral("filters"), QStringLiteral("themes"), QStringLiteral("sounds"),
                     QStringLiteral("pics/CUSTOM")}) {
        const QString candidate = QDir::cleanPath(QDir(dataDir).absoluteFilePath(sub));
        if (QFileInfo::exists(candidate)) {
            atRisk.sourcesToBackUp.append(candidate);
        }
    }

    // Card database files live in dataDir by default but are not in the list above. Seed them
    // explicitly so they do not depend on the offender merge below to be backed up.
    for (const QString &dbFile :
         QStringList{paths.getCardDatabasePath(), paths.getTokenDatabasePath(), paths.getSpoilerCardDatabasePath()}) {
        if (dbFile.isEmpty()) {
            continue;
        }
        const QString cleanedDbFile = QDir::cleanPath(dbFile);
        if (!QFileInfo::exists(cleanedDbFile) || !PathsSettings::isInsideDir(cleanedDbFile, appDir)) {
            continue;
        }
        // isInsideDir is equality-aware and case-insensitive on Windows/macOS,
        // so it also covers exact duplicates with different casing or separators.
        if (!isInsideAnyDir(cleanedDbFile, atRisk.sourcesToBackUp)) {
            atRisk.sourcesToBackUp.append(cleanedDbFile);
        }
    }

    // A portable user may also have pointed a custom path inside the install
    // dir. Offenders outside data/ (e.g. appDir/mydecks) would be wiped by the
    // old uninstaller but are not in the hardcoded list above - and neither
    // is a custom folder inside data/ itself (e.g. dataDir/mydecks), which the
    // old uninstaller deletes all the same. Cover any offender that is not
    // already inside a backed-up source; when an offender is a parent of
    // already-collected sources it supersedes them.
    for (const QString &offender : paths.pathsInsideDir(appDir)) {
        if (!QFileInfo::exists(offender)) {
            continue;
        }
        const QString cleanedOffender = QDir::cleanPath(offender);
        if (isPortableBackupExcluded(cleanedOffender)) {
            continue;
        }
        mergeCoveringSource(atRisk.sourcesToBackUp, cleanedOffender);
    }

    // A fresh portable install has no data on disk yet, but the user still
    // needs to know their data will live where the 3.1.0/3.1.1 uninstaller
    // deletes it - so fall through to the warning with an empty backup
    // instead of returning silently.
    if (atRisk.sourcesToBackUp.isEmpty()) {
        qCInfo(AtRiskDataWarningLog) << "Portable install with no existing data files, warning without backup";
    }
    return atRisk;
}

QStringList AtRiskDataWarning::collectInstalledAtRiskPaths(const QString &appDir)
{
    QStringList offenders = SettingsCache::instance().paths().pathsInsideDir(appDir);

    // Loose decks sitting directly in the program folder are wiped by the old uninstaller
    // just like configured paths. Mirror the installer backup set (DeckLoader extensions
    // minus *.txt) so a manual run of the old uninstall.exe is covered too.
    const QDir appDirHandle(appDir);
    const QStringList topLevelFiles = appDirHandle.entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    for (const QString &fileName : topLevelFiles) {
        if (!isDeckFileSuffix(QFileInfo(fileName).suffix().toLower())) {
            continue;
        }
        const QString looseDeck = QDir::cleanPath(appDirHandle.absoluteFilePath(fileName));
        if (!isInsideAnyDir(looseDeck, offenders)) {
            offenders.append(looseDeck);
        }
    }
    return offenders;
}

AtRiskDataWarning::AtRiskBackupOutcome AtRiskDataWarning::backUpAtRiskPaths(const QStringList &sources,
                                                                            const QString &appDir)
{
    AtRiskBackupOutcome outcome;
    const QString backupRoot = resolveBackupRoot(appDir);

    // Throttle: dismissing the dialog without "Don't show again" must not create
    // a full duplicate on every launch. A recent session is reused only when it is
    // complete and its source manifest covers the current source set; otherwise a
    // fresh copy is taken so the dialog never claims an uncovered path is backed up.
    // The size estimate runs only when a fresh copy is actually going to happen,
    // keeping a synchronous tree walk off the reuse fast path at startup.
    // Search all recent sessions for a complete, covering one: a partial failure must not
    // pin the newest slot and force a full re-copy on every later launch.
    const QString reusableSession = UserDataBackup::findRecentUsableBackupSession(backupRoot, sources, 24);
    if (!reusableSession.isEmpty()) {
        outcome.sessionDir = reusableSession;
        outcome.locations = UserDataBackup::listSessionBackupLocations(reusableSession);
        outcome.reused = true;
        qCInfo(AtRiskDataWarningLog) << "Reusing recent at-risk user data backup at" << outcome.sessionDir;
    } else if (!sources.isEmpty()) {
        // The copy runs synchronously on the UI thread at startup, so cap it: a
        // pictures folder pointed into the program dir can be gigabytes, and freezing
        // the client to duplicate it is worse than asking for a manual copy.
        // Portable sources are small by construction (downloaded pictures and replays
        // are excluded above); this guard bites for non-portable offenders.
        outcome.bytes = UserDataBackup::estimateBackupSize(sources);
        if (outcome.bytes >= UserDataBackup::MAX_AUTO_BACKUP_BYTES) {
            outcome.skippedDueToSize = true;
            qCWarning(AtRiskDataWarningLog)
                << "At-risk user data backup skipped: estimated" << outcome.bytes << "bytes exceeds automatic cap";
        } else {
            // One timestamped session directory for all sources, so a second boundary
            // never splits a single startup check across two backup folders and
            // same-named sources cannot overwrite each other.
            outcome.complete =
                UserDataBackup::backupAtRiskPaths(sources, backupRoot, &outcome.locations, &outcome.sessionDir);
            UserDataBackup::pruneOldBackupSessions(backupRoot, 5);
            if (outcome.locations.isEmpty()) {
                // Fresh copy failed entirely: fall back to the newest covering session so the
                // dialog still points at something, flagged incomplete below.
                const QString fallbackSession =
                    UserDataBackup::findRecentCoveringBackupSession(backupRoot, sources, 24);
                if (!fallbackSession.isEmpty()) {
                    outcome.sessionDir = fallbackSession;
                    outcome.locations = UserDataBackup::listSessionBackupLocations(fallbackSession);
                    outcome.complete = false;
                }
            }
        }
    }
    for (const QString &backupLocation : outcome.locations) {
        qCInfo(AtRiskDataWarningLog) << "Backed up at-risk user data to" << backupLocation;
    }
    if (!outcome.complete) {
        qCWarning(AtRiskDataWarningLog) << "At-risk user data backup incomplete, session dir:" << outcome.sessionDir;
    }
    return outcome;
}

QString AtRiskDataWarning::buildPortableAtRiskMessage(const AtRiskBackupOutcome &outcome, bool emptySources)
{
    const QString backupVerb = outcome.reused ? tr("is already available") : tr("has been created");
    const QString sizeInfo = tr("%1 MB (automatic backups are capped at %2 MB)")
                                 .arg(QString::number(outcome.bytes / 1048576.0, 'f', 1),
                                      QString::number(UserDataBackup::MAX_AUTO_BACKUP_BYTES / 1048576));
    if (!outcome.locations.isEmpty()) {
        QString message =
            tr("This portable install keeps its data inside the program directory, where the "
               "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
               "update.\n\nA safety backup of your decks, custom sets, settings, filters, themes, sounds and "
               "custom "
               "pictures %1 "
               "at:\n%2\n\nReplays and downloaded pictures are not included in this backup. Uninstalling "
               "with the old uninstaller still deletes the program "
               "directory, so keep your own copy of that backup until you have updated once.")
                .arg(backupVerb, outcome.locations.join(QLatin1Char('\n')));
        if (!outcome.complete) {
            message += tr("\n\nThe backup may be incomplete; please back up these folders manually as well.");
        }
        return message;
    }
    if (outcome.skippedDueToSize) {
        return tr("This portable install keeps its data inside the program directory, where the "
                  "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
                  "update.\n\nThe automatic safety backup was skipped because of its size: %1. "
                  "Please back up your data folder manually "
                  "before uninstalling or updating.\n\nReplays and downloaded pictures would not have been "
                  "included in that backup either.")
            .arg(sizeInfo);
    }
    if (emptySources) {
        return tr("This portable install keeps its data inside the program directory, where the "
                  "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
                  "update.\n\nNone of your data files exist on disk yet, so there was nothing to back up. "
                  "Files created there later are still at risk until you have updated once.");
    }
    return tr("This portable install keeps its data inside the program directory, where the "
              "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
              "update.\n\nThe automatic safety backup failed; please back up your data folder manually "
              "before uninstalling or updating.\n\nReplays and downloaded pictures would not have been "
              "included in that backup either.");
}

QString AtRiskDataWarning::buildInstalledAtRiskMessage(const QStringList &offenders,
                                                       const QStringList &sources,
                                                       const AtRiskBackupOutcome &outcome)
{
    const QString backupVerb = outcome.reused ? tr("is already available") : tr("has been created");
    const QString sizeInfo = tr("%1 MB (automatic backups are capped at %2 MB)")
                                 .arg(QString::number(outcome.bytes / 1048576.0, 'f', 1),
                                      QString::number(UserDataBackup::MAX_AUTO_BACKUP_BYTES / 1048576));
    QString message = tr("These paths are configured inside the Cockatrice program directory:\n\n%1\n\n"
                         "Cockatrice keeps its own data outside the program directory by default. Files "
                         "stored inside it can be deleted by an update or by uninstalling Cockatrice. "
                         "Change them in the Settings dialog - most are on the General page - to keep "
                         "them safe.")
                          .arg(offenders.join(QLatin1Char('\n')));
    if (!outcome.locations.isEmpty()) {
        message += tr("\n\nA safety backup %1 at:\n%2").arg(backupVerb, outcome.locations.join(QLatin1Char('\n')));
        if (!outcome.complete) {
            message += tr("\n\nThe backup may be incomplete; please back up these folders manually as well.");
        }
        if (hasLooseDeckFile(offenders)) {
            message += tr("\n\nDeck files ending in .txt are not backed up - move those manually.");
        }
    } else if (outcome.skippedDueToSize) {
        message += tr("\n\nThe automatic safety backup was skipped because of its size: %1. "
                      "Please back up these folders manually.")
                       .arg(sizeInfo);
    } else if (sources.isEmpty()) {
        message += tr("\n\nNone of these paths exist on disk yet, so there was nothing to back up. "
                      "Files created there later are still at risk until the paths are moved.");
    } else {
        message += tr("\n\nThe automatic safety backup failed; please back up these folders manually.");
    }
    return message;
}

void AtRiskDataWarning::showAtRiskDataWarning(QWidget *parent, const QString &title, const QString &message)
{
    QMessageBox box(QMessageBox::Warning, title, message, QMessageBox::Ok, parent);
    box.setTextFormat(Qt::PlainText);
    auto *dontShowAgain = new QCheckBox(tr("Don't show this again"));
    box.setCheckBox(dontShowAgain);
    box.exec();
    if (dontShowAgain->isChecked()) {
        SettingsCache::instance().paths().setAppDirWarningAcknowledged(true);
    }
}

void AtRiskDataWarning::warnAboutDataPathsInProgramDir(QWidget *parent)
{
    auto &paths = SettingsCache::instance().paths();
    if (paths.getAppDirWarningAcknowledged()) {
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const bool isPortable = SettingsCache::instance().getIsPortableBuild();

    // Sources to back up outside the install directory before warning. The uninstallers
    // shipped in 3.1.0/3.1.1 delete the whole install tree, so a safety copy is kept
    // even though the fixed uninstaller preserves foreign files - the destructive one
    // is already on disk on affected machines.
    AtRiskPaths atRisk;
    if (isPortable) {
        atRisk = collectPortableAtRiskPaths(appDir);
    } else {
        atRisk.offendersForDisplay = collectInstalledAtRiskPaths(appDir);
        if (atRisk.offendersForDisplay.isEmpty()) {
            return;
        }
        // Only existing paths can be copied; a configured-but-never-created path has
        // nothing to lose yet and must not turn the warning into a bogus "failed".
        for (const QString &existingOffender : atRisk.offendersForDisplay) {
            if (QFileInfo::exists(existingOffender)) {
                atRisk.sourcesToBackUp.append(existingOffender);
            }
        }
    }

    const AtRiskBackupOutcome outcome = backUpAtRiskPaths(atRisk.sourcesToBackUp, appDir);

    const QString title =
        isPortable ? tr("Portable data at risk on uninstall") : tr("Data paths inside the program directory");
    const QString message =
        isPortable ? buildPortableAtRiskMessage(outcome, atRisk.sourcesToBackUp.isEmpty())
                   : buildInstalledAtRiskMessage(atRisk.offendersForDisplay, atRisk.sourcesToBackUp, outcome);
    showAtRiskDataWarning(parent, title, message);
}
