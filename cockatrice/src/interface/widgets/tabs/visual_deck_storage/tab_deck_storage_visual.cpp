#include "tab_deck_storage_visual.h"

#include "../../../deck_loader/deck_loader.h"
#include "../../cards/additional_info/deck_color_identity.h"
#include "../../deck_share/deck_share_utils.h"
#include "../../interface/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "../tab_supervisor.h"

#include <QDateTime>
#include <QMessageBox>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_share_create.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_share_create.pb.h>
#include <libcockatrice/protocol/pending_command.h>

TabDeckStorageVisual::TabDeckStorageVisual(TabSupervisor *_tabSupervisor)
    : Tab(_tabSupervisor), visualDeckStorageWidget(new VisualDeckStorageWidget(this))
{
    connect(this, &TabDeckStorageVisual::openDeckEditor, tabSupervisor, &TabSupervisor::openDeckInNewTab);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::deckLoadRequested, this,
            &TabDeckStorageVisual::actOpenLocalDeck);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::openDeckEditor, this,
            &TabDeckStorageVisual::openDeckEditor);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::shareDeckRequested, this,
            &TabDeckStorageVisual::actShareDeck);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::shareSelectionChanged, this,
            &TabDeckStorageVisual::onShareSelectionChanged);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::shareRequested, this, [this] {
        if (shareDeckAvailable) {
            enterShareMode();
        }
    });

    AbstractClient *client = tabSupervisor->getClient();
    connect(client, &AbstractClient::statusChanged, this, &TabDeckStorageVisual::handleConnectionChanged);
    shareDeckAvailable = (client->getStatus() == StatusLoggedIn);
    visualDeckStorageWidget->setShareAvailable(shareDeckAvailable);

    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    widget->setLayout(layout);
    this->setCentralWidget(widget);
    layout->addWidget(visualDeckStorageWidget);

    shareBar = new ShareBarWidget(this);
    connect(shareBar, &ShareBarWidget::createRequested, this, &TabDeckStorageVisual::actShareSelected);
    connect(shareBar, &ShareBarWidget::cancelRequested, this, &TabDeckStorageVisual::exitShareMode);

    layout->insertWidget(0, shareBar);
    shareBar->setVisible(false);

    retranslateUi();
}

void TabDeckStorageVisual::retranslateUi()
{
    shareBar->retranslateUi();
}

void TabDeckStorageVisual::actOpenLocalDeck(const QString &filePath)
{
    std::optional<LoadedDeck> deckOpt =
        DeckLoader::loadFromFile(filePath, DeckFileFormat::getFormatFromName(filePath), true);
    if (!deckOpt) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(filePath));
        return;
    }

    emit openDeckEditor(deckOpt.value());
}

void TabDeckStorageVisual::enterShareMode(const QStringList &preselectFiles)
{
    if (!shareDeckAvailable) {
        return; // sharing is gated on being logged in
    }
    shareBar->setCreateEnabled(true);
    visualDeckStorageWidget->setShareSelectable(true);
    visualDeckStorageWidget->setShareSelectedFiles(preselectFiles);
    shareBar->setName(tr("Shared decks"));
    shareBar->setVisible(true);
    updateShareHint();
    shareBar->focusName();
}

void TabDeckStorageVisual::exitShareMode()
{
    visualDeckStorageWidget->setShareSelectable(false);
    visualDeckStorageWidget->clearShareSelection();
    shareBar->setVisible(false);
}

void TabDeckStorageVisual::actShareDeck(const QString &filePath)
{
    if (!shareDeckAvailable) {
        QMessageBox::information(this, tr("Share deck"), tr("You must be connected to the server to share a deck."));
        return;
    }
    enterShareMode({filePath});
}

void TabDeckStorageVisual::onShareSelectionChanged()
{
    if (shareBar->isVisible()) {
        updateShareHint();
    }
}

void TabDeckStorageVisual::updateShareHint()
{
    const int count = visualDeckStorageWidget->selectedFilePaths().size();
    shareBar->setCountText(tr("%n deck(s)", "", count));
    if (count == 0) {
        shareBar->setHintText(tr("Click deck tiles to select the decks you want to share."), true);
    } else {
        shareBar->setHintText(tr("%n deck(s) selected. Create the link to share %1 with other players.", "", count)
                                  .arg(count == 1 ? tr("it") : tr("them")),
                              true);
    }
}

void TabDeckStorageVisual::actShareSelected()
{
    const QStringList filePaths = visualDeckStorageWidget->selectedFilePaths();
    if (filePaths.isEmpty()) {
        QMessageBox::warning(this, tr("Share decks"), tr("Select at least one deck to share."));
        return;
    }

    Command_DeckShareCreate cmd;
    cmd.set_name(shareBar->name().toStdString());
    if (cmd.name().empty()) {
        cmd.set_name(tr("Shared decks").toStdString());
    }

    for (const QString &filePath : filePaths) {
        std::optional<LoadedDeck> deckOpt =
            DeckLoader::loadFromFile(filePath, DeckFileFormat::getFormatFromName(filePath), true);
        if (!deckOpt) {
            QMessageBox::warning(this, tr("Share decks"), tr("Unable to load deck file %1").arg(filePath));
            return;
        }
        DeckShareItem *item = cmd.add_items();
        item->set_deck_list(deckOpt->deckList.writeToString_Native().toStdString());
        item->set_color_identity(getDeckColorIdentity(deckOpt->deckList, CardDatabaseManager::query()).toStdString());
    }

    shareBar->setCreateEnabled(false);
    PendingCommand *pend = tabSupervisor->getClient()->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabDeckStorageVisual::shareFinished);
    tabSupervisor->getClient()->sendCommand(pend);
}

void TabDeckStorageVisual::shareFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    shareBar->setCreateEnabled(true);
    if (response.response_code() != Response::RespOk) {
        showShareNotice(tr("Failed to create the share link (server response code %1).")
                            .arg(QString::number(static_cast<int>(response.response_code()))),
                        true);
        return;
    }

    const Response_DeckShareCreate &resp = response.GetExtension(Response_DeckShareCreate::ext);
    const QString token = QString::fromStdString(resp.token());
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    const QDateTime expiry = QDateTime::fromSecsSinceEpoch(resp.expires_at(), QTimeZone::UTC);
#else
    const QDateTime expiry = QDateTime::fromSecsSinceEpoch(resp.expires_at(), Qt::UTC);
#endif

    const QString link = DeckShareUtils::buildShareLink(tabSupervisor->getClient(), token);
    DeckShareUtils::copyShareLinkToClipboard(link);

    showShareNotice(
        tr("Share link copied to the clipboard.\nExpires on %1.").arg(DeckShareUtils::formatShareExpiry(expiry)));
    exitShareMode();
}

void TabDeckStorageVisual::handleConnectionChanged(ClientStatus status)
{
    shareDeckAvailable = (status == StatusLoggedIn);
    visualDeckStorageWidget->setShareAvailable(shareDeckAvailable);
    if (!shareDeckAvailable && shareBar->isVisible()) {
        exitShareMode();
    }
}

void TabDeckStorageVisual::showShareNotice(const QString &message, bool warning)
{
    QMessageBox box(warning ? QMessageBox::Warning : QMessageBox::Information, tr("Deck share"), message,
                    QMessageBox::Ok, this);
    box.exec();
}