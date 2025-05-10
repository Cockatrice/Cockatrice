#include "card_info_text_widget.h"

#include "../../../../game/board/card_item.h"
#include "../../../../game/game_specific_terms.h"

#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>

CardInfoTextWidget::CardInfoTextWidget(QWidget *parent) : QFrame(parent), info(nullptr)
{
    nameLabel = new QLabel;
    nameLabel->setOpenExternalLinks(false);
    nameLabel->setWordWrap(true);
    connect(nameLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));

    textLabel = new QTextEdit();
    textLabel->setReadOnly(true);

    auto *grid = new QGridLayout(this);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(textLabel, 1, 0, -1, 2);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);

    retranslateUi();
}

void CardInfoTextWidget::setCard(CardInfoPtr card)
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
    for (const QString &key : cardProps) {
        if (key.contains("-"))
            continue;
        QString keyText = Mtg::getNicePropertyName(key).toHtmlEscaped() + ":";
        text +=
            QString("<tr><td>%1</td><td></td><td>%2</td></tr>").arg(keyText, card->getProperty(key).toHtmlEscaped());
    }

    auto relatedCards = card->getAllRelatedCards();
    if (!relatedCards.empty()) {
        text += QString("<tr><td>%1</td><td width=\"5\"></td><td>").arg(tr("Related cards:"));

        for (auto *relatedCard : relatedCards) {
            QString tmp = relatedCard->getName().toHtmlEscaped();
            text += "<a href=\"" + tmp + "\">" + tmp + "</a><br>";
        }

        text += "</td></tr>";
    }

    text += "</table>";
    nameLabel->setText(text);
    textLabel->setText(card->getText());
}

void CardInfoTextWidget::setInvalidCardName(const QString &cardName)
{
    nameLabel->setText(tr("Unknown card:") + " " + cardName);
    textLabel->setText("");
}

void CardInfoTextWidget::retranslateUi()
{
    /*
     * There's no way we can really translate the text currently being rendered.
     * The best we can do is invalidate the current text.
     */
    setInvalidCardName("");
}
