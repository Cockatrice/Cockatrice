#include "deck_preview_widget.h"

#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../settings/cache_settings.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "deck_preview_deck_tags_display_widget.h"

#include <QClipboard>
#include <QFileInfo>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSet>
#include <QStandardItemModel>
#include <QVBoxLayout>

DeckPreviewWidget::DeckPreviewWidget(QWidget *_parent,
                                     VisualDeckStorageWidget *_visualDeckStorageWidget,
                                     const QString &_filePath)
    : QWidget(_parent), visualDeckStorageWidget(_visualDeckStorageWidget), filePath(_filePath),
      checkInitializeUiTimer(QTimer(this))
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    deckLoader = new DeckLoader();
    deckLoader->setParent(this);
    connect(deckLoader, &DeckLoader::loadFinished, this, &DeckPreviewWidget::onDeckLoadFinished);
    /* TODO: We shouldn't update the tags on *every* deck load, since it's kinda expensive. We should instead count how
     many deck loads have finished already and if we've loaded all decks and THEN load all the tags at once. */
    connect(deckLoader, &DeckLoader::loadFinished, visualDeckStorageWidget->tagFilterWidget,
            &VisualDeckStorageTagFilterWidget::refreshTags);
    deckLoader->loadFromFileAsync(filePath, DeckLoader::getFormatFromName(filePath), false);

    bannerCardDisplayWidget =
        new DeckPreviewCardPictureWidget(this, false, visualDeckStorageWidget->deckPreviewSelectionAnimationEnabled);

    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageClicked, this,
            &DeckPreviewWidget::imageClickedEvent);
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &DeckPreviewWidget::imageDoubleClickedEvent);

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowTagsOnDeckPreviewsChanged, this,
            &DeckPreviewWidget::updateTagsVisibility);
    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowBannerCardComboBoxChanged, this,
            &DeckPreviewWidget::updateBannerCardComboBoxVisibility);

    layout->addWidget(bannerCardDisplayWidget);

    checkInitializeUiTimer.setInterval(10);
    connect(&checkInitializeUiTimer, &QTimer::timeout, this, &DeckPreviewWidget::tryInitializeUi);
    checkInitializeUiTimer.start();
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

void DeckPreviewWidget::onDeckLoadFinished()
{
    deckLoaded = true;
    tryInitializeUi();
}

/**
 * Attempts to initialize the child widgets.
 * No-ops if the child widgets aren't ready to be created yet or have already been created.
 *
 * DeckPreviewWidget delays the creation of its child widgets until it is on screen.
 * This significantly improves the performance of the VDS.
 */
void DeckPreviewWidget::tryInitializeUi()
{
    // stop if child widgets are already created from initializeUi
    if (colorIdentityWidget) {
        return;
    }

    // stop if deck isn't loaded yet
    if (!deckLoaded) {
        return;
    }

    // stop if this widget is offscreen
    if (this->visibleRegion().isNull()) {
        return;
    }

    initializeUi();
}

/**
 * Initializes the child widgets of the DeckPreviewWidget.
 */
void DeckPreviewWidget::initializeUi()
{
    checkInitializeUiTimer.stop();

    auto bannerCard = deckLoader->getBannerCard().first.isEmpty()
                          ? CardInfoPtr()
                          : CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
                                deckLoader->getBannerCard().first, deckLoader->getBannerCard().second);

    bannerCardDisplayWidget->setCard(bannerCard);
    bannerCardDisplayWidget->setFontSize(24);
    refreshBannerCardText();
    setFilePath(deckLoader->getLastFileName());

    colorIdentityWidget = new ColorIdentityWidget(this, getColorIdentity());
    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, deckLoader);

    bannerCardLabel = new QLabel(this);
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardComboBox = new QComboBox(this);
    bannerCardComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    bannerCardComboBox->setObjectName("bannerCardComboBox");
    bannerCardComboBox->setCurrentText(deckLoader->getBannerCard().first);
    bannerCardComboBox->installEventFilter(new NoScrollFilter());
    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeckPreviewWidget::setBannerCard);

    updateBannerCardComboBox();
    updateBannerCardComboBoxVisibility(SettingsCache::instance().getVisualDeckStorageShowBannerCardComboBox());
    updateTagsVisibility(SettingsCache::instance().getVisualDeckStorageShowTagsOnDeckPreviews());

    layout->addWidget(colorIdentityWidget);
    layout->addWidget(deckTagsDisplayWidget);
    layout->addWidget(bannerCardLabel);
    layout->addWidget(bannerCardComboBox);

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
    QStringList cardList = deckLoader->getCardList();
    if (cardList.isEmpty()) {
        return {};
    }

    QSet<QChar> colorSet; // A set to collect unique color symbols (e.g., W, U, B, R, G)

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = CardDatabaseManager::getInstance()->getCard(cardName);
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

void DeckPreviewWidget::setFilePath(const QString &_filePath)
{
    filePath = _filePath;
}

void DeckPreviewWidget::refreshBannerCardText()
{
    bannerCardDisplayWidget->setOverlayText(
        deckLoader->getName().isEmpty() ? QFileInfo(deckLoader->getLastFileName()).fileName() : deckLoader->getName());
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
    InnerDecklistNode *listRoot = deckLoader->getRoot();
    for (auto i : *listRoot) {
        auto *currentZone = dynamic_cast<InnerDecklistNode *>(i);
        for (auto j : *currentZone) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(j);
            if (!currentCard)
                continue;

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                bannerCardSet.insert(QPair<QString, QString>(currentCard->getName(), currentCard->getCardProviderId()));
            }
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
        int bannerIndex = bannerCardComboBox->findText(deckLoader->getBannerCard().first);
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
    QVariant itemData = bannerCardComboBox->itemData(bannerCardComboBox->currentIndex());
    deckLoader->setBannerCard(QPair<QString, QString>(bannerCardComboBox->currentText(), itemData.toString()));
    deckLoader->saveToFile(filePath, DeckLoader::getFormatFromName(filePath));
    bannerCardDisplayWidget->setCard(CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
        bannerCardComboBox->currentText(), itemData.toString()));
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

QMenu *DeckPreviewWidget::createRightClickMenu()
{
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    connect(menu->addAction(tr("Open in deck editor")), &QAction::triggered, this,
            [this] { emit openDeckEditor(deckLoader); });

    connect(menu->addAction(tr("Edit Tags")), &QAction::triggered, deckTagsDisplayWidget,
            &DeckPreviewDeckTagsDisplayWidget::openTagEditDlg);

    addSetBannerCardMenu(menu);

    menu->addSeparator();

    connect(menu->addAction(tr("Rename Deck")), &QAction::triggered, this, &DeckPreviewWidget::actRenameDeck);

    auto saveToClipboardMenu = menu->addMenu(tr("Save Deck to Clipboard"));

    connect(saveToClipboardMenu->addAction(tr("Annotated")), &QAction::triggered, this,
            [this] { deckLoader->saveToClipboard(true, true); });
    connect(saveToClipboardMenu->addAction(tr("Annotated (No set info)")), &QAction::triggered, this,
            [this] { deckLoader->saveToClipboard(true, false); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated")), &QAction::triggered, this,
            [this] { deckLoader->saveToClipboard(false, true); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated (No set info)")), &QAction::triggered, this,
            [this] { deckLoader->saveToClipboard(false, false); });

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
    const QString oldName = deckLoader->getName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename deck", tr("New name:"), QLineEdit::Normal, oldName, &ok);
    if (!ok || oldName == newName) {
        return;
    }

    // write change
    deckLoader->setName(newName);
    deckLoader->saveToFile(filePath, DeckLoader::getFormatFromName(filePath));

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

    deckLoader->setLastFileName(newFilePath);

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
