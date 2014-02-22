#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QMouseEvent>
#include <QDesktopWidget>
#include "cardinfowidget.h"
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"
#include "settingscache.h"

CardInfoWidget::CardInfoWidget(ResizeMode _mode, const QString &cardName, QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags)
    , pixmapWidth(0)
    , aspectRatio((qreal) CARD_HEIGHT / (qreal) CARD_WIDTH)
    , minimized(settingsCache->getCardInfoMinimized()) // Initialize the cardinfo view status from cache.
    , mode(_mode)
    , info(0)
{
    if (mode == ModeGameTab) {
        // Create indexed list of status views for card.
        const QStringList cardInfoStatus = QStringList() << tr("Show card only") << tr("Show text only") << tr("Show full info");
        
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
    loyaltyLabel1 = new QLabel;
    loyaltyLabel2 = new QLabel;

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
    grid->addWidget(loyaltyLabel1, row, 0);
    grid->addWidget(loyaltyLabel2, row++, 1);
    grid->addWidget(textLabel, row, 0, -1, 2);
    grid->setRowStretch(row, 1);
    grid->setColumnStretch(1, 1);

    retranslateUi();
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    if (mode == ModeGameTab) {
        textLabel->setMinimumHeight(100);
        setFixedWidth(sizeHint().width());
    } else if (mode == ModePopUp) {
        QDesktopWidget desktopWidget;
        pixmapWidth = desktopWidget.screenGeometry().height() / 3 / aspectRatio;
        setFixedWidth(pixmapWidth + 150);
    } else
        setFixedWidth(250);
    
    setCard(db->getCard(cardName));
    setMinimized(settingsCache->getCardInfoMinimized());
}

void CardInfoWidget::minimizeClicked(int newMinimized)
{
    // Set new status, and store it in the settings cache.
    setMinimized(newMinimized);
    settingsCache->setCardInfoMinimized(newMinimized);
}

bool CardInfoWidget::shouldShowPowTough()
{
//    return (!info->getPowTough().isEmpty() && (minimized != 0));
    return (minimized != 0);
}

bool CardInfoWidget::shouldShowLoyalty()
{
//    return ((info->getLoyalty() > 0) && (minimized != 0));
    return (minimized != 0);
}

void CardInfoWidget::setMinimized(int _minimized)
{
    minimized = _minimized;

    // Toggle oracle fields according to selected view.
    bool showAll = ((minimized == 1) || (minimized == 2));
    bool showPowTough = info ? (showAll && shouldShowPowTough()) : true;
    bool showLoyalty = info ? (showAll && shouldShowLoyalty()) : true;
    if (mode == ModeGameTab) {
        nameLabel1->setVisible(showAll);
        nameLabel2->setVisible(showAll);
        manacostLabel1->setVisible(showAll);
        manacostLabel2->setVisible(showAll);
        cardtypeLabel1->setVisible(showAll);
        cardtypeLabel2->setVisible(showAll);
        powtoughLabel1->setVisible(showPowTough);
        powtoughLabel2->setVisible(showPowTough);
        loyaltyLabel1->setVisible(showLoyalty);
        loyaltyLabel2->setVisible(showLoyalty);
        textLabel->setVisible(showAll);
    }

    cardPicture->hide();
    cardHeightOffset = minimumSizeHint().height() + 10;
    
    // Set the picture to be shown only at "card only" (0) and "full info" (2)
    if (mode == ModeGameTab) {
        cardPicture->setVisible((minimized == 0) || (minimized == 2));

        if (minimized == 0)
            setMaximumHeight(cardHeightOffset + width() * aspectRatio);
        else
            setMaximumHeight(1000000);
    } else
        cardPicture->show();
    resize(width(), sizeHint().height());
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
    loyaltyLabel2->setText(card->getLoyalty() > 0 ? QString::number(card->getLoyalty()) : QString());
    textLabel->setText(card->getText());

    powtoughLabel1->setVisible(shouldShowPowTough());
    powtoughLabel2->setVisible(shouldShowPowTough());
    loyaltyLabel1->setVisible(shouldShowLoyalty());
    loyaltyLabel2->setVisible(shouldShowLoyalty());
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
    if (pixmapWidth == 0)
        return;
    
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
    loyaltyLabel1->setText(tr("Loyalty:"));
}
void CardInfoWidget::resizeEvent(QResizeEvent * /*event*/)
{
    if (mode == ModePopUp)
        return;
    if ((minimized == 1) && (mode == ModeGameTab)) {
        pixmapWidth = 0;
        return;
    }
    qreal newPixmapWidth = qMax((qreal) 100.0, qMin((qreal) cardPicture->width(), (qreal) ((height() - cardHeightOffset) / aspectRatio)));
    if (newPixmapWidth != pixmapWidth) {
        pixmapWidth = newPixmapWidth;
        updatePixmap();
    }
}

QString CardInfoWidget::getCardName() const
{
    return nameLabel2->text();
}
