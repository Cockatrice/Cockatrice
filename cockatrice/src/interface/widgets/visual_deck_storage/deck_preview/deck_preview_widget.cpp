#include "deck_preview_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "deck_preview_deck_tags_display_widget.h"

#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSet>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>

DeckPreviewWidget::DeckPreviewWidget(QWidget *_parent,
                                     VisualDeckStorageWidget *_visualDeckStorageWidget,
                                     const QString &_filePath)
    : QWidget(_parent), visualDeckStorageWidget(_visualDeckStorageWidget), filePath(_filePath),
      colorIdentityWidget(nullptr), deckTagsDisplayWidget(nullptr)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    deckLoader = new DeckLoader(this);
    connect(deckLoader, &DeckLoader::loadFinished, this, &DeckPreviewWidget::initializeUi);
    /* TODO: We shouldn't update the tags on *every* deck load, since it's kinda expensive. We should instead count how
     many deck loads have finished already and if we've loaded all decks and THEN load all the tags at once. */
    connect(deckLoader, &DeckLoader::loadFinished, visualDeckStorageWidget->tagFilterWidget,
            &VisualDeckStorageTagFilterWidget::refreshTags);
    deckLoader->loadFromFileAsync(filePath, DeckFileFormat::getFormatFromName(filePath), false);

    bannerCardDisplayWidget =
        new DeckPreviewCardPictureWidget(this, false, visualDeckStorageWidget->deckPreviewSelectionAnimationEnabled);

    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageClicked, this,
            &DeckPreviewWidget::imageClickedEvent);
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &DeckPreviewWidget::imageDoubleClickedEvent);

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowColorIdentityChanged, this,
            &DeckPreviewWidget::updateColorIdentityVisibility);
    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowTagsOnDeckPreviewsChanged, this,
            &DeckPreviewWidget::updateTagsVisibility);
    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowBannerCardComboBoxChanged, this,
            &DeckPreviewWidget::updateBannerCardComboBoxVisibility);
    connect(visualDeckStorageWidget->settings(), &VisualDeckStorageQuickSettingsWidget::deckPreviewTooltipChanged, this,
            &DeckPreviewWidget::refreshBannerCardToolTip);

    layout->addWidget(bannerCardDisplayWidget);
}

void DeckPreviewWidget::retranslateUi()
{
    bannerCardLabel->setText(tr("Banner Card"));
}

void DeckPreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (bannerCardDisplayWidget == nullptr) {
        return;
    }
    QList<QWidget *> widgets = findChildren<QWidget *>();
    for (QWidget *widget : widgets) {
        widget->setMaximumWidth(bannerCardDisplayWidget->width());
    }
}

void DeckPreviewWidget::initializeUi(const bool deckLoadSuccess)
{
    if (!deckLoadSuccess) {
        return;
    }
    auto bannerCard = deckLoader->getDeck().deckList.getBannerCard().name.isEmpty()
                          ? ExactCard()
                          : CardDatabaseManager::query()->getCard(deckLoader->getDeck().deckList.getBannerCard());

    bannerCardDisplayWidget->setCard(bannerCard);
    bannerCardDisplayWidget->setFontSize(24);
    setFilePath(deckLoader->getDeck().lastLoadInfo.fileName);

    colorIdentityWidget = new ColorIdentityWidget(this, getColorIdentity());
    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, deckLoader->getDeck().deckList.getTags());
    connect(deckTagsDisplayWidget, &DeckPreviewDeckTagsDisplayWidget::tagsChanged, this, &DeckPreviewWidget::setTags);

    bannerCardLabel = new QLabel(this);
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardComboBox = new QComboBox(this);
    bannerCardComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    bannerCardComboBox->setObjectName("bannerCardComboBox");
    bannerCardComboBox->setCurrentText(deckLoader->getDeck().deckList.getBannerCard().name);
    bannerCardComboBox->installEventFilter(new NoScrollFilter(bannerCardComboBox));
    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeckPreviewWidget::setBannerCard);

    updateColorIdentityVisibility(SettingsCache::instance().getVisualDeckStorageShowColorIdentity());
    updateBannerCardComboBox();
    updateBannerCardComboBoxVisibility(SettingsCache::instance().getVisualDeckStorageShowBannerCardComboBox());
    updateTagsVisibility(SettingsCache::instance().getVisualDeckStorageShowTagsOnDeckPreviews());

    layout->addWidget(colorIdentityWidget);
    layout->addWidget(deckTagsDisplayWidget);
    layout->addWidget(bannerCardLabel);
    layout->addWidget(bannerCardComboBox);

    refreshBannerCardText();

    retranslateUi();
}

void DeckPreviewWidget::updateVisibility()
{
    setHidden(!checkVisibility());
}

bool DeckPreviewWidget::checkVisibility() const
{
    if (filteredBySearch || filteredByColor || filteredByTags) {
        return false;
    }
    return true;
}

void DeckPreviewWidget::updateColorIdentityVisibility(bool visible)
{
    if (colorIdentityWidget == nullptr) {
        return;
    }

    colorIdentityWidget->setVisible(visible);
}

void DeckPreviewWidget::updateBannerCardComboBoxVisibility(bool visible)
{
    if (bannerCardComboBox == nullptr) {
        return;
    }

    if (visible) {
        bannerCardComboBox->setVisible(true);
        bannerCardLabel->setVisible(true);
    } else {
        bannerCardComboBox->setHidden(true);
        bannerCardLabel->setHidden(true);
    }
}

void DeckPreviewWidget::updateTagsVisibility(bool visible)
{
    if (deckTagsDisplayWidget == nullptr) {
        return;
    }

    if (visible) {
        deckTagsDisplayWidget->setVisible(true);
    } else {
        deckTagsDisplayWidget->setHidden(true);
    }
}

QString DeckPreviewWidget::getColorIdentity()
{
    QStringList cardList = deckLoader->getDeck().deckList.getCardList({DECK_ZONE_MAIN, DECK_ZONE_SIDE});
    if (cardList.isEmpty()) {
        return {};
    }

    QSet<QChar> colorSet; // A set to collect unique color symbols (e.g., W, U, B, R, G)

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = CardDatabaseManager::query()->getCardInfo(cardName);
        if (currentCard) {
            QString colors = currentCard->getColors(); // Assuming this returns something like "WUB"
            for (const QChar &color : colors) {
                colorSet.insert(color);
            }
        }
    }

    // Ensure the color identity is in WUBRG order
    QString colorIdentity;
    const QString wubrgOrder = "WUBRG";
    for (const QChar &color : wubrgOrder) {
        if (colorSet.contains(color)) {
            colorIdentity.append(color);
        }
    }

    return colorIdentity;
}

/**
 * The display name is given by the deck name, or the filename if the deck name is not set.
 */
QString DeckPreviewWidget::getDisplayName() const
{
    QString deckName = deckLoader->getDeck().deckList.getName();
    return !deckName.isEmpty() ? deckName : QFileInfo(deckLoader->getDeck().lastLoadInfo.fileName).fileName();
}

void DeckPreviewWidget::setFilePath(const QString &_filePath)
{
    filePath = _filePath;
}

/**
 * Refreshes the banner card text.
 * This also calls `refreshBannerCardToolTip`, since those two often need to be updated together.
 */
void DeckPreviewWidget::refreshBannerCardText()
{
    bannerCardDisplayWidget->setOverlayText(getDisplayName());

    refreshBannerCardToolTip();
}

void DeckPreviewWidget::refreshBannerCardToolTip()
{
    auto type = visualDeckStorageWidget->settings()->getDeckPreviewTooltip();
    switch (type) {
        case VisualDeckStorageQuickSettingsWidget::TooltipType::None:
            bannerCardDisplayWidget->setToolTip("");
            break;
        case VisualDeckStorageQuickSettingsWidget::TooltipType::Filepath:
            bannerCardDisplayWidget->setToolTip(filePath);
            break;
    }
}

void DeckPreviewWidget::updateBannerCardComboBox()
{
    // Store the current text of the combo box
    QString currentText = bannerCardComboBox->currentText();

    // Block signals temporarily
    bool wasBlocked = bannerCardComboBox->blockSignals(true);
    bannerCardComboBox->setUpdatesEnabled(false);

    // Clear the existing items in the combo box
    bannerCardComboBox->clear();

    // Prepare the new items with deduplication
    QSet<QPair<QString, QString>> bannerCardSet;

    QList<const DecklistCardNode *> cardsInDeck = deckLoader->getDeck().deckList.getCardNodes();

    for (auto currentCard : cardsInDeck) {
        for (int k = 0; k < currentCard->getNumber(); ++k) {
            bannerCardSet.insert(QPair<QString, QString>(currentCard->getName(), currentCard->getCardProviderId()));
        }
    }

    QList<QPair<QString, QString>> pairList = bannerCardSet.values();

    // Sort QList by the first() element of the QPair
    std::sort(pairList.begin(), pairList.end(), [](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
        return a.first.toLower() < b.first.toLower();
    });

    // This is *slightly* more performant than using addItem in a loop.

    QStandardItemModel *model = new QStandardItemModel(pairList.size(), 1, bannerCardComboBox);

    int row = 0;
    for (const auto &pair : pairList) {
        QStandardItem *item = new QStandardItem(pair.first);
        item->setData(QVariant::fromValue(pair), Qt::UserRole);
        model->setItem(row++, 0, item);
    }

    bannerCardComboBox->setModel(model);

    // Try to restore the previous selection by finding the currentText
    int restoredIndex = bannerCardComboBox->findText(currentText);
    if (restoredIndex != -1) {
        bannerCardComboBox->setCurrentIndex(restoredIndex);
    } else {
        // Add a placeholder "-" and set it as the current selection
        int bannerIndex = bannerCardComboBox->findText(deckLoader->getDeck().deckList.getBannerCard().name);
        if (bannerIndex != -1) {
            bannerCardComboBox->setCurrentIndex(bannerIndex);
        } else {
            bannerCardComboBox->insertItem(0, "-");
            bannerCardComboBox->setCurrentIndex(0);
        }
    }

    // Restore the previous signal blocking state
    bannerCardComboBox->blockSignals(wasBlocked);
    bannerCardComboBox->setUpdatesEnabled(true);
}

void DeckPreviewWidget::setBannerCard(int /* changedIndex */)
{
    auto [name, id] = bannerCardComboBox->currentData().value<QPair<QString, QString>>();
    CardRef cardRef = {name, id};
    deckLoader->getDeck().deckList.setBannerCard(cardRef);
    DeckLoader::saveToFile(deckLoader->getDeck());
    bannerCardDisplayWidget->setCard(CardDatabaseManager::query()->getCard(cardRef));
}

void DeckPreviewWidget::imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    Q_UNUSED(instance);

    if (event && event->button() == Qt::RightButton) {
        createRightClickMenu()->popup(QCursor::pos());
    }
}

void DeckPreviewWidget::imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    Q_UNUSED(event);
    Q_UNUSED(instance);
    emit deckLoadRequested(filePath);
}

void DeckPreviewWidget::setTags(const QStringList &tags)
{
    deckLoader->getDeck().deckList.setTags(tags);
    DeckLoader::saveToFile(deckLoader->getDeck());
}

QMenu *DeckPreviewWidget::createRightClickMenu()
{
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    connect(menu->addAction(tr("Open in deck editor")), &QAction::triggered, this,
            [this] { emit openDeckEditor(deckLoader->getDeck()); });

    connect(menu->addAction(tr("Edit Tags")), &QAction::triggered, deckTagsDisplayWidget,
            &DeckPreviewDeckTagsDisplayWidget::openTagEditDlg);

    addSetBannerCardMenu(menu);

    menu->addSeparator();

    connect(menu->addAction(tr("Rename Deck")), &QAction::triggered, this, &DeckPreviewWidget::actRenameDeck);

    auto saveToClipboardMenu = menu->addMenu(tr("Save Deck to Clipboard"));

    connect(saveToClipboardMenu->addAction(tr("Annotated")), &QAction::triggered, this,
            [this] { DeckLoader::saveToClipboard(deckLoader->getDeck().deckList, true, true); });
    connect(saveToClipboardMenu->addAction(tr("Annotated (No set info)")), &QAction::triggered, this,
            [this] { DeckLoader::saveToClipboard(deckLoader->getDeck().deckList, true, false); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated")), &QAction::triggered, this,
            [this] { DeckLoader::saveToClipboard(deckLoader->getDeck().deckList, false, true); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated (No set info)")), &QAction::triggered, this,
            [this] { DeckLoader::saveToClipboard(deckLoader->getDeck().deckList, false, false); });

    menu->addSeparator();

    connect(menu->addAction(tr("Rename File")), &QAction::triggered, this, &DeckPreviewWidget::actRenameFile);

    connect(menu->addAction(tr("Delete File")), &QAction::triggered, this, &DeckPreviewWidget::actDeleteFile);

    return menu;
}

/**
 * Adds the "Set Banner Card" submenu to the given menu. Does nothing if bannerCardComboBox is null.
 * @param menu The menu to add the submenu to
 */
void DeckPreviewWidget::addSetBannerCardMenu(QMenu *menu)
{
    if (!bannerCardComboBox) {
        return;
    }

    auto bannerCardMenu = menu->addMenu(tr("Set Banner Card"));

    for (int i = 0; i < bannerCardComboBox->count(); ++i) {
        auto action = bannerCardMenu->addAction(bannerCardComboBox->itemText(i));
        connect(action, &QAction::triggered, this, [this, i] { bannerCardComboBox->setCurrentIndex(i); });

        // the checkability is purely for visuals
        action->setCheckable(true);
        action->setChecked(bannerCardComboBox->currentIndex() == i);
    }
}

void DeckPreviewWidget::actRenameDeck()
{
    // read input
    const QString oldName = deckLoader->getDeck().deckList.getName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename deck", tr("New name:"), QLineEdit::Normal, oldName, &ok);
    if (!ok || oldName == newName) {
        return;
    }

    // write change
    deckLoader->getDeck().deckList.setName(newName);
    DeckLoader::saveToFile(deckLoader->getDeck());

    // update VDS
    refreshBannerCardText();
}

void DeckPreviewWidget::actRenameFile()
{
    // read input
    const auto info = QFileInfo(filePath);
    const QString oldName = info.baseName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename file", tr("New name:"), QLineEdit::Normal, oldName, &ok);
    if (!ok || newName.isEmpty() || oldName == newName) {
        return;
    }

    QString newFileName = newName;
    if (!info.suffix().isEmpty()) {
        newFileName += "." + info.suffix();
    }

    // write change
    const QString newFilePath = QFileInfo(info.dir(), newFileName).filePath();
    if (!QFile::rename(info.filePath(), newFilePath)) {
        QMessageBox::critical(this, tr("Error"), tr("Rename failed"));
        return;
    }

    deckLoader->getDeck().lastLoadInfo.fileName = newFilePath;

    // update VDS
    setFilePath(newFilePath);
    refreshBannerCardText();
}

void DeckPreviewWidget::actDeleteFile()
{
    // read input
    auto res = QMessageBox::warning(this, tr("Delete file"), tr("Are you sure you want to delete the selected file?"),
                                    QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) {
        return;
    }

    // write change
    if (!QFile::remove(QFileInfo(filePath).filePath())) {
        QMessageBox::critical(this, tr("Error"), tr("Delete failed"));
        return;
    }

    // update VDS
    this->deleteLater();
}
