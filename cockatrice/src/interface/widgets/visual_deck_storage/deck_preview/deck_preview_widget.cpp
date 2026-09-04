#include "deck_preview_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../../interface/widgets/dialogs/dlg_convert_deck_to_cod_format.h"
#include "../../../deck_loader/deck_loader.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/additional_info/deck_color_identity.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "../visual_deck_storage_quick_settings_widget.h"
#include "../visual_deck_storage_tag_filter_widget.h"
#include "../visual_deck_storage_widget.h"
#include "deck_preview_deck_tags_display_widget.h"

#include <QFileInfo>
#include <QFrame>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSet>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <algorithm>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

DeckPreviewWidget::DeckPreviewWidget(QWidget *_parent,
                                     VisualDeckStorageWidget *_visualDeckStorageWidget,
                                     VisualDeckStorageModel *_model,
                                     const QString &_filePath)
    : QWidget(_parent), filePath(_filePath), visualDeckStorageWidget(_visualDeckStorageWidget), model(_model)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    auto *pictureWidget =
        new DeckPreviewCardPictureWidget(this, false, visualDeckStorageWidget->deckPreviewSelectionAnimationEnabled);
    pictureWidget->setFontSize(24);
    connect(pictureWidget, &DeckPreviewCardPictureWidget::imageClicked, this, &DeckPreviewWidget::imageClickedEvent);
    connect(pictureWidget, &DeckPreviewCardPictureWidget::imageSingleClicked, this,
            &DeckPreviewWidget::imageSingleClicked);
    connect(pictureWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &DeckPreviewWidget::imageDoubleClickedEvent);
    bannerCardDisplayWidget = pictureWidget;

    colorIdentityWidget = new ColorIdentityWidget(this);
    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this);
    connect(deckTagsDisplayWidget, &DeckPreviewDeckTagsDisplayWidget::tagsChanged, this, &DeckPreviewWidget::setTags);
    deckTagsDisplayWidget->setKnownTagsProvider(
        [this] { return visualDeckStorageWidget->tagFilterWidget->getAllKnownTags(); });
    deckTagsDisplayWidget->setConversionPromptHandler([this] { return promptFileConversionIfRequired(); });

    bannerCardLabel = new QLabel(this);
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardComboBox = new QComboBox(this);
    bannerCardComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    bannerCardComboBox->setObjectName("bannerCardComboBox");
    bannerCardComboBox->installEventFilter(new NoScrollFilter(bannerCardComboBox));
    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeckPreviewWidget::setBannerCard);

    // Apply the initial visibility settings and keep them in sync while they change.
    updateColorIdentityVisibility(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowColorIdentity());
    updateBannerCardComboBoxVisibility(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowBannerCardComboBox());
    updateTagsVisibility(SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagsOnDeckPreviews());

    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageShowColorIdentityChanged, this,
            &DeckPreviewWidget::updateColorIdentityVisibility);
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageShowTagsOnDeckPreviewsChanged, this,
            &DeckPreviewWidget::updateTagsVisibility);
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageShowBannerCardComboBoxChanged, this,
            &DeckPreviewWidget::updateBannerCardComboBoxVisibility);
    connect(visualDeckStorageWidget->settings(), &VisualDeckStorageQuickSettingsWidget::deckPreviewTooltipChanged, this,
            &DeckPreviewWidget::refreshBannerCardToolTip);

    layout->addWidget(bannerCardDisplayWidget);
    layout->addWidget(colorIdentityWidget);
    layout->addWidget(deckTagsDisplayWidget);
    layout->addWidget(bannerCardLabel);
    layout->addWidget(bannerCardComboBox);

    // Only re-sync when this widget's own row changed. Without the row check, every
    // finished deck load would trigger a full resync (card db lookup + combo rebuild)
    // in every preview widget.
    connect(model, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                const int r = row();
                if (r >= topLeft.row() && r <= bottomRight.row()) {
                    syncFromModel();
                }
            });

    retranslateUi();
    syncFromModel();

    // resizeEvent clamps every child to the picture's width, so collect them once here
    // to keep the resize handler from searching the widget tree on every layout pass.
    fixedWidthChildren = {bannerCardDisplayWidget, colorIdentityWidget, deckTagsDisplayWidget, bannerCardLabel,
                          bannerCardComboBox};

    // Child of the banner widget so the frame tracks the banner's selection animation
    // (which animates the banner's position) instead of staying at a stale static offset.
    selectionFrame = new QFrame(bannerCardDisplayWidget);
    selectionFrame->setAttribute(Qt::WA_TransparentForMouseEvents);
    selectionFrame->setStyleSheet(QStringLiteral(
        "QFrame { border: 2px solid palette(highlight); border-radius: 4px; background: transparent; }"));
    selectionFrame->setVisible(false);
    bannerCardDisplayWidget->installEventFilter(this);
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

    const int width = bannerCardDisplayWidget->width();
    if (width == lastKnownBannerWidth) {
        return;
    }
    lastKnownBannerWidth = width;

    for (QWidget *widget : fixedWidthChildren) {
        widget->setMaximumWidth(width);
    }
    updateSelectionFrameGeometry();
}

bool DeckPreviewWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == bannerCardDisplayWidget && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        updateSelectionFrameGeometry();
    }
    return QWidget::eventFilter(watched, event);
}

void DeckPreviewWidget::setShareSelectable(bool selectable)
{
    shareSelectable = selectable;
    if (!selectable) {
        setShareSelected(false);
    }
    updateSelectionStyle();
}

void DeckPreviewWidget::setShareSelected(bool selected)
{
    if (shareSelected == selected) {
        return;
    }
    shareSelected = selected;
    updateSelectionStyle();
    emit shareSelectionToggled(selected);
}

bool DeckPreviewWidget::isShareSelected() const
{
    return shareSelected;
}

bool DeckPreviewWidget::isShareSelectable() const
{
    return shareSelectable;
}

void DeckPreviewWidget::updateSelectionFrameGeometry()
{
    if (selectionFrame == nullptr || bannerCardDisplayWidget == nullptr) {
        return;
    }
    // Frame is a child of the banner, so it is positioned in banner coordinates and
    // tracks the banner's selection animation automatically. A small inset keeps the
    // highlight visible around the card art without occluding it.
    selectionFrame->setGeometry(bannerCardDisplayWidget->rect().adjusted(1, 1, -1, -1));
    selectionFrame->raise();
}

void DeckPreviewWidget::updateSelectionStyle()
{
    if (selectionFrame != nullptr) {
        selectionFrame->setVisible(shareSelectable && isShareSelected());
    }
}

void DeckPreviewWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);

    // Don't do reloads until the deck has actually been loaded once.
    reloadIfModified();
}

/**
 * @brief The row of this deck in the source model, or -1 if it no longer exists.
 */
int DeckPreviewWidget::row() const
{
    return model->rowForFilePath(filePath);
}

/**
 * @brief The display name is given by the deck name, or the filename if the deck name is not set.
 */
QString DeckPreviewWidget::getDisplayName() const
{
    const int r = row();
    if (r == -1) {
        return {};
    }
    return model->dataForRow(r).displayName;
}

/**
 * @brief Reloads the deck if the file's last modified time has increased since we last checked.
 */
void DeckPreviewWidget::reloadIfModified()
{
    const int r = row();
    if (r == -1 || !model->dataForRow(r).loadSucceeded) {
        return;
    }

    model->reloadIfModified(r);
}

/**
 * @brief Syncs the contents of the child widgets with the current row's data.
 */
void DeckPreviewWidget::syncFromModel()
{
    const int r = row();
    if (r == -1) {
        return;
    }

    const DeckPreviewData &data = model->dataForRow(r);
    filePath = data.filePath;

    const CardRef bannerCardRef = data.deck.deckList.getBannerCard();
    const ExactCard bannerCard =
        bannerCardRef.name.isEmpty() ? ExactCard() : CardDatabaseManager::query()->getCard(bannerCardRef);

    bannerCardDisplayWidget->setCard(bannerCard);
    refreshBannerCardText();
    updateBannerCardComboBox(bannerCardRef.name);
    colorIdentityWidget->setColorIdentity(data.colorIdentity);
    deckTagsDisplayWidget->setTags(data.tags);
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

void DeckPreviewWidget::updateBannerCardComboBox(const QString &currentText)
{
    // Block signals temporarily
    bool wasBlocked = bannerCardComboBox->blockSignals(true);
    bannerCardComboBox->setUpdatesEnabled(false);

    // Clear the existing items in the combo box
    bannerCardComboBox->clear();

    // Prepare the new items with deduplication
    QSet<QPair<QString, QString>> bannerCardSet;

    const int r = row();
    if (r != -1) {
        const DeckList &deckList = model->dataForRow(r).deck.deckList;
        const QList<const DecklistCardNode *> cardsInDeck = deckList.getCardNodes();

        for (auto currentCard : cardsInDeck) {
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

    QStandardItemModel *comboModel = new QStandardItemModel(pairList.size(), 1, bannerCardComboBox);

    int row = 0;
    for (const auto &pair : pairList) {
        QStandardItem *item = new QStandardItem(pair.first);
        item->setData(QVariant::fromValue(pair), Qt::UserRole);
        comboModel->setItem(row++, 0, item);
    }

    bannerCardComboBox->setModel(comboModel);

    // Try to restore the previous selection by finding the currentText
    int restoredIndex = bannerCardComboBox->findText(currentText);
    if (restoredIndex != -1) {
        bannerCardComboBox->setCurrentIndex(restoredIndex);
    } else {
        // Add a placeholder "-" and set it as the current selection
        const QString currentBannerCardName =
            r == -1 ? QString() : model->dataForRow(r).deck.deckList.getBannerCard().name;
        int bannerIndex = bannerCardComboBox->findText(currentBannerCardName);
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
    const int r = row();
    if (r == -1) {
        return;
    }
    model->setBannerCard(r, cardRef);
    bannerCardDisplayWidget->setCard(CardDatabaseManager::query()->getCard(cardRef));
}

void DeckPreviewWidget::imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    Q_UNUSED(instance);

    if (event && event->button() == Qt::RightButton) {
        createRightClickMenu()->popup(QCursor::pos());
    }
}

void DeckPreviewWidget::imageSingleClicked()
{
    if (isShareSelectable()) {
        setShareSelected(!isShareSelected());
    }
}

void DeckPreviewWidget::imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    Q_UNUSED(event);
    Q_UNUSED(instance);
    if (isShareSelectable()) {
        return; // in share mode a double click would just toggle a single selection
    }
    emit deckLoadRequested(filePath);
}

void DeckPreviewWidget::setTags(const QStringList &tags)
{
    const int r = row();
    if (r != -1) {
        model->setTags(r, tags);
    }
}

QMenu *DeckPreviewWidget::createRightClickMenu()
{
    const int r = row();

    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    connect(menu->addAction(tr("Open in deck editor")), &QAction::triggered, this, [this, r] {
        if (r != -1) {
            emit openDeckEditor(model->deckForRow(r));
        }
    });

    connect(menu->addAction(tr("Share deck...")), &QAction::triggered, this,
            [this] { emit shareDeckRequested(filePath); });

    connect(menu->addAction(tr("Edit Tags")), &QAction::triggered, deckTagsDisplayWidget,
            &DeckPreviewDeckTagsDisplayWidget::openTagEditDlg);

    addSetBannerCardMenu(menu);

    menu->addSeparator();

    connect(menu->addAction(tr("Rename Deck")), &QAction::triggered, this, &DeckPreviewWidget::actRenameDeck);

    auto saveToClipboardMenu = menu->addMenu(tr("Save Deck to Clipboard"));

    connect(saveToClipboardMenu->addAction(tr("Annotated")), &QAction::triggered, this, [this, r] {
        if (r != -1) {
            DeckLoader::saveToClipboard(model->dataForRow(r).deck.deckList, true, true);
        }
    });
    connect(saveToClipboardMenu->addAction(tr("Annotated (No set info)")), &QAction::triggered, this, [this, r] {
        if (r != -1) {
            DeckLoader::saveToClipboard(model->dataForRow(r).deck.deckList, true, false);
        }
    });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated")), &QAction::triggered, this, [this, r] {
        if (r != -1) {
            DeckLoader::saveToClipboard(model->dataForRow(r).deck.deckList, false, true);
        }
    });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated (No set info)")), &QAction::triggered, this, [this, r] {
        if (r != -1) {
            DeckLoader::saveToClipboard(model->dataForRow(r).deck.deckList, false, false);
        }
    });

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
    const int r = row();
    if (r == -1) {
        return;
    }

    // read input
    const QString oldName = model->dataForRow(r).deckName;

    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename deck"), tr("New name:"), QLineEdit::Normal, oldName, &ok);
    if (!ok || oldName == newName) {
        return;
    }

    // write change
    model->renameDeck(r, newName);

    // The banner card text updates via the model's dataChanged signal.
}

void DeckPreviewWidget::actRenameFile()
{
    const int r = row();
    if (r == -1) {
        return;
    }

    // read input
    const auto info = QFileInfo(filePath);
    const QString oldName = info.baseName();

    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename file"), tr("New name:"), QLineEdit::Normal, oldName, &ok);
    if (!ok || newName.isEmpty() || oldName == newName) {
        return;
    }

    // write change
    if (!model->renameFile(r, newName)) {
        QMessageBox::critical(this, tr("Error"), tr("Rename failed"));
    }

    // The file path and banner card text update via the model's signals.
}

void DeckPreviewWidget::actDeleteFile()
{
    const int r = row();
    if (r == -1) {
        return;
    }

    // read input
    auto res = QMessageBox::warning(this, tr("Delete file"), tr("Are you sure you want to delete the selected file?"),
                                    QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) {
        return;
    }

    // write change
    if (!model->deleteFile(r)) {
        QMessageBox::critical(this, tr("Error"), tr("Delete failed"));
    }

    // The folder widget removes this preview once the row is gone.
}

/**
 * Checks if the deck's file format supports tags.
 * If not, then prompt the user for file conversion.
 * @return whether the resulting file can support adding tags
 */
bool DeckPreviewWidget::promptFileConversionIfRequired()
{
    return DialogConvertDeckToCodFormat::promptIfRequired(this, filePath, [this] {
        model->convertToCockatriceFormat(row());
        return true;
    });
}
