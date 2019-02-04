#include "cardinfotext.h"
#include "carditem.h"
#include "game_specific_terms.h"
#include "main.h"

#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>

CardInfoText::CardInfoText(QWidget *parent) : QFrame(parent), info(nullptr)
{
    nameLabel = new QLabel;
    nameLabel->setOpenExternalLinks(false);
    connect(nameLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));

    textLabel = new QTextEdit();
    textLabel->setReadOnly(true);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(textLabel, 1, 0, -1, 2);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);

    retranslateUi();
}

void CardInfoText::setCard(CardInfoPtr card)
{
    if (card == nullptr) {
        nameLabel->setText("");
        textLabel->setText("");
        return;
    }

    QString text = "<table width=\"100%\" border=0 cellspacing=0 cellpadding=0>";
    text += QString("<tr><td>%1</td><td width=\"5\"></td><td>%2</td></tr>")
                .arg(tr("Name:"), card->getName().toHtmlEscaped());

    QStringList cardProps = card->getProperties();
    foreach (QString key, cardProps) {
        QString keyText = Mtg::getNicePropertyName(key).toHtmlEscaped() + ":";
        text +=
            QString("<tr><td>%1</td><td></td><td>%2</td></tr>").arg(keyText, card->getProperty(key).toHtmlEscaped());
    }

    auto relatedCards = card->getRelatedCards();
    auto reverserelatedCards2Me = card->getReverseRelatedCards2Me();
    if (relatedCards.size() || reverserelatedCards2Me.size()) {
        text += QString("<tr><td>%1</td><td width=\"5\"></td><td>").arg(tr("Related cards:"));

        for (int i = 0; i < relatedCards.size(); ++i) {
            QString tmp = relatedCards.at(i)->getName().toHtmlEscaped();
            text += "<a href=\"" + tmp + "\">" + tmp + "</a><br>";
        }

        for (int i = 0; i < reverserelatedCards2Me.size(); ++i) {
            QString tmp = reverserelatedCards2Me.at(i)->getName().toHtmlEscaped();
            text += "<a href=\"" + tmp + "\">" + tmp + "</a><br>";
        }

        text += "</td></tr>";
    }

    text += "</table>";
    nameLabel->setText(text);
    textLabel->setText(card->getText());
}

void CardInfoText::setInvalidCardName(const QString &cardName)
{
    nameLabel->setText(tr("Unknown card:") + " " + cardName);
    textLabel->setText("");
}

void CardInfoText::retranslateUi()
{
    /*
     * There's no way we can really translate the text currently being rendered.
     * The best we can do is invalidate the current text.
     */
    setInvalidCardName("");
}
