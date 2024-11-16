#include "card_info_display_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/cards/card_item.h"
#include "../../../../main.h"
#include "card_info_picture_widget.h"
#include "card_info_text_widget.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <utility>

CardInfoDisplayWidget::CardInfoDisplayWidget(const QString &cardName, QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags), aspectRatio((qreal)CARD_HEIGHT / (qreal)CARD_WIDTH), info(nullptr)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPictureWidget();
    pic->setObjectName("pic");
    text = new CardInfoTextWidget();
    text->setObjectName("text");
    connect(text, SIGNAL(linkActivated(const QString &)), this, SLOT(setCard(const QString &)));

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

    setCard(cardName);

    // ensure our parent gets a valid size to position us correctly
    resize(width(), sizeHint().height());
}

void CardInfoDisplayWidget::setCard(CardInfoPtr card)
{
    if (info)
        disconnect(info.data(), nullptr, this, nullptr);
    info = std::move(card);
    if (info)
        connect(info.data(), SIGNAL(destroyed()), this, SLOT(clear()));

    text->setCard(info);
    pic->setCard(info);
}

void CardInfoDisplayWidget::setCard(const QString &cardName)
{
    setCard(CardDatabaseManager::getInstance()->guessCard(cardName));
    if (info == nullptr) {
        text->setInvalidCardName(cardName);
    }
}

void CardInfoDisplayWidget::setCard(AbstractCardItem *card)
{
    setCard(card->getInfo());
}

void CardInfoDisplayWidget::clear()
{
    setCard((CardInfoPtr) nullptr);
}
