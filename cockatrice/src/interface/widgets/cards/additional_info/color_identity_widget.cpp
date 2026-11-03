#include "color_identity_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "libcockatrice/card/card_info.h"
#include <libcockatrice/card/essence_types.h>
#include "mana_symbol_widget.h"

#include <QChar>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QList>
#include <QObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QResizeEvent>
#include <QSharedPointer>
#include <QSize>
#include <QtGlobal>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>

ColorIdentityWidget::ColorIdentityWidget(QWidget *parent, const QString &_colorIdentity)
    : QWidget(parent), colorIdentity(_colorIdentity)
{
    layout = QtUtils::makeLayout<QHBoxLayout>(this, 5);
    layout->setAlignment(Qt::AlignCenter); // Ensure icons are centered
    setLayout(layout);

    populateManaSymbolWidgets();

    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageDrawUnusedColorIdentitiesChanged, this,
            &ColorIdentityWidget::toggleUnusedVisibility);
}

ColorIdentityWidget::ColorIdentityWidget(QWidget *parent, const CardInfoPtr &card)
    : ColorIdentityWidget(parent, card->getColors())
{
}

void ColorIdentityWidget::populateManaSymbolWidgets()
{
    // Continuum Universes supports fourteen named Essence codes plus N (colorless).
    const QString &allEssences = Essence::symbols();
    QStringList symbols = parseColorIdentity(colorIdentity);

    // clear old layout
    QtUtils::clearLayoutRec(layout);

    // The freshly created symbols haven't been sized yet, so force the next resize pass
    // to apply the symbol size again.
    lastIconSize = -1;
    lastWidth = -1;

    // populate mana symbols
    if (SettingsCache::instance().visualDeckStorage().getVisualDeckStorageDrawUnusedColorIdentities()) {
        for (const QChar code : allEssences) {
            const QString symbol(code);
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

void ColorIdentityWidget::setColorIdentity(const QString &_colorIdentity)
{
    if (colorIdentity == _colorIdentity) {
        return;
    }

    colorIdentity = _colorIdentity;
    populateManaSymbolWidgets();
}

void ColorIdentityWidget::toggleUnusedVisibility()
{
    populateManaSymbolWidgets();
}

void ColorIdentityWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int totalWidth = event->size().width();
    if (totalWidth == lastWidth && lastIconSize != -1) {
        return;
    }
    lastWidth = totalWidth;

    const int totalHeight = qMax(0, totalWidth / 6); // Set height to 1/4 of the width
    setFixedHeight(totalHeight);

    const int count = layout->count();
    if (count == 0) {
        return;
    }

    const int spacing = layout->spacing();
    const int availableWidth = totalWidth - (spacing * (count - 1));
    const int iconSize = qMin(availableWidth / count, totalHeight); // Ensure icons fit within the new height

    if (iconSize <= 0) {
        lastIconSize = iconSize;
        return;
    }
    if (iconSize == lastIconSize) {
        return;
    }
    lastIconSize = iconSize;

    for (int i = 0; i < count; ++i) {
        if (auto *w = qobject_cast<ManaSymbolWidget *>(layout->itemAt(i)->widget())) {
            w->setFixedSize(iconSize, iconSize);
        }
    }
}

QStringList ColorIdentityWidget::parseColorIdentity(const QString &essenceIdentity)
{
    QStringList symbols;
    for (const QChar code : Essence::normalizeIdentity(essenceIdentity)) {
        symbols.append(QString(code));
    }
    return symbols;
}
