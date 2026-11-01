#include "dlg_deck_history.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/deck_list/deck_diff.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_download.pb.h>
#include <libcockatrice/protocol/pb/command_deck_history_list.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pb/response_deck_history_list.pb.h>
#include <libcockatrice/protocol/pending_command.h>

namespace
{
QString formatVersionTime(qint64 secsSinceEpoch)
{
    return QLocale().toString(QDateTime::fromSecsSinceEpoch(secsSinceEpoch), QLocale::ShortFormat);
}
} // namespace

DlgDeckHistory::DlgDeckHistory(int _deckId,
                               const QString &_deckName,
                               int _limit,
                               AbstractClient *_client,
                               QWidget *parent)
    : QDialog(parent), deckId(_deckId), deckName(_deckName), limit(_limit), client(_client)
{
    auto *layout = new QVBoxLayout(this);

    detailLabel = new QLabel(this);
    detailLabel->setWordWrap(true);
    versionsList = new QListWidget(this);
    previewList = new QListWidget(this);
    restoreButton = new QPushButton(this);
    restoreButton->setEnabled(false);

    layout->addWidget(detailLabel);
    layout->addWidget(versionsList);
    layout->addWidget(previewList);
    layout->addWidget(restoreButton);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(versionsList, &QListWidget::currentRowChanged, this, &DlgDeckHistory::onSelectionChanged);
    connect(restoreButton, &QPushButton::clicked, this, &DlgDeckHistory::onRestoreClicked);

    Command_DeckHistoryList listCmd;
    listCmd.set_deck_id(deckId);
    listCmd.set_limit(static_cast<google::protobuf::uint32>(limit));
    PendingCommand *listPend = AbstractClient::prepareSessionCommand(listCmd);
    connect(listPend, &PendingCommand::finished, this, &DlgDeckHistory::onHistoryFinished);
    client->sendCommand(listPend);

    Command_DeckDownload downloadCmd;
    downloadCmd.set_deck_id(deckId);
    PendingCommand *downloadPend = AbstractClient::prepareSessionCommand(downloadCmd);
    connect(downloadPend, &PendingCommand::finished, this, &DlgDeckHistory::onCurrentFinished);
    client->sendCommand(downloadPend);

    retranslateUi();
}

void DlgDeckHistory::retranslateUi()
{
    setWindowTitle(tr("Deck history"));
    detailLabel->setText(
        tr("Stored versions of %1 (newest first). Restoring uploads the version as new content.").arg(deckName));
    restoreButton->setText(tr("Restore selected version"));
    renderVersions();
    renderPreview();
}

void DlgDeckHistory::onHistoryFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (response.response_code() != Response::RespOk) {
        detailLabel->setText(tr("Could not load version history."));
        return;
    }
    const Response_DeckHistoryList &list = response.GetExtension(Response_DeckHistoryList::ext);
    versions.clear();
    for (const DeckVersionInfo &version : list.versions()) {
        DeckHistoryVersion entry;
        entry.versionNo = version.version_no();
        entry.createdAtSecs = static_cast<qint64>(version.created_at());
        entry.createdByClientId = QString::fromStdString(version.created_by_clientid());
        entry.hash = QString::fromStdString(version.hash());
        entry.content = QString::fromStdString(version.content());
        versions.append(entry);
    }
    historyArrived = true;
    renderVersions();
    renderPreview();
}

void DlgDeckHistory::onCurrentFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (response.response_code() != Response::RespOk) {
        detailLabel->setText(tr("Could not load the current deck."));
        return;
    }
    const Response_DeckDownload &download = response.GetExtension(Response_DeckDownload::ext);
    currentContent = QString::fromStdString(download.deck());
    currentArrived = true;
    renderPreview();
}

void DlgDeckHistory::renderVersions()
{
    const int row = versionsList->currentRow();
    versionsList->clear();
    for (const DeckHistoryVersion &version : versions) {
        QString label = tr("Version %1 — %2").arg(version.versionNo).arg(formatVersionTime(version.createdAtSecs));
        if (!version.createdByClientId.isEmpty()) {
            label += tr(" — %1").arg(version.createdByClientId);
        }
        versionsList->addItem(label);
    }
    if (row >= 0 && row < versionsList->count()) {
        versionsList->setCurrentRow(row);
    } else if (versionsList->count() > 0) {
        versionsList->setCurrentRow(0);
    }
    onSelectionChanged();
}

void DlgDeckHistory::renderPreview()
{
    previewList->clear();
    restoreButton->setEnabled(false);
    const int row = versionsList->currentRow();
    if (!historyArrived || !currentArrived || row < 0 || row >= versions.size()) {
        return;
    }
    DeckList currentDeck;
    DeckList selectedDeck;
    if (!currentDeck.loadFromString_Native(currentContent) ||
        !selectedDeck.loadFromString_Native(versions.at(row).content)) {
        return;
    }
    const DeckDiffResult shown = DeckDiff::diff(currentDeck, selectedDeck);
    for (const DeckCardChange &change : shown.changes) {
        previewList->addItem(
            tr("%1: %2 %3 → %4").arg(change.zone).arg(change.cardName).arg(change.oldCount).arg(change.newCount));
    }
    restoreButton->setEnabled(true);
}

void DlgDeckHistory::onSelectionChanged()
{
    renderPreview();
}

void DlgDeckHistory::onRestoreClicked()
{
    const int row = versionsList->currentRow();
    if (row < 0 || row >= versions.size()) {
        return;
    }
    if (QMessageBox::question(this, tr("Restore version"),
                              tr("Restore version %1 as the current deck content? This becomes a new version.")
                                  .arg(versions.at(row).versionNo)) != QMessageBox::Yes) {
        return;
    }
    emit restoreRequested(deckId, versions.at(row).content);
    accept();
}
