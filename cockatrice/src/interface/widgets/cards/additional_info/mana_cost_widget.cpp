#include "mana_cost_widget.h"

#include "mana_symbol_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QSize>
#include <qregularexpression.h>

ManaCostWidget::ManaCostWidget(QWidget *parent, CardInfoPtr _card) : QWidget(parent), card(_card)
{
    layout = new QHBoxLayout(this);
    layout->setSpacing(5); // Small spacing between icons
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    setFixedHeight(50); // Fixed height

    if (card) {
        QString manaCost = card->getManaCost();        // Get mana cost string
        QStringList symbols = parseManaCost(manaCost); // Parse mana cost string

        for (const QString &symbol : symbols) {
            ManaSymbolWidget *manaSymbol = new ManaSymbolWidget(this, symbol, true, false);
            layout->addWidget(manaSymbol);
        }
    }
}

void ManaCostWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QList<ManaSymbolWidget *> manaSymbols = findChildren<ManaSymbolWidget *>();

    if (!manaSymbols.isEmpty()) {
        int totalWidth = event->size().width();
        int spacing = layout->spacing();
        int count = manaSymbols.size();

        // Available width minus total spacing
        int availableWidth = totalWidth - (spacing * (count - 1));
        int iconSize = qMin(50, availableWidth / count);

        for (ManaSymbolWidget *manaSymbol : manaSymbols) {
            manaSymbol->setFixedSize(iconSize, iconSize);
        }
    }
}

QStringList ManaCostWidget::parseManaCost(const QString &cmc)
{
    QStringList symbols;

    // Handle split costs (e.g., "3U // 4UU")
    QStringList splitCosts = cmc.split(" // ");
    for (const QString &part : splitCosts) {
        QRegularExpression regex(R"(\{([^}]+)\}|(\d+)|([WUBRGCSPX]))");
        QRegularExpressionMatchIterator matches = regex.globalMatch(part);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            if (match.captured(1).isEmpty()) { // If no `{}` group was captured, check other groups
                if (!match.captured(2).isEmpty()) {
                    symbols.append(match.captured(2)); // Number match
                } else {
                    symbols.append(match.captured(3)); // Single mana letter match
                }
            } else {
                symbols.append(match.captured(1)); // `{}` enclosed match
            }
        }
    }

    return symbols;
}
