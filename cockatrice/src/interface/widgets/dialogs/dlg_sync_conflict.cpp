#include "dlg_sync_conflict.h"

#include <QCheckBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFile>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QRadioButton>
#include <QVBoxLayout>
#include <libcockatrice/deck_list/deck_diff.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

namespace
{
QString formatTime(qint64 secsSinceEpoch)
{
    return QLocale().toString(QDateTime::fromSecsSinceEpoch(secsSinceEpoch), QLocale::ShortFormat);
}

int zoneCardCount(const DeckList &deck, const QString &zone)
{
    int count = 0;
    deck.forEachCard([&count, &zone](InnerDecklistNode *zoneNode, DecklistCardNode *card) {
        if (zoneNode->getName() == zone) {
            count += card->getNumber();
        }
    });
    return count;
}
} // namespace

DlgSyncConflict::DlgSyncConflict(const DeckSyncConflict &_conflict, QWidget *parent)
    : QDialog(parent), conflict(_conflict)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(this);
    detailLabel = new QLabel(this);
    detailLabel->setWordWrap(true);
    keepMineRadio = new QRadioButton(this);
    keepServerRadio = new QRadioButton(this);
    keepBothRadio = new QRadioButton(this);
    mergeRadio = new QRadioButton(this);
    changesList = new QListWidget(this);
    mergeSummaryLabel = new QLabel(this);
    mergeSummaryLabel->setWordWrap(true);
    rememberSessionBox = new QCheckBox(this);
    rememberAlwaysBox = new QCheckBox(this);

    layout->addWidget(titleLabel);
    layout->addWidget(detailLabel);
    layout->addWidget(keepMineRadio);
    layout->addWidget(keepServerRadio);
    layout->addWidget(keepBothRadio);
    layout->addWidget(mergeRadio);
    layout->addWidget(changesList);
    layout->addWidget(mergeSummaryLabel);
    layout->addWidget(rememberSessionBox);
    layout->addWidget(rememberAlwaysBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    DeckList localDeck;
    DeckList serverDeck;
    QFile localFile(conflict.localAbsolutePath);
    decksParsed = localFile.open(QIODevice::ReadOnly) && localDeck.loadFromFile_Native(&localFile) &&
                  serverDeck.loadFromString_Native(conflict.serverDeckString);
    localIsNewer = conflict.localSecs >= conflict.serverSecs;

    if (decksParsed) {
        shownChanges = DeckDiff::diff(localDeck, serverDeck);
        if (conflict.hasBase) {
            DeckList baseDeck;
            if (baseDeck.loadFromString_Native(conflict.baseDeckString)) {
                const DeckDiffResult localChanges = DeckDiff::diff(baseDeck, localDeck);
                const DeckDiffResult remoteChanges = DeckDiff::diff(baseDeck, serverDeck);
                mergeable = DeckDiff::canAutoMerge(localChanges, remoteChanges);
                if (mergeable) {
                    const DeckList merged = DeckDiff::merge(baseDeck, localChanges, remoteChanges);
                    mergedMainCount = zoneCardCount(merged, "main");
                    mergedSideCount = zoneCardCount(merged, "side");
                }
            }
        }
    }

    if (localIsNewer) {
        keepMineRadio->setChecked(true);
    } else {
        keepServerRadio->setChecked(true);
    }
    mergeRadio->setEnabled(mergeable);

    connect(keepMineRadio, &QRadioButton::toggled, this, &DlgSyncConflict::updateRememberBoxes);
    connect(keepServerRadio, &QRadioButton::toggled, this, &DlgSyncConflict::updateRememberBoxes);
    connect(keepBothRadio, &QRadioButton::toggled, this, &DlgSyncConflict::updateRememberBoxes);
    connect(mergeRadio, &QRadioButton::toggled, this, &DlgSyncConflict::updateRememberBoxes);
    updateRememberBoxes();
    retranslateUi();
}

void DlgSyncConflict::retranslateUi()
{
    setWindowTitle(tr("Sync conflict"));
    titleLabel->setText(conflict.relativePath);
    if (!decksParsed) {
        detailLabel->setText(tr("This deck changed on both sides, but one version could not be read."));
    } else {
        detailLabel->setText(tr("This deck changed both on this device and on the server."));
    }

    keepMineRadio->setText(
        tr("Keep my deck (%1%2)").arg(formatTime(conflict.localSecs), localIsNewer ? tr(", newest") : QString()));
    keepServerRadio->setText(tr("Keep the server deck (%1%2)")
                                 .arg(formatTime(conflict.serverSecs), localIsNewer ? QString() : tr(", newest")));
    keepBothRadio->setText(tr("Keep both (store the server version aside)"));
    mergeRadio->setText(mergeable ? tr("Use the auto-merge") : tr("Use the auto-merge (not available)"));

    changesList->clear();
    for (const DeckCardChange &change : shownChanges.changes) {
        changesList->addItem(
            tr("%1: %2 %3 → %4").arg(change.zone).arg(change.cardName).arg(change.oldCount).arg(change.newCount));
    }

    if (mergeable) {
        mergeSummaryLabel->setText(tr("Auto-merge proposal: union of both sides (%1 main, %2 side).")
                                       .arg(mergedMainCount)
                                       .arg(mergedSideCount));
    } else if (!conflict.hasBase) {
        mergeSummaryLabel->setText(tr("No base snapshot exists, so the versions cannot be merged automatically."));
    } else {
        mergeSummaryLabel->setText(tr("The changes overlap, so they cannot be merged automatically."));
    }

    rememberSessionBox->setText(tr("Remember for the rest of this sync"));
    rememberAlwaysBox->setText(tr("Always use this rule"));
}

void DlgSyncConflict::updateRememberBoxes()
{
    const bool single = keepMineRadio->isChecked() || keepServerRadio->isChecked();
    rememberSessionBox->setEnabled(single);
    rememberAlwaysBox->setEnabled(single);
    if (!single) {
        rememberSessionBox->setChecked(false);
        rememberAlwaysBox->setChecked(false);
    }
}

DeckSyncResolutionAction DlgSyncConflict::selectedAction() const
{
    if (keepServerRadio->isChecked()) {
        return DeckSyncKeepServer;
    }
    if (keepBothRadio->isChecked()) {
        return DeckSyncKeepBoth;
    }
    if (mergeRadio->isChecked()) {
        return DeckSyncUseMerge;
    }
    return DeckSyncKeepMine;
}

DeckSyncResolution DlgSyncConflict::resolution() const
{
    DeckSyncResolution result;
    result.action = selectedAction();
    result.rememberSession = rememberSessionBox->isEnabled() && rememberSessionBox->isChecked();
    result.rememberAlways = rememberAlwaysBox->isEnabled() && rememberAlwaysBox->isChecked();
    return result;
}
