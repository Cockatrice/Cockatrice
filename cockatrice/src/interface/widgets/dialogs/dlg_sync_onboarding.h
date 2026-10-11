/**
 * @file dlg_sync_onboarding.h
 * @brief First-sync choice when local decks and server decks meet.
 */

#ifndef DLG_SYNC_ONBOARDING_H
#define DLG_SYNC_ONBOARDING_H

#include "../../deck_sync/deck_sync_conflict.h"

#include <QDialog>

class QLabel;
class QRadioButton;

/**
 * @class DlgSyncOnboarding
 * @brief Asks how to combine a non-empty local collection with a non-empty server collection.
 *
 * Merge (two-way) is the default. Upload and download restrict the run to one
 * direction; keeping separate records the profile as sync-disabled.
 * Rejected dialogs cancel the sync without recording anything.
 */
class DlgSyncOnboarding : public QDialog
{
    Q_OBJECT
public:
    DlgSyncOnboarding(int _localCount, int _serverCount, QWidget *parent = nullptr);
    [[nodiscard]] DeckSyncMode selectedMode() const;
    void retranslateUi();

private:
    int localCount = 0;
    int serverCount = 0;

    QLabel *detailLabel;
    QRadioButton *mergeRadio;
    QRadioButton *uploadRadio;
    QRadioButton *downloadRadio;
    QRadioButton *separateRadio;
};

#endif // DLG_SYNC_ONBOARDING_H
