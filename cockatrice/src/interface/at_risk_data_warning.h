#ifndef AT_RISK_DATA_WARNING_H
#define AT_RISK_DATA_WARNING_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QtGlobal>

class QWidget;

/**
 * @brief Warns when user data lives inside the program directory and backs it up first.
 *
 * The uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete the whole install
 * tree, so data paths configured inside the program directory get a safety
 * backup outside the install directory before the warning is shown.
 * Portable installs keep their data inside the program directory by design but
 * never shipped an uninstaller, so they are skipped.
 */
class AtRiskDataWarning
{
    Q_DECLARE_TR_FUNCTIONS(AtRiskDataWarning)

public:
    struct AtRiskBackupOutcome
    {
        QStringList locations;         ///< backup targets created for this check
        QString sessionDir;            ///< session directory holding the backup
        bool complete = true;          ///< false when only a partial/fallback backup exists
        bool reused = false;           ///< true when a recent session was reused instead of copying
        bool skippedDueToSize = false; ///< true when the automatic copy was skipped for size
        qint64 bytes = 0;              ///< estimated bytes, only set when a fresh copy was attempted
    };

    /**
     * @brief Backs up at-risk data paths and warns about them.
     * @param parent Parent widget for the warning dialog
     */
    static void warnAboutDataPathsInProgramDir(QWidget *parent);

private:
    static QStringList collectInstalledAtRiskPaths(const QString &appDir);
    static AtRiskBackupOutcome backUpAtRiskPaths(const QStringList &sources, const QString &appDir);
    static QString buildInstalledAtRiskMessage(const QStringList &offenders,
                                               const QStringList &sources,
                                               const AtRiskBackupOutcome &outcome);
    static void showAtRiskDataWarning(QWidget *parent, const QString &title, const QString &message);
};

#endif // AT_RISK_DATA_WARNING_H
