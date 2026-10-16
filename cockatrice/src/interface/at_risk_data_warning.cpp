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

Q_LOGGING_CATEGORY(AtRiskDataWarningLog, "at_risk_data_warning");

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

    // Portable installs keep their data inside the program directory by design, but they
    // never shipped an uninstaller, and portable updates use the installer's payload
    // purge, which preserves data/ - so there is nothing destructive to warn about.
    if (SettingsCache::instance().getIsPortableBuild()) {
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();

    // Sources to back up outside the install directory before warning. The uninstallers
    // shipped in 3.1.0/3.1.1 delete the whole install tree, so a safety copy is kept
    // even though the fixed uninstaller preserves foreign files - the destructive one
    // is already on disk on affected machines.
    const QStringList offenders = collectInstalledAtRiskPaths(appDir);
    if (offenders.isEmpty()) {
        return;
    }
    // Only existing paths can be copied; a configured-but-never-created path has
    // nothing to lose yet and must not turn the warning into a bogus "failed".
    QStringList sourcesToBackUp;
    for (const QString &offender : offenders) {
        if (QFileInfo::exists(offender)) {
            sourcesToBackUp.append(offender);
        }
    }

    const AtRiskBackupOutcome outcome = backUpAtRiskPaths(sourcesToBackUp, appDir);

    const QString message = buildInstalledAtRiskMessage(offenders, sourcesToBackUp, outcome);
    showAtRiskDataWarning(parent, tr("Data paths inside the program directory"), message);
}
