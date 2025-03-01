#include "deck_preview_widget.h"

#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../settings/cache_settings.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "deck_preview_deck_tags_display_widget.h"

#include <QClipboard>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QSet>
#include <QVBoxLayout>

DeckPreviewWidget::DeckPreviewWidget(QWidget *_parent,
                                     VisualDeckStorageWidget *_visualDeckStorageWidget,
                                     const QString &_filePath)
    : QWidget(_parent), visualDeckStorageWidget(_visualDeckStorageWidget), filePath(_filePath),
      colorIdentityWidget(nullptr), deckTagsDisplayWidget(nullptr)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    deckLoader = new DeckLoader();
    deckLoader->setParent(this);
    connect(deckLoader, &DeckLoader::loadFinished, this, &DeckPreviewWidget::initializeUi);
    connect(deckLoader, &DeckLoader::loadFinished, visualDeckStorageWidget->tagFilterWidget,
            &VisualDeckStorageTagFilterWidget::refreshTags);
    deckLoader->loadFromFileAsync(filePath, DeckLoader::getFormatFromName(filePath), false);

    bannerCardDisplayWidget = new DeckPreviewCardPictureWidget(this);

    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageClicked, this,
            &DeckPreviewWidget::imageClickedEvent);
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &DeckPreviewWidget::imageDoubleClickedEvent);

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowTagsOnDeckPreviewsChanged, this,
            &DeckPreviewWidget::updateTagsVisibility);
    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageShowBannerCardComboBoxChanged, this,
            &DeckPreviewWidget::updateBannerCardComboBoxVisibility);

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
    auto bannerCard = deckLoader->getBannerCard().first.isEmpty()
                          ? CardInfoPtr()
                          : CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
                                deckLoader->getBannerCard().first, deckLoader->getBannerCard().second);

    bannerCardDisplayWidget->setCard(bannerCard);
    bannerCardDisplayWidget->setOverlayText(
        deckLoader->getName().isEmpty() ? QFileInfo(deckLoader->getLastFileName()).fileName() : deckLoader->getName());
    bannerCardDisplayWidget->setFontSize(24);
    setFilePath(deckLoader->getLastFileName());

    colorIdentityWidget = new DeckPreviewColorIdentityWidget(this, getColorIdentity());
    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, deckLoader);

    bannerCardLabel = new QLabel();
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
    if (isVisible() != checkVisibility()) {
        setHidden(!checkVisibility());
        emit visibilityUpdated();
    }
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
                CardInfoPtr info = CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
                    currentCard->getName(), currentCard->getCardProviderId());
                if (info) {
                    bannerCardSet.insert(
                        QPair<QString, QString>(currentCard->getName(), currentCard->getCardProviderId()));
                }
            }
        }
    }

    QList<QPair<QString, QString>> pairList = bannerCardSet.values();

    // Sort QList by the first() element of the QPair
    std::sort(pairList.begin(), pairList.end(), [](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
        return a.first.toLower() < b.first.toLower();
    });

    for (const auto &pair : pairList) {
        QVariantMap dataMap;
        dataMap["name"] = pair.first;
        dataMap["uuid"] = pair.second;

        bannerCardComboBox->addItem(pair.first, dataMap);
    }

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
}

void DeckPreviewWidget::setBannerCard(int /* changedIndex */)
{
    QVariantMap itemData = bannerCardComboBox->itemData(bannerCardComboBox->currentIndex()).toMap();
    deckLoader->setBannerCard(QPair<QString, QString>(itemData["name"].toString(), itemData["uuid"].toString()));
    deckLoader->saveToFile(filePath, DeckLoader::getFormatFromName(filePath));
    bannerCardDisplayWidget->setCard(CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
        itemData["name"].toString(), itemData["uuid"].toString()));
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

static void saveDeckToClipboard(const DeckLoader *deckLoader, bool addComments, bool addSetNameAndNumber)
{
    QString buffer;
    QTextStream stream(&buffer);
    deckLoader->saveToStream_Plain(stream, addComments, addSetNameAndNumber);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

QMenu *DeckPreviewWidget::createRightClickMenu()
{
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    connect(menu->addAction(tr("Open in deck editor")), &QAction::triggered, this,
            [this] { emit openDeckEditor(deckLoader); });

    connect(menu->addAction(tr("Edit Tags")), &QAction::triggered, deckTagsDisplayWidget,
            &DeckPreviewDeckTagsDisplayWidget::openTagEditDlg);

    menu->addSeparator();

    auto saveToClipboardMenu = menu->addMenu(tr("Save Deck to Clipboard"));

    connect(saveToClipboardMenu->addAction(tr("Annotated")), &QAction::triggered, this,
            [this] { saveDeckToClipboard(deckLoader, true, true); });
    connect(saveToClipboardMenu->addAction(tr("Annotated (No set name or number)")), &QAction::triggered, this,
            [this] { saveDeckToClipboard(deckLoader, true, false); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated")), &QAction::triggered, this,
            [this] { saveDeckToClipboard(deckLoader, false, true); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated (No set name or number)")), &QAction::triggered, this,
            [this] { saveDeckToClipboard(deckLoader, false, false); });

    return menu;
}
