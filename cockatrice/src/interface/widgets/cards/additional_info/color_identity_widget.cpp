#include "color_identity_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "mana_symbol_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSize>

ColorIdentityWidget::ColorIdentityWidget(QWidget *parent, CardInfoPtr _card) : QWidget(parent), card(_card)
{
    layout = new QHBoxLayout(this);
    layout->setSpacing(5); // Small spacing between icons
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter); // Ensure icons are centered
    setLayout(layout);

    // Define the full WUBRG set (White, Blue, Black, Red, Green)
    QString fullColorIdentity = "WUBRG";

    if (card) {
        manaCost = card->getColors();                       // Get mana cost string
        QStringList symbols = parseColorIdentity(manaCost); // Parse mana cost string

        populateManaSymbolWidgets();
    }
    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageDrawUnusedColorIdentitiesChanged, this,
            &ColorIdentityWidget::toggleUnusedVisibility);
}

ColorIdentityWidget::ColorIdentityWidget(QWidget *parent, QString _manaCost)
    : QWidget(parent), card(nullptr), manaCost(_manaCost)
{
    layout = new QHBoxLayout(this);
    layout->setSpacing(5); // Small spacing between icons
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter); // Ensure icons are centered
    setLayout(layout);

    populateManaSymbolWidgets();

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageDrawUnusedColorIdentitiesChanged, this,
            &ColorIdentityWidget::toggleUnusedVisibility);
}

void ColorIdentityWidget::populateManaSymbolWidgets()
{
    // Define the full WUBRG set (White, Blue, Black, Red, Green)
    QString fullColorIdentity = "WUBRG";
    QStringList symbols = parseColorIdentity(manaCost); // Parse mana cost string

    if (SettingsCache::instance().getVisualDeckStorageDrawUnusedColorIdentities()) {
        for (const QString symbol : fullColorIdentity) {
            auto *manaSymbol = new ManaSymbolWidget(this, symbol, symbols.contains(symbol));
            layout->addWidget(manaSymbol);
        }
    } else {
        for (const QString &symbol : symbols) {
            auto *manaSymbol = new ManaSymbolWidget(this, symbol, symbols.contains(symbol));
            layout->addWidget(manaSymbol);
        }
    }
}

void ColorIdentityWidget::toggleUnusedVisibility()
{
    if (layout != nullptr) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            item->widget()->deleteLater(); // Delete the widget
            delete item;                   // Delete the layout item
        }
    }
    populateManaSymbolWidgets();
}

void ColorIdentityWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QList<ManaSymbolWidget *> manaSymbols = findChildren<ManaSymbolWidget *>();

    if (!manaSymbols.isEmpty()) {
        int totalWidth = event->size().width();
        int totalHeight = totalWidth / 6; // Set height to 1/4 of the width
        setFixedHeight(totalHeight);

        int spacing = layout->spacing();
        int count = manaSymbols.size();
        int availableWidth = totalWidth - (spacing * (count - 1));
        int iconSize = qMin(availableWidth / count, totalHeight); // Ensure icons fit within the new height

        for (ManaSymbolWidget *manaSymbol : manaSymbols) {
            manaSymbol->setFixedSize(iconSize, iconSize);
        }
    }
}

QStringList ColorIdentityWidget::parseColorIdentity(const QString &cmc)
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
