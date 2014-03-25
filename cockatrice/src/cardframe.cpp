#include "cardframe.h"

#include "carditem.h"
#include "carddatabase.h"
#include "main.h"
#include "cardinfopicture.h"
#include "cardinfotext.h"

CardFrame::CardFrame(int width, int height,
                        const QString &cardName, QWidget *parent)
    : QStackedWidget(parent)
    , info(0)
    , cardTextOnly(false)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setMaximumWidth(width);
    setMinimumWidth(width);
    setMaximumHeight(height);
    setMinimumHeight(height);
    pic = new CardInfoPicture(width);
    addWidget(pic);
    text = new CardInfoText();
    addWidget(text);
    connect(pic, SIGNAL(hasPictureChanged()), this, SLOT(hasPictureChanged()));
    setCard(db->getCard(cardName));
}

void CardFrame::setCard(CardInfo *card)
{
    if (info)
        disconnect(info, 0, this, 0);
    info = card;
    connect(info, SIGNAL(destroyed()), this, SLOT(clear()));
    text->setCard(info);
    pic->setCard(info);
}

void CardFrame::setCard(const QString &cardName)
{
    setCard(db->getCard(cardName));
}

void CardFrame::setCard(AbstractCardItem *card)
{
    setCard(card->getInfo());
}

void CardFrame::clear()
{
    setCard(db->getCard());
}

void CardFrame::hasPictureChanged()
{
    if (pic->hasPicture() && !cardTextOnly)
        setCurrentWidget(pic);
    else
        setCurrentWidget(text);
}
