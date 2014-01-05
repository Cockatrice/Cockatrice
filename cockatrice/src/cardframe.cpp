#include "cardframe.h"

#include <QLabel>
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardFrame::CardFrame(const QString &cardName, QWidget *parent, Qt::WindowFlags flags)
	: QLabel(parent, flags)
	, info(0)
{
	this->setAlignment(Qt::AlignCenter);

	setFrameStyle(QFrame::Panel | QFrame::Raised);
	setFixedWidth(250);

	setCard(db->getCard(cardName));
}

void CardFrame::setCard(CardInfo *card)
{
	if (info)
		disconnect(info, 0, this, 0);
	info = card;
	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));
	connect(info, SIGNAL(destroyed()), this, SLOT(clear()));

	updatePixmap();
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

void CardFrame::updatePixmap()
{
	qreal aspectRatio = (qreal) CARD_HEIGHT / (qreal) CARD_WIDTH;
	qreal pixmapWidth = this->width();

	if (pixmapWidth == 0)
		return;

	QPixmap *resizedPixmap = info->getPixmap(QSize(pixmapWidth, pixmapWidth * aspectRatio));
	if (resizedPixmap)
		this->setPixmap(*resizedPixmap);
	else
		this->setPixmap(*(db->getCard()->getPixmap(QSize(pixmapWidth, pixmapWidth * aspectRatio))));
}

QString CardFrame::getCardName() const
{
	return info->getName();
}
