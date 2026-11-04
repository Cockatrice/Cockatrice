#include "mana_cost_widget.h"

#include "libcockatrice/card/card_info.h"
#include <libcockatrice/card/essence_types.h>
#include "mana_symbol_widget.h"

#include <QHBoxLayout>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QResizeEvent>
#include <QSize>
#include <QtGlobal>
#include <libcockatrice/utility/qt_utils.h>

ManaCostWidget::ManaCostWidget(QWidget *parent, CardInfoPtr _card) : QWidget(parent), card(_card)
{
    layout = QtUtils::makeLayout<QHBoxLayout>(this, 5);
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

QStringList ManaCostWidget::parseManaCost(const QString &essenceCost)
{
    return Essence::parseCost(essenceCost);
}
