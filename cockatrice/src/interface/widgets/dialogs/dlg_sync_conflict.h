/**
 * @file dlg_sync_conflict.h
 * @brief Dialog for resolving a deck that changed both locally and on the server.
 */

#ifndef DLG_SYNC_CONFLICT_H
#define DLG_SYNC_CONFLICT_H

#include "../../deck_sync/deck_sync_conflict.h"

#include <QDialog>
#include <QString>
#include <libcockatrice/deck_list/deck_diff.h>

class QCheckBox;
class QLabel;
class QListWidget;
class QRadioButton;

/**
 * @class DlgSyncConflict
 * @brief Shows both versions of a conflicted deck and returns the verdict.
 *
 * Radio choice between Keep mine / Keep server (newest preselected), Keep both
 * and Use merge (enabled only when the changes touch disjoint cards). The
 * change list shows card-count deltas between the two versions; the merge
 * summary describes the auto-merge proposal. Rejected dialogs resolve as Skip.
 */
class DlgSyncConflict : public QDialog
{
    Q_OBJECT
public:
    explicit DlgSyncConflict(const DeckSyncConflict &conflict, QWidget *parent = nullptr);
    [[nodiscard]] DeckSyncResolution resolution() const;
    void retranslateUi();

private slots:
    void updateRememberBoxes();

private:
    DeckSyncResolutionAction selectedAction() const;

    DeckSyncConflict conflict;
    bool decksParsed = false;
    bool localIsNewer = true;
    DeckDiffResult shownChanges;
    bool mergeable = false;
    int mergedMainCount = 0;
    int mergedSideCount = 0;

    QLabel *titleLabel;
    QLabel *detailLabel;
    QRadioButton *keepMineRadio;
    QRadioButton *keepServerRadio;
    QRadioButton *keepBothRadio;
    QRadioButton *mergeRadio;
    QListWidget *changesList;
    QLabel *mergeSummaryLabel;
    QCheckBox *rememberSessionBox;
    QCheckBox *rememberAlwaysBox;
};

#endif // DLG_SYNC_CONFLICT_H
