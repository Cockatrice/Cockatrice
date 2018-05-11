#include "cardinfotext.h"

#include "carditem.h"
#include "main.h"
#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>

CardInfoText::CardInfoText(QWidget *parent) : QFrame(parent), info(nullptr)
{
    nameLabel1 = new QLabel;
    nameLabel2 = new QLabel;
    nameLabel2->setWordWrap(true);
    nameLabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    manacostLabel1 = new QLabel;
    manacostLabel2 = new QLabel;
    manacostLabel2->setWordWrap(true);
    manacostLabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    colorLabel1 = new QLabel;
    colorLabel2 = new QLabel;
    colorLabel2->setWordWrap(true);
    colorLabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    cardtypeLabel1 = new QLabel;
    cardtypeLabel2 = new QLabel;
    cardtypeLabel2->setWordWrap(true);
    cardtypeLabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    powtoughLabel1 = new QLabel;
    powtoughLabel2 = new QLabel;
    powtoughLabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    loyaltyLabel1 = new QLabel;
    loyaltyLabel2 = new QLabel;
    loyaltyLabel1->setTextInteractionFlags(Qt::TextBrowserInteraction);

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
// Reset every label which is optionally hidden
void CardInfoText::resetLabels()
{
    manacostLabel1->show();
    manacostLabel2->show();
    powtoughLabel1->show();
    powtoughLabel2->show();
    loyaltyLabel1->show();
    loyaltyLabel2->show();
}
void CardInfoText::setCard(CardInfoPtr card)
{
    if (card) {

        resetLabels();

        nameLabel2->setText(card->getName());
        if(!card->getManaCost().isEmpty())
        {
            manacostLabel2->setText(card->getManaCost());
        }
        else
        {
            manacostLabel1->hide();
            manacostLabel2->hide();
        }
        if(!card->getColors().isEmpty())
        {
            colorLabel2->setText(card->getColors().join(""));
        }
        else
        {
            colorLabel2->setText("Colorless");
        }
        cardtypeLabel2->setText(card->getCardType());
        if(!card->getPowTough().isEmpty())
        {
            powtoughLabel2->setText(card->getPowTough());
        }
        else
        {
            powtoughLabel1->hide();
            powtoughLabel2->hide();
        }
        if(!card->getLoyalty().isEmpty())
        {
            loyaltyLabel2->setText(card->getLoyalty());
        }
        else
        {
            loyaltyLabel1->hide();
            loyaltyLabel1->hide();
        }
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
