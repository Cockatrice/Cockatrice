#include "shared_deck_preview_widget.h"

#include "../cards/additional_info/color_identity_widget.h"
#include "../cards/deck_preview_card_picture_widget.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_querier.h>

SharedDeckPreviewWidget::SharedDeckPreviewWidget(QWidget *parent,
                                                 const CardDatabaseQuerier *querier,
                                                 const QString &deckName,
                                                 const QString &bannerCardName,
                                                 const QString &colorIdentity,
                                                 const QString &gameFormat,
                                                 const QString &deckToolTip)
    : QWidget(parent)
{
    bannerCardDisplayWidget =
        new DeckPreviewCardPictureWidget(this, false, false, Qt::white, Qt::black, 12, Qt::AlignCenter, true);
    bannerCardDisplayWidget->setScaleFactor(100);
    const ExactCard bannerCard = bannerCardName.isEmpty() ? ExactCard() : querier->getCard(CardRef{bannerCardName, {}});
    bannerCardDisplayWidget->setCard(bannerCard);
    bannerCardDisplayWidget->setOverlayText(deckName);
    setToolTip(deckToolTip.isEmpty() ? deckName : deckToolTip);
    setFocusPolicy(Qt::StrongFocus);
    setBaseAccessibleName(deckName);

    colorIdentityWidget = new ColorIdentityWidget(this, colorIdentity);
    colorIdentityWidget->setVisible(!colorIdentity.isEmpty());

    gameFormatLabel = new QLabel(gameFormat, this);
    gameFormatLabel->setAlignment(Qt::AlignCenter);
    gameFormatLabel->setVisible(!gameFormat.isEmpty());

    selectionCheckBox = new QCheckBox(this);
    selectionCheckBox->setToolTip(tr("Select this deck"));
    // The tile itself is focusable (Space/Enter toggles); keep the checkbox
    // from creating a second tab stop per tile.
    selectionCheckBox->setFocusPolicy(Qt::NoFocus);

    // Selection frame reused from the deck-preview selection covenant: a
    // palette(highlight) border around the banner card, shown while selected.
    selectionFrame = new QFrame(bannerCardDisplayWidget);
    selectionFrame->setAttribute(Qt::WA_TransparentForMouseEvents);
    selectionFrame->setStyleSheet(QStringLiteral(
        "QFrame { border: 2px solid palette(highlight); border-radius: 4px; background: transparent; }"));
    selectionFrame->setVisible(false);

    auto *selectionRow = new QHBoxLayout;
    selectionRow->addWidget(selectionCheckBox);
    selectionRow->addStretch(1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(selectionRow);
    layout->addWidget(bannerCardDisplayWidget, 0, Qt::AlignHCenter);
    layout->addWidget(colorIdentityWidget, 0, Qt::AlignHCenter);
    layout->addWidget(gameFormatLabel, 0, Qt::AlignHCenter);
    setLayout(layout);

    connect(selectionCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        updateSelectionVisual(checked);
        emit selectionToggled(checked);
    });
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageClicked, this,
            &SharedDeckPreviewWidget::toggleSelection);
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &SharedDeckPreviewWidget::activate);
}

bool SharedDeckPreviewWidget::isSelected() const
{
    return selectionCheckBox->isChecked();
}

void SharedDeckPreviewWidget::setSelected(bool selected)
{
    if (isSelected() == selected) {
        return;
    }
    selectionCheckBox->setChecked(selected);
}

void SharedDeckPreviewWidget::updateSelectionVisual(bool selected)
{
    selectionFrame->setVisible(selected);
    selectionFrame->raise();
    if (selected) {
        setAccessibleName(baseAccessibleName + tr(" (selected)"));
    } else {
        setAccessibleName(baseAccessibleName);
    }
}

void SharedDeckPreviewWidget::setBaseAccessibleName(const QString &name)
{
    baseAccessibleName = name;
    setAccessibleName(name);
}

void SharedDeckPreviewWidget::toggleSelection()
{
    setSelected(!isSelected());
}

void SharedDeckPreviewWidget::activate()
{
    setSelected(true);
    emit activated();
}

void SharedDeckPreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateSelectionFrameGeometry();
}

void SharedDeckPreviewWidget::updateSelectionFrameGeometry()
{
    if (selectionFrame == nullptr || bannerCardDisplayWidget == nullptr) {
        return;
    }
    selectionFrame->setGeometry(bannerCardDisplayWidget->rect().adjusted(1, 1, -1, -1));
}

void SharedDeckPreviewWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        toggleSelection();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}