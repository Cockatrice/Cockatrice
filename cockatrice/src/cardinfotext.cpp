#include "cardinfotext.h"

#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include "carditem.h"
#include "carddatabase.h"
#include "main.h"

CardInfoText::CardInfoText(QWidget *parent)
    : QFrame(parent), info(nullptr)
{
    nameLabel1 = new QLabel;
    nameLabel2 = new QLabel;
    nameLabel2->setWordWrap(true);
    manacostLabel1 = new QLabel;
    manacostLabel2 = new QLabel;
    manacostLabel2->setWordWrap(true);
    colorLabel1 = new QLabel;
    colorLabel2 = new QLabel;
    colorLabel2->setWordWrap(true);
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
    grid->addWidget(nameLabel1, row, 0);
    grid->addWidget(nameLabel2, row++, 1);
    grid->addWidget(manacostLabel1, row, 0);
    grid->addWidget(manacostLabel2, row++, 1);
    grid->addWidget(colorLabel1, row, 0);
    grid->addWidget(colorLabel2, row++, 1);
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
}

void CardInfoText::setCard(CardInfo *card)
{
    if(card)
    {
        nameLabel2->setText(card->getName());
        manacostLabel2->setText(card->getManaCost());
        colorLabel2->setText(card->getColors().join(""));
        cardtypeLabel2->setText(card->getCardType());
        powtoughLabel2->setText(card->getPowTough());
        loyaltyLabel2->setText(card->getLoyalty() > 0 ? QString::number(card->getLoyalty()) : QString());
        textLabel->setText(card->getText());
    } else {
        nameLabel2->setText("");
        manacostLabel2->setText("");
        colorLabel2->setText("");
        cardtypeLabel2->setText("");
        powtoughLabel2->setText("");
        loyaltyLabel2->setText("");
        textLabel->setText("");
    }
}

void CardInfoText::retranslateUi()
{
    nameLabel1->setText(tr("Name:"));
    manacostLabel1->setText(tr("Mana cost:"));
    colorLabel1->setText(tr("Color(s):"));
    cardtypeLabel1->setText(tr("Card type:"));
    powtoughLabel1->setText(tr("P / T:"));
    loyaltyLabel1->setText(tr("Loyalty:"));
}
