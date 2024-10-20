#include "card_info_widget.h"

#include "../../main.h"
#include "card_info_picture.h"
#include "card_info_text.h"
#include "card_item.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <utility>

CardInfoWidget::CardInfoWidget(const QString &cardName, QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags), aspectRatio((qreal)CARD_HEIGHT / (qreal)CARD_WIDTH), info(nullptr)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPicture();
    pic->setObjectName("pic");
    text = new CardInfoText();
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

void CardInfoWidget::setCard(CardInfoPtr card)
{
    if (info)
        disconnect(info.data(), nullptr, this, nullptr);
    info = std::move(card);
    if (info)
        connect(info.data(), SIGNAL(destroyed()), this, SLOT(clear()));

    text->setCard(info);
    pic->setCard(info);
}

void CardInfoWidget::setCard(const QString &cardName)
{
    setCard(db->guessCard(cardName));
    if (info == nullptr) {
        text->setInvalidCardName(cardName);
    }
}

void CardInfoWidget::setCard(AbstractCardItem *card)
{
    setCard(card->getInfo());
}

void CardInfoWidget::clear()
{
    setCard((CardInfoPtr) nullptr);
}
