#include "cardinfowidget.h"
#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>

CardInfoWidget::CardInfoWidget(CardDatabase *_db, QWidget *parent)
	: QFrame(parent), db(_db), pixmapHeight(0)
{
	cardPicture = new QLabel();
	cardPicture->setAlignment(Qt::AlignCenter);

	nameLabel1 = new QLabel(tr("Name:"));
	nameLabel2 = new QLabel();
	nameLabel2->setWordWrap(true);
	manacostLabel1 = new QLabel(tr("Mana cost:"));
	manacostLabel2 = new QLabel();
	manacostLabel2->setWordWrap(true);
	cardtypeLabel1 = new QLabel(tr("Card type:"));
	cardtypeLabel2 = new QLabel();
	cardtypeLabel2->setWordWrap(true);
	powtoughLabel1 = new QLabel(tr("P / T:"));
	powtoughLabel2 = new QLabel();

	textLabel = new QTextEdit();
	textLabel->setReadOnly(true);

	QGridLayout *grid = new QGridLayout(this);
	grid->addWidget(cardPicture, 0, 0, 1, 2);
	grid->addWidget(nameLabel1, 1, 0);
	grid->addWidget(nameLabel2, 1, 1);
	grid->addWidget(manacostLabel1, 2, 0);
	grid->addWidget(manacostLabel2, 2, 1);
	grid->addWidget(cardtypeLabel1, 3, 0);
	grid->addWidget(cardtypeLabel2, 3, 1);
	grid->addWidget(powtoughLabel1, 4, 0);
	grid->addWidget(powtoughLabel2, 4, 1);
	grid->addWidget(textLabel, 5, 0, -1, 2);
	grid->setRowStretch(5, 1);

	setFrameStyle(QFrame::Panel | QFrame::Raised);
}

void CardInfoWidget::setCard(CardInfo *card)
{
	if (!card)
		return;

	if (pixmapHeight == 0) {
		QPixmap *bigPixmap = card->loadPixmap();
		pixmapHeight = pixmapWidth * bigPixmap->height() / bigPixmap->width();
	}
	cardPicture->setPixmap(*card->getPixmap(QSize(pixmapWidth, pixmapHeight)));
	
	nameLabel2->setText(card->getName());
	manacostLabel2->setText(card->getManacost());
	cardtypeLabel2->setText(card->getCardType());
	powtoughLabel2->setText(card->getPowTough());
	textLabel->setText(card->getText().join("\n"));
}

void CardInfoWidget::setCard(const QString &cardName)
{
	setCard(db->getCard(cardName));
}
