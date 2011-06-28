#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QMouseEvent>
#include "cardinfowidget.h"
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"
#include "settingscache.h"

CardInfoWidget::CardInfoWidget(ResizeMode _mode, QWidget *parent, Qt::WindowFlags flags)
	: QFrame(parent, flags)
	, pixmapWidth(160)
	, aspectRatio((qreal) CARD_HEIGHT / (qreal) CARD_WIDTH)
	, minimized(settingsCache->getCardInfoMinimized()) // Initialize the cardinfo view status from cache.
	, mode(_mode)
	, info(0)
{
	if (mode == ModeGameTab) {
		// Create indexed list of status views for card.
		const QStringList cardInfoStatus = QStringList() << tr("Hide card info") << tr("Show card only") << tr("Show text only") << tr("Show full info");
		
		// Create droplist for cardinfo view selection, and set right current index.
		dropList = new QComboBox();
		dropList->addItems(cardInfoStatus);
		dropList->setCurrentIndex(minimized);
		connect(dropList, SIGNAL(currentIndexChanged(int)), this, SLOT(minimizeClicked(int)));
	}

	cardPicture = new QLabel;
	cardPicture->setAlignment(Qt::AlignCenter);

	nameLabel1 = new QLabel;
	nameLabel2 = new QLabel;
	nameLabel2->setWordWrap(true);
	manacostLabel1 = new QLabel;
	manacostLabel2 = new QLabel;
	manacostLabel2->setWordWrap(true);
	cardtypeLabel1 = new QLabel;
	cardtypeLabel2 = new QLabel;
	cardtypeLabel2->setWordWrap(true);
	powtoughLabel1 = new QLabel;
	powtoughLabel2 = new QLabel;

	textLabel = new QTextEdit();
	textLabel->setReadOnly(true);

	QGridLayout *grid = new QGridLayout(this);
	int row = 0;
	if (mode == ModeGameTab)
		grid->addWidget(dropList, row++, 1, 1, 1, Qt::AlignRight);
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
	if (mode == ModeGameTab) {
		textLabel->setFixedHeight(100);
		setFixedWidth(sizeHint().width());
		setMinimized(settingsCache->getCardInfoMinimized());
	} else if (mode == ModePopUp)
		setFixedWidth(350);
	else
		setFixedWidth(250);
	if (mode != ModeDeckEditor)
		setFixedHeight(sizeHint().height());
}

void CardInfoWidget::minimizeClicked(int newMinimized)
{
	// Set new status, and store it in the settings cache.
	setMinimized(newMinimized);
	settingsCache->setCardInfoMinimized(newMinimized);
}

void CardInfoWidget::setMinimized(int _minimized)
{
	minimized = _minimized;

	// Set the picture to be shown only at "card only" (1) and "full info" (3)
	if (minimized == 1 || minimized == 3) {
		cardPicture->setVisible(true);
	} else {
		cardPicture->setVisible(false);
	}

	// Set the rest of the fields to be shown only at "full info" (3) and "oracle only" (2)
	bool showAll = (minimized == 2 || minimized == 3) ? true : false;

	// Toggle oracle fields as according to selected view.
	nameLabel2->setVisible(showAll);
	nameLabel1->setVisible(showAll);
	manacostLabel1->setVisible(showAll);
	manacostLabel2->setVisible(showAll);
	cardtypeLabel1->setVisible(showAll);
	cardtypeLabel2->setVisible(showAll);
	powtoughLabel1->setVisible(showAll);
	powtoughLabel2->setVisible(showAll);
	textLabel->setVisible(showAll);

	setFixedHeight(sizeHint().height());
}

void CardInfoWidget::setCard(CardInfo *card)
{
	if (info)
		disconnect(info, 0, this, 0);
	info = card;
	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(updatePixmap()));
	connect(info, SIGNAL(destroyed()), this, SLOT(clear()));

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

void CardInfoWidget::clear()
{
	setCard(db->getCard());
}

void CardInfoWidget::updatePixmap()
{
	QPixmap *resizedPixmap = info->getPixmap(QSize(pixmapWidth, pixmapWidth * aspectRatio));
	if (resizedPixmap)
		cardPicture->setPixmap(*resizedPixmap);
	else
		cardPicture->setPixmap(*(db->getCard()->getPixmap(QSize(pixmapWidth, pixmapWidth * aspectRatio))));
}

void CardInfoWidget::retranslateUi()
{
	nameLabel1->setText(tr("Name:"));
	manacostLabel1->setText(tr("Mana cost:"));
	cardtypeLabel1->setText(tr("Card type:"));
	powtoughLabel1->setText(tr("P / T:"));
}

void CardInfoWidget::resizeEvent(QResizeEvent * /*event*/)
{
	if (mode == ModeDeckEditor) {
		pixmapWidth = qMin(width() * 0.95, (height() - 200) / aspectRatio);
		updatePixmap();
	}
}

QString CardInfoWidget::getCardName() const
{
	return nameLabel2->text();
}