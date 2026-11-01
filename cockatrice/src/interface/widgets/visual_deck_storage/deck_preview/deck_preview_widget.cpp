#include "deck_preview_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../../interface/widgets/dialogs/dlg_convert_deck_to_cod_format.h"
#include "../../../deck_loader/deck_loader.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "../visual_deck_storage_model.h"
#include "../visual_deck_storage_quick_settings_widget.h"
#include "../visual_deck_storage_tag_filter_widget.h"
#include "../visual_deck_storage_widget.h"
#include "deck_preview_deck_tags_display_widget.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/deck_list/deck_list.h"
#include "libcockatrice/deck_list/tree/deck_list_card_node.h"
#include "libcockatrice/utility/card_ref.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPair>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QRect>
#include <QSet>
#include <QSharedPointer>
#include <QSizePolicy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringLiteral>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGlobal>
#include <algorithm>
#include <compare>
#include <functional>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/loaded_deck.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>
#include <utility>

DeckPreviewWidget::DeckPreviewWidget(QWidget *_parent,
                                     VisualDeckStorageWidget *_visualDeckStorageWidget,
                                     VisualDeckStorageModel *_model,
                                     const QString &_filePath)
    : QWidget(_parent), filePath(_filePath), visualDeckStorageWidget(_visualDeckStorageWidget), model(_model),
      previewIndex(_model->index(_model->rowForFilePath(_filePath)))
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

    // Retain row identity across renames and row movement. Looking up the old
    // filePath after a rename cannot find the renamed row in the model's map.
    const QPersistentModelIndex previewRow = previewIndex;
    const auto syncCurrentRow = [this, previewRow] {
        if (previewRow.isValid()) {
            filePath = model->dataForRow(previewRow.row()).filePath;
            syncFromModel();
        }
    };
    connect(model, &QAbstractItemModel::dataChanged, this,
            [this, previewRow](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                if (!previewRow.isValid()) {
                    return;
                }
                const int r = previewRow.row();
                const QString currentFilePath = model->dataForRow(r).filePath;
                // A direct receiver may have renamed/moved this row again during
                // signal delivery. Always adopt its current path, but avoid full
                // resyncs for unrelated finished deck loads.
                if (filePath != currentFilePath || (r >= topLeft.row() && r <= bottomRight.row())) {
                    filePath = currentFilePath;
                    syncFromModel();
                }
            });
    // Structural changes can occur in an earlier direct dataChanged receiver,
    // making its original row range stale before our callback runs.
    connect(model, &QAbstractItemModel::rowsInserted, this, syncCurrentRow);
    connect(model, &QAbstractItemModel::rowsRemoved, this, syncCurrentRow);
    connect(model, &QAbstractItemModel::rowsMoved, this, syncCurrentRow);
    connect(model, &QAbstractItemModel::layoutChanged, this, syncCurrentRow);

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
    updateSelectionStyle();
    if (!selectable) {
        // This emits to external receivers, which may remove this preview.
        setShareSelected(false);
    }
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
    return previewIndex.isValid() ? previewIndex.row() : -1;
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
    const QPointer<DeckPreviewWidget> self(this);
    // Successful saves synchronize through dataChanged. A failed save restores
    // the model's old value, so also restore the already-changed combo selection.
    const bool saved = model->setBannerCard(r, cardRef);
    if (self && !saved) {
        syncFromModel();
    }
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
    if (isShareSelectable() || row() == -1) {
        return; // share mode toggles selection; a removed deck cannot be opened
    }
    const QString requestedFilePath = filePath;
    emit deckLoadRequested(requestedFilePath);
}

void DeckPreviewWidget::setTags(const QStringList &tags)
{
    const int r = row();
    if (r == -1) {
        return;
    }
    const QPointer<DeckPreviewWidget> self(this);
    const bool saved = model->setTags(r, tags);
    if (self && !saved) {
        syncFromModel();
    }
}

QMenu *DeckPreviewWidget::createRightClickMenu()
{
    const QPointer<VisualDeckStorageModel> deckModel(model);
    const QString requestedFilePath = filePath;
    const QPersistentModelIndex requestedRow = previewIndex;
    const auto currentRow = [this, deckModel, requestedFilePath, requestedRow] {
        if (!deckModel || !requestedRow.isValid() || filePath != requestedFilePath) {
            return -1;
        }
        const int r = requestedRow.row();
        return deckModel->dataForRow(r).filePath == requestedFilePath ? r : -1;
    };

    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // A popup survives event-loop turns. Keep the original row identity and path,
    // so even removing and reinserting the same filename cancels its old actions.
    connect(menu->addAction(tr("Open in deck editor")), &QAction::triggered, this,
            [this, deckModel, currentRow] {
        const int r = currentRow();
        if (r != -1) {
            // Signal receivers can reset or destroy the model during delivery.
            const LoadedDeck deck = deckModel->deckForRow(r);
            emit openDeckEditor(deck);
        }
    });

    connect(menu->addAction(tr("Share deck...")), &QAction::triggered, this,
            [this, currentRow, requestedFilePath] {
        if (currentRow() != -1) {
            const QString sharedFilePath = requestedFilePath;
            emit shareDeckRequested(sharedFilePath);
        }
    });

    connect(menu->addAction(tr("Edit Tags")), &QAction::triggered, this, [this, currentRow] {
        if (currentRow() != -1) {
            deckTagsDisplayWidget->openTagEditDlg();
        }
    });

    addSetBannerCardMenu(menu);

    menu->addSeparator();

    connect(menu->addAction(tr("Rename Deck")), &QAction::triggered, this, [this, currentRow] {
        if (currentRow() != -1) {
            actRenameDeck();
        }
    });

    auto saveToClipboardMenu = menu->addMenu(tr("Save Deck to Clipboard"));
    const auto copyDeck = [deckModel, currentRow](bool addComments, bool addSetNameAndNumber) {
        const int r = currentRow();
        if (r != -1) {
            const DeckList deckList = deckModel->dataForRow(r).deck.deckList;
            DeckLoader::saveToClipboard(deckList, addComments, addSetNameAndNumber);
        }
    };

    connect(saveToClipboardMenu->addAction(tr("Annotated")), &QAction::triggered, this,
            [copyDeck] { copyDeck(true, true); });
    connect(saveToClipboardMenu->addAction(tr("Annotated (No set info)")), &QAction::triggered, this,
            [copyDeck] { copyDeck(true, false); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated")), &QAction::triggered, this,
            [copyDeck] { copyDeck(false, true); });
    connect(saveToClipboardMenu->addAction(tr("Not Annotated (No set info)")), &QAction::triggered, this,
            [copyDeck] { copyDeck(false, false); });

    menu->addSeparator();

    connect(menu->addAction(tr("Rename File")), &QAction::triggered, this, [this, currentRow] {
        if (currentRow() != -1) {
            actRenameFile();
        }
    });

    connect(menu->addAction(tr("Delete File")), &QAction::triggered, this, [this, currentRow] {
        if (currentRow() != -1) {
            actDeleteFile();
        }
    });

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
    const QString requestedFilePath = filePath;
    const QPointer<VisualDeckStorageModel> deckModel(model);
    const QPersistentModelIndex requestedRow = previewIndex;

    for (int i = 0; i < bannerCardComboBox->count(); ++i) {
        auto action = bannerCardMenu->addAction(bannerCardComboBox->itemText(i));
        const QVariant card = bannerCardComboBox->itemData(i);
        connect(action, &QAction::triggered, this, [this, deckModel, requestedFilePath, requestedRow, card] {
            if (!deckModel || !requestedRow.isValid() || filePath != requestedFilePath ||
                deckModel->dataForRow(requestedRow.row()).filePath != requestedFilePath) {
                return;
            }
            // A reload can rebuild the combo while this menu remains open.
            const int currentIndex = bannerCardComboBox->findData(card);
            if (currentIndex != -1) {
                bannerCardComboBox->setCurrentIndex(currentIndex);
            }
        });

        // the checkability is purely for visuals
        action->setCheckable(true);
        action->setChecked(bannerCardComboBox->currentIndex() == i);
    }
}

void DeckPreviewWidget::actRenameDeck()
{
    const QPointer<DeckPreviewWidget> self(this);
    const QPointer<VisualDeckStorageModel> deckModel(model);
    const QString originalFilePath = filePath;
    const int r = row();
    if (r == -1) {
        return;
    }

    const QString oldName = deckModel->dataForRow(r).deckName;
    const auto newName = QtUtils::promptText(this, tr("Rename deck"), tr("New name:"), oldName);
    if (!self || !deckModel || !newName || *newName == oldName || filePath != originalFilePath) {
        return;
    }

    // Rows can move while the prompt runs; resolve this same preview again.
    const int currentRow = row();
    if (currentRow == -1) {
        return;
    }
    deckModel->renameDeck(currentRow, *newName);
    // The banner card text updates via the model's dataChanged signal.
}

void DeckPreviewWidget::actRenameFile()
{
    const QPointer<DeckPreviewWidget> self(this);
    const QPointer<VisualDeckStorageModel> deckModel(model);
    const QString originalFilePath = filePath;
    if (row() == -1) {
        return;
    }

    const QString oldName = QFileInfo(originalFilePath).baseName();
    const auto newName = QtUtils::promptText(this, tr("Rename file"), tr("New name:"), oldName);
    if (!self || !deckModel || !newName || newName->isEmpty() || *newName == oldName ||
        filePath != originalFilePath) {
        return;
    }

    const int currentRow = row();
    if (currentRow == -1) {
        return;
    }
    const bool renamed = deckModel->renameFile(currentRow, *newName);
    if (!self || !deckModel) {
        return;
    }
    if (!renamed) {
        QtUtils::messageBox(this, QMessageBox::Critical, tr("Error"), tr("Rename failed"), QMessageBox::Ok);
    }
    // The file path and banner card text update via the model's signals.
}

void DeckPreviewWidget::actDeleteFile()
{
    const QPointer<DeckPreviewWidget> self(this);
    const QPointer<VisualDeckStorageModel> deckModel(model);
    const QString originalFilePath = filePath;
    if (row() == -1) {
        return;
    }

    const auto result = QtUtils::messageBox(this, QMessageBox::Warning, tr("Delete file"),
        tr("Are you sure you want to delete the selected file?"), QMessageBox::Yes | QMessageBox::No);
    if (!self || !deckModel || result != QMessageBox::Yes || filePath != originalFilePath) {
        return;
    }

    const int currentRow = row();
    if (currentRow == -1) {
        return;
    }
    const bool deleted = deckModel->deleteFile(currentRow);
    if (!self || !deckModel) {
        return;
    }
    if (!deleted) {
        QtUtils::messageBox(this, QMessageBox::Critical, tr("Error"), tr("Delete failed"), QMessageBox::Ok);
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
    if (row() == -1) {
        return false;
    }
    const QPointer<DeckPreviewWidget> self(this);
    const QPointer<VisualDeckStorageModel> deckModel(model);
    const QPersistentModelIndex requestedRow = previewIndex;
    const QString requestedFilePath = filePath;
    return DialogConvertDeckToCodFormat::promptIfRequired(
        this, requestedFilePath, [self, deckModel, requestedRow, requestedFilePath] {
            // The overwrite confirmation belongs to this path. A rename during either
            // modal prompt must not convert a different file without its own confirmation.
            if (!self || !deckModel || !requestedRow.isValid() || self->filePath != requestedFilePath ||
                deckModel->dataForRow(requestedRow.row()).filePath != requestedFilePath) {
                return false;
            }
            return deckModel->convertToCockatriceFormat(requestedRow.row());
        });
}
