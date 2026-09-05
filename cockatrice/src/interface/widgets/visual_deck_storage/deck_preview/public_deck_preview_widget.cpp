#include "public_deck_preview_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "../../general/layout_containers/flow_widget.h"
#include "deck_preview_tag_display_widget.h"

#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

PublicDeckPreviewWidget::PublicDeckPreviewWidget(QWidget *parent, const RemotePublicDecksModel::DeckEntry &entry)
    : QWidget(parent)
{
    bannerCardDisplayWidget = new DeckPreviewCardPictureWidget(this);
    bannerCardDisplayWidget->setFontSize(24);

    // The whole tile is a single focusable, keyboard-operable control: Tab lands
    // on it and Space/Enter opens the deck, mirroring the shared-deck preview tile.
    setFocusPolicy(Qt::StrongFocus);

    uploadTimeLabel = new QLabel(this);
    uploadTimeLabel->setAlignment(Qt::AlignHCenter);

    colorIdentityWidget = new ColorIdentityWidget(this);

    tagsFlowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    tagsFlowWidget->setSpacing(3, 3);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(bannerCardDisplayWidget);
    layout->addWidget(uploadTimeLabel);
    layout->addWidget(colorIdentityWidget);
    layout->addWidget(tagsFlowWidget);
    setLayout(layout);

    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageShowColorIdentityChanged, this,
            &PublicDeckPreviewWidget::updateColorIdentityVisibility);
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageShowTagsOnDeckPreviewsChanged, this,
            &PublicDeckPreviewWidget::updateTagsVisibility);
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageShowUploadTimeChanged, this,
            &PublicDeckPreviewWidget::updateUploadTimeVisibility);

    setEntry(entry);

    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageClicked, this,
            &PublicDeckPreviewWidget::imageClickedEvent);
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &PublicDeckPreviewWidget::imageDoubleClickedEvent);

    // resizeEvent clamps every child to the banner picture's width, so collect them
    // once here to keep the resize handler from searching the widget tree on every pass.
    fixedWidthChildren = {bannerCardDisplayWidget, uploadTimeLabel, colorIdentityWidget, tagsFlowWidget};
}

void PublicDeckPreviewWidget::resizeEvent(QResizeEvent *event)
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
}

void PublicDeckPreviewWidget::setEntry(const RemotePublicDecksModel::DeckEntry &entry)
{
    deckId = entry.id;

    hasColorIdentity = !entry.colorIdentity.isEmpty();
    colorIdentityWidget->setColorIdentity(entry.colorIdentity);
    updateColorIdentityVisibility();

    const ExactCard bannerCard =
        entry.bannerCardName.isEmpty()
            ? ExactCard()
            : CardDatabaseManager::query()->getCard(CardRef{entry.bannerCardName, entry.bannerCardProvider});
    bannerCardDisplayWidget->setCard(bannerCard);

    // The deck name is the overlay text on the banner, like the local preview.
    bannerCardDisplayWidget->setOverlayText(entry.name);
    setToolTip(entry.name);
    setBaseAccessibleName(entry.name);

    tagsFlowWidget->clearLayout();
    for (const QString &tag : entry.tags) {
        auto *chip = new DeckPreviewTagDisplayWidget(tagsFlowWidget, tag);
        chip->setAttribute(Qt::WA_TransparentForMouseEvents);
        tagsFlowWidget->addWidget(chip);
    }
    hasTags = !entry.tags.isEmpty();
    updateTagsVisibility();

    uploadTimeLabel->setText(tr("Uploaded %1").arg(entry.uploadTime.toString(Qt::TextDate)));
    hasUploadTime = !entry.uploadTime.isNull();
    updateUploadTimeVisibility();
}

void PublicDeckPreviewWidget::updateColorIdentityVisibility()
{
    colorIdentityWidget->setVisible(
        hasColorIdentity && SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowColorIdentity());
}

void PublicDeckPreviewWidget::updateTagsVisibility()
{
    tagsFlowWidget->setVisible(
        hasTags && SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagsOnDeckPreviews());
}

void PublicDeckPreviewWidget::updateUploadTimeVisibility()
{
    uploadTimeLabel->setVisible(hasUploadTime &&
                                SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowUploadTime());
}

void PublicDeckPreviewWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        event->accept();
        emit openDeckRequested(deckId);
        return;
    }
    QWidget::keyPressEvent(event);
}

void PublicDeckPreviewWidget::setBaseAccessibleName(const QString &name)
{
    baseAccessibleName = name;
    setAccessibleName(name);
}

void PublicDeckPreviewWidget::setScaleFactor(int scale)
{
    bannerCardDisplayWidget->setScaleFactor(scale);
}

void PublicDeckPreviewWidget::imageClickedEvent(QMouseEvent * /*event*/, DeckPreviewCardPictureWidget * /*instance*/)
{
    // Reserved: clicking could show a card popup for the banner card.
}

void PublicDeckPreviewWidget::imageDoubleClickedEvent(QMouseEvent * /*event*/,
                                                      DeckPreviewCardPictureWidget * /*instance*/)
{
    emit openDeckRequested(deckId);
}