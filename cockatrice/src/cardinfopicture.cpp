#include "cardinfopicture.h"

#include <QLabel>
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardInfoPicture::CardInfoPicture(int maximumWidth, QWidget *parent)
	: QLabel(parent)
	, info(0)
	, noPicture(true)
{
	setMaximumWidth(maximumWidth);
}

void CardInfoPicture::setNoPicture(bool status)
{
	if (noPicture != status) {
		noPicture = status;
		emit hasPictureChanged();
	}
}

void CardInfoPicture::setCard(CardInfo *card)
{
	if (info)
		disconnect(info, 0, this, 0);
	info = card;
	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));

	updatePixmap();
}

void CardInfoPicture::updatePixmap()
{
	qreal aspectRatio = (qreal) CARD_HEIGHT / (qreal) CARD_WIDTH;
	qreal pixmapWidth = this->width();

	if (pixmapWidth == 0) {
		setNoPicture(true);
		return;
	}

	QPixmap *resizedPixmap = info->getPixmap(QSize(pixmapWidth, pixmapWidth * aspectRatio));
	if (resizedPixmap) {
		setNoPicture(false);
		this->setPixmap(*resizedPixmap);
	}
	else {
		setNoPicture(true);
		this->setPixmap(*(db->getCard()->getPixmap(QSize(pixmapWidth, pixmapWidth * aspectRatio))));
	}
}
