#include "card_info_text_widget.h"

#include "../../../game/board/card_item.h"

#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QTextEdit>
#include <libcockatrice/card/game_specific_terms.h>
#include <libcockatrice/card/relation/card_relation.h>

CardInfoTextWidget::CardInfoTextWidget(QWidget *parent) : QFrame(parent), info(nullptr)
{
    propsLabel = new QLabel;
    propsLabel->setOpenExternalLinks(false);
    propsLabel->setWordWrap(true);
    connect(propsLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));

    textLabel = new QTextEdit();
    textLabel->setReadOnly(true);
    textLabel->setMinimumSize(35, 35);

    propsScroll = new QScrollArea(this);
    propsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    propsScroll->setWidgetResizable(true);
    propsScroll->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    propsScroll->setContentsMargins(0, 0, 0, 0);
    propsScroll->setFrameStyle(QFrame::NoFrame);
    propsScroll->setWidget(propsLabel);

    // blend into normal background color, note that themes may override this!
    propsScroll->viewport()->setStyleSheet("QWidget{background: transparent}");

    auto *grid = new QGridLayout(this);
    grid->addWidget(propsScroll, 0, 0);
    grid->addWidget(textLabel, 1, 0);
    grid->setRowStretch(0, 2);
    grid->setRowStretch(1, 1);

    retranslateUi();
}

void CardInfoTextWidget::setTexts(const QString &propsText, const QString &textText)
{
    propsScroll->setMaximumHeight(0); // reset the max height, otherwise the scrollbar will blink in and out sometimes
    propsLabel->setText(propsText);
    propsScroll->setMinimumWidth(propsLabel->minimumWidth() + propsScroll->verticalScrollBar()->width());
    propsScroll->setMaximumHeight(propsLabel->sizeHint().height());
    textLabel->setText(textText);
}

void CardInfoTextWidget::setCard(CardInfoPtr card)
{
    if (card == nullptr) {
        setTexts("", "");
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
    setTexts(text, card->getText());
}

void CardInfoTextWidget::setInvalidCardName(const QString &cardName)
{
    setTexts(tr("Unknown card:") + " " + cardName, "");
}

void CardInfoTextWidget::retranslateUi()
{
    /*
     * There's no way we can really translate the text currently being rendered.
     * The best we can do is invalidate the current text.
     */
    setTexts("", "");
}
