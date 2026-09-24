#include "card_info_text_widget.h"

#include "../../../game_graphics/board/card_item.h"
#include "../../card_localization.h"

#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <libcockatrice/card/game_specific_terms.h>
#include <libcockatrice/card/relation/card_relation.h>

namespace
{
constexpr int TAGS_PREVIEW_LIMIT = 3;
const QString SHOW_ALL_TAGS_LINK = QStringLiteral("cockatrice://show-all-tags");
} // namespace

CardInfoTextWidget::CardInfoTextWidget(QWidget *parent) : QFrame(parent)
{
    propsLabel = new QLabel;
    propsLabel->setOpenExternalLinks(false);
    propsLabel->setWordWrap(true);
    connect(propsLabel, &QLabel::linkActivated, this, [this](const QString &link) {
        if (link == SHOW_ALL_TAGS_LINK) {
            showAllTags = true;
            setCard(currentCard);
            return;
        }
        emit linkActivated(link);
    });

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

    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::cardLangChanged, this, [this] {
        if (currentCard) {
            setCard(currentCard);
        }
    });
}

void CardInfoTextWidget::setTexts(const QString &propsText, const QString &textText)
{
    propsScroll->setMaximumHeight(0); // reset the max height, otherwise the scrollbar will blink in and out sometimes
    propsLabel->setText(propsText);
    propsScroll->setMinimumWidth(propsLabel->minimumWidth() + propsScroll->verticalScrollBar()->width());
    propsScroll->setMaximumHeight(propsLabel->sizeHint().height());
    textLabel->setText(textText);
}

void CardInfoTextWidget::setCard(const ExactCard &exactCard)
{
    auto card = exactCard.getCardPtr();
    if (card == nullptr) {
        setTexts("", "");
        return;
    }

    // Expanding the tag list re-renders the same card, so only reset the
    // collapsed state when a genuinely different card is shown.
    if (exactCard != currentCard) {
        showAllTags = false;
    }

    QString text = "<table width=\"100%\" border=0 cellspacing=0 cellpadding=0>";
    text += QString("<tr><td>%1</td><td width=\"5\"></td><td>%2</td></tr>")
                .arg(tr("Name:"), CardLocalization::displayName(card).toHtmlEscaped());

    if (!exactCard.getPrinting().isEmpty()) {
        QString setShort = exactCard.getPrinting().getSet()->getShortName().toHtmlEscaped();
        QString cardNum = exactCard.getPrinting().getProperty("num").toHtmlEscaped();

        text += QString("<tr><td>%1</td><td></td><td>%2</td></tr>").arg(tr("Set:"), setShort);

        text += QString("<tr><td>%1</td><td></td><td>%2</td></tr>").arg(tr("Collector Number:"), cardNum);
    }

    QStringList cardProps = card->getProperties();
    for (const QString &key : cardProps) {
        if (key.contains("-")) {
            continue;
        }
        QString keyText = Mtg::getNicePropertyName(key).toHtmlEscaped() + ":";

        if (key == Mtg::Tags) {
            const QStringList tags = card->getProperty(Mtg::Tags).split(" ", Qt::SkipEmptyParts);

            QString tagsText;
            if (!showAllTags && tags.size() > TAGS_PREVIEW_LIMIT) {
                const QStringList preview = tags.mid(0, TAGS_PREVIEW_LIMIT);
                const int remaining = tags.size() - preview.size();
                // Join with non-breaking spaces so "… and N more" never wraps mid-phrase.
                const QString moreText =
                    tr("and %n more", nullptr, remaining).toHtmlEscaped().split(" ").join("&nbsp;");
                tagsText = preview.join(", ").toHtmlEscaped() +
                           QString(" <a href=\"%1\">…&nbsp;%2</a>").arg(SHOW_ALL_TAGS_LINK, moreText);
            } else {
                tagsText = tags.join(", ").toHtmlEscaped();
            }

            text += QString("<tr><td>%1</td><td></td><td>%2</td></tr>").arg(keyText, tagsText);
            continue;
        }

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
    setTexts(text, CardLocalization::displayText(card));
    currentCard = exactCard;
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
