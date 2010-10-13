#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QStyle>
#include "cardinfowidget.h"
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardInfoWidget::CardInfoWidget(bool showMinimizeButton, QWidget *parent, Qt::WindowFlags flags)
	: QFrame(parent, flags), pixmapHeight(pixmapWidth), minimized(false), minimizeButton(0), info(0)
{
	pixmapHeight = pixmapWidth * CARD_HEIGHT / CARD_WIDTH;

	if (showMinimizeButton) {
		minimizeButton = new QPushButton(QIcon(style()->standardIcon(QStyle::SP_ArrowUp)), QString());
		connect(minimizeButton, SIGNAL(clicked()), this, SLOT(minimizeClicked()));
	}
	
	cardPicture = new QLabel;
	cardPicture->setAlignment(Qt::AlignCenter);

	QFont f;
	f.setPixelSize(11);

	nameLabel1 = new QLabel;
	nameLabel1->setFont(f);
	nameLabel2 = new QLabel;
	nameLabel2->setWordWrap(true);
	nameLabel2->setFont(f);
	manacostLabel1 = new QLabel;
	manacostLabel1->setFont(f);
	manacostLabel2 = new QLabel;
	manacostLabel2->setFont(f);
	manacostLabel2->setWordWrap(true);
	cardtypeLabel1 = new QLabel;
	cardtypeLabel1->setFont(f);
	cardtypeLabel2 = new QLabel;
	cardtypeLabel2->setFont(f);
	cardtypeLabel2->setWordWrap(true);
	powtoughLabel1 = new QLabel;
	powtoughLabel1->setFont(f);
	powtoughLabel2 = new QLabel;
	powtoughLabel2->setFont(f);

	textLabel = new QTextEdit();
	textLabel->setReadOnly(true);
	textLabel->setFont(f);

	QGridLayout *grid = new QGridLayout(this);
	int row = 0;
	if (showMinimizeButton)
		grid->addWidget(minimizeButton, row++, 1, 1, 1, Qt::AlignRight);
	grid->addWidget(cardPicture, row++, 0, 1, 2);
	grid->addWidget(nameLabel1, row, 0);
	grid->addWidget(nameLabel2, row++, 1);
	grid->addWidget(manacostLabel1, row, 0);
	grid->addWidget(manacostLabel2, row++, 1);
	grid->addWidget(cardtypeLabel1, row, 0);
	grid->addWidget(cardtypeLabel2, row++, 1);
	grid->addWidget(powtoughLabel1, row, 0);
	grid->addWidget(powtoughLabel2, row++, 1);
	grid->addWidget(textLabel, row, 0, -1, 2);
	grid->setRowStretch(row, 1);
	grid->setColumnStretch(1, 1);

	CardInfo *cardBack = db->getCard();
	setCard(cardBack);

	retranslateUi();
	setFrameStyle(QFrame::Panel | QFrame::Raised);
	if (showMinimizeButton) {
		textLabel->setFixedHeight(100);
		setFixedWidth(sizeHint().width());
	} else
		setFixedWidth(350);
	setFixedHeight(sizeHint().height());
}

void CardInfoWidget::minimizeClicked()
{
	cardPicture->setVisible(minimized);
	nameLabel2->setVisible(minimized);
	nameLabel1->setVisible(minimized);
	manacostLabel1->setVisible(minimized);
	manacostLabel2->setVisible(minimized);
	cardtypeLabel1->setVisible(minimized);
	cardtypeLabel2->setVisible(minimized);
	powtoughLabel1->setVisible(minimized);
	powtoughLabel2->setVisible(minimized);
	textLabel->setVisible(minimized);
	
	minimizeButton->setIcon(style()->standardIcon(minimized ? QStyle::SP_ArrowUp : QStyle::SP_ArrowDown));
	minimized = !minimized;
	setFixedHeight(sizeHint().height());
}

void CardInfoWidget::setCard(CardInfo *card)
{
	if (info)
		disconnect(info, 0, this, 0);
	info = card;
	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));

	updatePixmap();
	nameLabel2->setText(card->getName());
	manacostLabel2->setText(card->getManaCost());
	cardtypeLabel2->setText(card->getCardType());
	powtoughLabel2->setText(card->getPowTough());
	textLabel->setText(card->getText());
}

void CardInfoWidget::setCard(const QString &cardName)
{
	setCard(db->getCard(cardName));
}

void CardInfoWidget::setCard(AbstractCardItem *card)
{
	setCard(card->getInfo());
}

void CardInfoWidget::updatePixmap()
{
	QPixmap *resizedPixmap = info->getPixmap(QSize(pixmapWidth, pixmapHeight));
	if (resizedPixmap)
		cardPicture->setPixmap(*resizedPixmap);
	else
		cardPicture->setPixmap(*(db->getCard()->getPixmap(QSize(pixmapWidth, pixmapHeight))));
}

void CardInfoWidget::retranslateUi()
{
	nameLabel1->setText(tr("Name:"));
	manacostLabel1->setText(tr("Mana cost:"));
	cardtypeLabel1->setText(tr("Card type:"));
	powtoughLabel1->setText(tr("P / T:"));
}
