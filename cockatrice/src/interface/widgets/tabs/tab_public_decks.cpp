#include "tab_public_decks.h"

#include "../../../client/settings/cache_settings.h"
#include "../../deck_loader/deck_loader.h"
#include "../general/layout_containers/flow_widget.h"
#include "../visual_deck_storage/deck_preview/deck_preview_color_identity_filter_widget.h"
#include "../visual_deck_storage/deck_preview/public_deck_preview_widget.h"
#include "../visual_deck_storage/remote_public_decks_model.h"
#include "../visual_deck_storage/visual_deck_storage_search_widget.h"
#include "../visual_deck_storage/visual_deck_storage_tag_filter_widget.h"
#include "public_decks_quick_settings_widget.h"
#include "tab_supervisor.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_download_public.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <optional>

TabPublicDecks::TabPublicDecks(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &_userName)
    : Tab(_tabSupervisor), client(_client), userName(_userName)
{
    model = new RemotePublicDecksModel(client, this);
    cardSize = SettingsCache::instance().cardsDisplay().getVisualDeckStorageCardSize();

    titleLabel = new QLabel(tr("Public decks of %1").arg(userName), this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);

    // Filter/toolbar row, matching the Visual Deck Storage: color identity filter
    // first, the search bar stretching in the middle, and the quick settings
    // cogwheel at the end. The card size slider lives inside the cogwheel popup.
    emptyLabel = new QLabel(tr("This user has not published any decks."), this);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setVisible(false);

    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setVisible(false);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    flowWidget->setSpacing(8, 8);

    colorIdentityFilter = new DeckPreviewColorIdentityFilterWidget(this);
    searchWidget = new VisualDeckStorageSearchWidget(this);
    searchWidget->setPlaceholderText(tr("Search by deck name"));
    refreshButton = new QToolButton(this);
    refreshButton->setIcon(QPixmap("theme:icons/reload"));
    refreshButton->setFixedSize(32, 32);
    quickSettingsWidget = new PublicDecksQuickSettingsWidget(this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(colorIdentityFilter);
    filterLayout->addWidget(searchWidget, 1);
    filterLayout->addWidget(refreshButton);
    filterLayout->addWidget(quickSettingsWidget);

    tagFilterWidget = new VisualDeckStorageTagFilterWidget(this);
    tagFilterWidget->setAllTagsProvider([this] { return model->allTags(); });
    updateTagsVisibility(quickSettingsWidget->getShowTagFilter());

    auto *layout = new QVBoxLayout;
    layout->addLayout(headerLayout);
    layout->addLayout(filterLayout);
    layout->addWidget(tagFilterWidget);
    layout->addWidget(statusLabel);
    layout->addWidget(emptyLabel);
    layout->addWidget(flowWidget, 1);

    auto *mainWidget = new QWidget(this);
    mainWidget->setLayout(layout);
    setCentralWidget(mainWidget);

    connect(refreshButton, &QToolButton::clicked, this, [this] { model->refresh(userName); });
    connect(model, &QAbstractItemModel::modelReset, this, &TabPublicDecks::rebuildGrid);
    connect(model, &RemotePublicDecksModel::loadingChanged, this, &TabPublicDecks::updateLoadingState);
    connect(model, &RemotePublicDecksModel::loadFailed, this, [this](const QString &message) {
        statusLabel->setText(message);
        statusLabel->setVisible(true);
        flowWidget->setVisible(false);
        emptyLabel->setVisible(false);
    });
    connect(searchWidget, &VisualDeckStorageSearchWidget::searchTextChanged, this,
            [this](const QString &text) { model->setSearchText(text); });
    connect(colorIdentityFilter, &DeckPreviewColorIdentityFilterWidget::activeColorsChanged, this,
            &TabPublicDecks::updateColorFilter);
    connect(colorIdentityFilter, &DeckPreviewColorIdentityFilterWidget::filterModeChanged, this,
            &TabPublicDecks::updateColorFilter);
    connect(tagFilterWidget, &VisualDeckStorageTagFilterWidget::filterChanged, this, &TabPublicDecks::updateTagFilter);
    connect(quickSettingsWidget, &PublicDecksQuickSettingsWidget::cardSizeChanged, this,
            &TabPublicDecks::updateCardSize);
    connect(quickSettingsWidget, &PublicDecksQuickSettingsWidget::showTagFilterChanged, this,
            &TabPublicDecks::updateTagsVisibility);

    model->refresh(userName);
}

QString TabPublicDecks::getTabText() const
{
    return tr("Public decks of %1").arg(userName);
}

void TabPublicDecks::retranslateUi()
{
    titleLabel->setText(tr("Public decks of %1").arg(userName));
    searchWidget->setPlaceholderText(tr("Search by deck name"));
    emptyLabel->setText(tr("This user has not published any decks."));
    refreshButton->setToolTip(tr("Refresh"));
    quickSettingsWidget->setToolTip(tr("Public Decks Settings"));
    emit tabTextChanged(this, getTabText());
}

bool TabPublicDecks::closeRequest()
{
    emit closing(this);
    return Tab::closeRequest();
}

void TabPublicDecks::rebuildGrid()
{
    flowWidget->clearLayout();

    const int count = model->rowCount();
    if (count == 0) {
        emptyLabel->setText(model->totalCount() > 0 ? tr("No decks match your filters.")
                                                    : tr("This user has not published any decks."));
    }
    emptyLabel->setVisible(count == 0);
    for (int i = 0; i < count; ++i) {
        auto *tile = new PublicDeckPreviewWidget(flowWidget, model->entryAt(i));
        tile->setScaleFactor(cardSize);
        connect(tile, &PublicDeckPreviewWidget::openDeckRequested, this, &TabPublicDecks::openDeck);
        flowWidget->addWidget(tile);
    }

    // The deck set changed, so the tag filter chips are re-gathered from it.
    tagFilterWidget->refreshTags();
}

void TabPublicDecks::updateColorFilter()
{
    model->setColorFilter(colorIdentityFilter->getFilterMode(), colorIdentityFilter->getActiveColors());
}

void TabPublicDecks::updateTagFilter()
{
    const QStringList selectedTags = tagFilterWidget->selectedTags();
    const QStringList excludedTags = tagFilterWidget->excludedTags();
    model->setTagFilter(QSet<QString>(selectedTags.cbegin(), selectedTags.cend()),
                        QSet<QString>(excludedTags.cbegin(), excludedTags.cend()));
    tagFilterWidget->refreshTags();
}

void TabPublicDecks::updateTagsVisibility(bool visible)
{
    tagFilterWidget->setVisible(visible);
}

void TabPublicDecks::updateLoadingState(bool loading)
{
    if (loading) {
        statusLabel->setText(tr("Loading public decks…"));
        statusLabel->setVisible(true);
        flowWidget->setVisible(false);
        emptyLabel->setVisible(false);
    } else {
        statusLabel->setVisible(false);
        flowWidget->setVisible(true);
    }
}

void TabPublicDecks::updateCardSize(int scale)
{
    cardSize = scale;
    applyCardSize(scale);
}

void TabPublicDecks::applyCardSize(int scale)
{
    const auto tiles = flowWidget->findChildren<PublicDeckPreviewWidget *>();
    for (PublicDeckPreviewWidget *tile : tiles) {
        tile->setScaleFactor(scale);
    }
    flowWidget->setMinimumSizeToMaxSizeHint();
}

void TabPublicDecks::openDeck(int deckId)
{
    Command_DeckDownloadPublic cmd;
    cmd.set_deck_id(deckId);

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabPublicDecks::openDeckFinished);
    client->sendCommand(pend);
}

void TabPublicDecks::openDeckFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (response.response_code() != Response::RespOk) {
        QMessageBox::warning(this, tr("Open public deck"),
                             tr("Failed to open the public deck (server response code %1).")
                                 .arg(QString::number(static_cast<int>(response.response_code()))));
        return;
    }

    const Response_DeckDownload &resp = response.GetExtension(Response_DeckDownload::ext);
    std::optional<LoadedDeck> deckOpt =
        DeckLoader::loadFromRemote(QString::fromStdString(resp.deck()), LoadedDeck::LoadInfo::NON_REMOTE_ID);
    if (!deckOpt) {
        QMessageBox::warning(this, tr("Open public deck"), tr("The public deck could not be parsed."));
        return;
    }

    tabSupervisor->openDeckInNewTab(deckOpt.value());
}
