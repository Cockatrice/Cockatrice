#include "card_info_display_widget.h"

#include "../../../game/board/card_item.h"
#include "card_info_picture_widget.h"
#include "card_info_text_widget.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>

CardInfoDisplayWidget::CardInfoDisplayWidget(const CardRef &cardRef, QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags), aspectRatio((qreal)CARD_HEIGHT / (qreal)CARD_WIDTH)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPictureWidget();
    pic->setObjectName("pic");
    text = new CardInfoTextWidget();
    text->setObjectName("text");
    connect(text, &CardInfoTextWidget::linkActivated, this, [this](const QString &card) { setCard({card}); });

    auto *layout = new QVBoxLayout();
    layout->setObjectName("layout");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(pic, 0, Qt::AlignCenter);
    layout->addWidget(text, 0, Qt::AlignCenter);
    setLayout(layout);

    setFrameStyle(QFrame::Panel | QFrame::Raised);

    int pixmapHeight = QGuiApplication::primaryScreen()->geometry().height() / 3;
    int pixmapWidth = static_cast<int>(pixmapHeight / aspectRatio);
    pic->setFixedWidth(pixmapWidth);
    pic->setFixedHeight(pixmapHeight);
    setFixedWidth(pixmapWidth + 150);

    setCard(cardRef);

    // ensure our parent gets a valid size to position us correctly
    resize(width(), sizeHint().height());
}

void CardInfoDisplayWidget::setCard(const ExactCard &card)
{
    if (exactCard)
        disconnect(exactCard.getCardPtr().data(), nullptr, this, nullptr);
    exactCard = card;
    if (exactCard)
        connect(exactCard.getCardPtr().data(), &QObject::destroyed, this, &CardInfoDisplayWidget::clear);

    text->setCard(exactCard);
    pic->setCard(exactCard);
}

void CardInfoDisplayWidget::setCard(const CardRef &cardRef)
{
    setCard(CardDatabaseManager::query()->guessCard(cardRef));
    if (exactCard.isEmpty()) {
        text->setInvalidCardName(cardRef.name);
    }
}

void CardInfoDisplayWidget::setCard(AbstractCardItem *card)
{
    setCard(card->getCard());
}

void CardInfoDisplayWidget::clear()
{
    setCard(ExactCard());
}
