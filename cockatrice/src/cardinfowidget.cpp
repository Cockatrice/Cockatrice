#include <QVBoxLayout>
#include <QDesktopWidget>
#include "cardinfowidget.h"
#include "carditem.h"
#include "carddatabase.h"
#include "cardinfopicture.h"
#include "cardinfotext.h"
#include "main.h"

CardInfoWidget::CardInfoWidget(const QString &cardName, QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags)
    , aspectRatio((qreal) CARD_HEIGHT / (qreal) CARD_WIDTH)
    , info(nullptr)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPicture();
    pic->setObjectName("pic");
    text = new CardInfoText();
    text->setObjectName("text");

    QVBoxLayout * layout = new QVBoxLayout();
    layout->setObjectName("layout");
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(pic, 0, Qt::AlignCenter);
    layout->addWidget(text, 0, Qt::AlignCenter);
    setLayout(layout);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    QDesktopWidget desktopWidget;
    int pixmapHeight = desktopWidget.screenGeometry().height() / 3;
    int pixmapWidth = pixmapHeight / aspectRatio;
    pic->setFixedWidth(pixmapWidth);
    pic->setFixedHeight(pixmapHeight);
    setFixedWidth(pixmapWidth + 150);
    
    setCard(cardName);

    // ensure our parent gets a valid size to position us correctly
    resize(width(), sizeHint().height());
}

void CardInfoWidget::setCard(CardInfo *card)
{
    if (info)
        disconnect(info, nullptr, this, nullptr);
    info = card;
    if(info)
        connect(info, SIGNAL(destroyed()), this, SLOT(clear()));

    text->setCard(info);
    pic->setCard(info);
}

void CardInfoWidget::setCard(const QString &cardName)
{
    setCard(db->getCardBySimpleName(cardName));
}

void CardInfoWidget::setCard(AbstractCardItem *card)
{
    setCard(card->getInfo());
}

void CardInfoWidget::clear()
{
    setCard((CardInfo *) nullptr);
}
