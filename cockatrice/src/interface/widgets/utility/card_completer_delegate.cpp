#include "card_completer_delegate.h"

#include "../cards/additional_info/mana_cost_widget.h"

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QSet>
#include <QStyleOptionViewItem>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/printing/printing_info.h>
#include <libcockatrice/models/database/card/card_search_model.h>

// ---------------------------------------------------------------------------
// Internal colour helpers
// ---------------------------------------------------------------------------

namespace
{

// Below this row width the delegate switches to a compact layout so the card
// name stays readable when the popup cannot be as wide as PopupRowWidth.
constexpr int CompactRowWidth = 380;
constexpr int CompactSymbolRadius = 6;
constexpr int CompactSymbolSpacing = 1;
constexpr int CompactRightPad = 8;
constexpr int ManaZoneWidth = 110;
constexpr int CompactManaZoneWidth = 70;
constexpr int RightPad = 14;
constexpr int TextLeftPad = 12;
constexpr int TextRightGap = 8;

// Below this text width the type-line band is dropped so the name can use the
// whole row height instead of being elided to a single readable character.
constexpr int MinNameWidth = 120;

struct ManaColor
{
    QColor fill;
    QColor rim;
    QColor text;
};

ManaColor manaColor(QChar symbol)
{
    switch (symbol.unicode()) {
        case 'W':
            return {QColor(248, 248, 246), QColor(190, 180, 160), QColor(80, 70, 50)};
        case 'U':
            return {QColor(55, 130, 210), QColor(30, 90, 160), QColor(255, 255, 255)};
        case 'B':
            return {QColor(90, 65, 160), QColor(55, 38, 110), QColor(220, 200, 255)};
        case 'R':
            return {QColor(210, 55, 55), QColor(150, 30, 30), QColor(255, 255, 255)};
        case 'G':
            return {QColor(45, 148, 90), QColor(28, 95, 58), QColor(255, 255, 255)};
        default:
            return {QColor(100, 115, 135), QColor(65, 78, 95), QColor(230, 235, 240)};
    }
}

QColor blend(QColor a, QColor b, qreal t)
{
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t, a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t, a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}

} // namespace

// ---------------------------------------------------------------------------

QColor CardCompleterDelegate::accentForColors(const QString &colors)
{
    if (colors.isEmpty()) {
        return QColor(100, 115, 135);
    }

    QSet<QChar> seen;
    for (const QChar c : colors) {
        if (QString("WUBRG").contains(c)) {
            seen.insert(c);
        }
    }

    if (seen.size() > 1) {
        return QColor(205, 145, 25);
    }

    if (seen.isEmpty()) {
        return QColor(100, 115, 135);
    }

    return manaColor(*seen.begin()).fill;
}

// ---------------------------------------------------------------------------

CardCompleterDelegate::CardCompleterDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    symbolCache.setMaxCost(64);
    setCodeCache.setMaxCost(64);
}

// ---------------------------------------------------------------------------
// sizeHint
// ---------------------------------------------------------------------------

QSize CardCompleterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (!index.isValid()) {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    // Fixed wide rows so the popup has room for name, type line, set and mana
    return {PopupRowWidth, CardRowHeight};
}

// ---------------------------------------------------------------------------
// Mana symbol painting
// ---------------------------------------------------------------------------

const QPixmap *CardCompleterDelegate::cachedSymbolPixmap(const QString &symbol, int size) const
{
    const QString key = symbol + QString::number(size);

    if (symbolCache.contains(key)) {
        return symbolCache[key];
    }

    QPixmap src(QString("theme:icons/mana/%1").arg(symbol));

    if (!src.isNull()) {
        auto *pm = new QPixmap(src.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        symbolCache.insert(key, pm);
        return pm;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------

void CardCompleterDelegate::drawManaSymbol(QPainter *p, QPoint centre, const QString &symbol, int radius) const
{
    const QRect pip(centre.x() - radius, centre.y() - radius, radius * 2, radius * 2);

    const QPixmap *px = cachedSymbolPixmap(symbol, radius * 2);

    if (px && !px->isNull()) {
        p->drawPixmap(pip, *px);
        return;
    }

    bool isNumeric = false;
    const int numVal = symbol.toInt(&isNumeric);

    const QString label = isNumeric ? QString::number(numVal) : symbol;

    const ManaColor mc =
        (symbol.length() == 1 && QString("WUBRG").contains(symbol)) ? manaColor(symbol[0]) : manaColor(QChar('X'));

    QPainterPath circle;
    circle.addEllipse(pip);

    p->save();
    p->setClipPath(circle);
    p->fillPath(circle, mc.fill);
    p->restore();

    p->setPen(QPen(mc.rim, 1.2));
    p->setBrush(Qt::NoBrush);
    p->drawEllipse(pip.adjusted(1, 1, -1, -1));

    QFont f = p->font();
    f.setPixelSize(qMax(radius - 1, 7));
    f.setBold(true);

    p->setFont(f);
    p->setPen(mc.text);
    p->drawText(pip, Qt::AlignCenter, label);
}

// ---------------------------------------------------------------------------

int CardCompleterDelegate::drawManaCost(QPainter *p,
                                        const QRect &row,
                                        const QString &manaCost,
                                        int radius,
                                        int spacing,
                                        int rightPad) const
{
    if (manaCost.isEmpty()) {
        return row.right();
    }

    const int diam = radius * 2;

    // Split, adventure, aftermath and prepare cards store both halves of the
    // cost joined by "//" (e.g. "1W // W"); draw each half as its own group.
    static const QRegularExpression splitRegex("\\s*//\\s*");

    QList<QStringList> parts;

    for (const QString &part : manaCost.split(splitRegex, Qt::SkipEmptyParts)) {
        const QStringList symbols = ManaCostWidget::parseManaCost(part);

        if (!symbols.isEmpty()) {
            parts.append(symbols);
        }
    }

    int totalW = 0;

    for (int i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            totalW += PartGap;
        }

        totalW += parts.at(i).size() * diam + qMax(0, parts.at(i).size() - 1) * spacing;
    }

    int x = row.right() - rightPad - totalW + radius;

    const int cy = row.center().y();

    for (int i = 0; i < parts.size(); ++i) {
        const QStringList &symbols = parts.at(i);

        for (const QString &sym : symbols) {
            drawManaSymbol(p, {x, cy}, sym, radius);
            x += diam + spacing;
        }

        if (i < parts.size() - 1) {
            x += PartGap - spacing;
        }
    }

    return row.right() - rightPad - totalW - 10;
}

// ---------------------------------------------------------------------------

QString CardCompleterDelegate::setCodeForCard(const QSharedPointer<CardInfo> &card) const
{
    if (!card) {
        return QString();
    }

    const QString name = card->getName();

    if (setCodeCache.contains(name)) {
        return *setCodeCache[name];
    }

    QString code;

    const PrintingInfo printing = CardDatabaseManager::query()->getPreferredPrinting(card);

    if (auto set = printing.getSet()) {
        code = set->getShortName();
    }

    auto *cached = new QString(code);
    setCodeCache.insert(name, cached);
    return code;
}

// ---------------------------------------------------------------------------
// paint
// ---------------------------------------------------------------------------

void CardCompleterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    const QRect rect = option.rect;
    const QPalette &pal = option.palette;

    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered = option.state & QStyle::State_MouseOver;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    auto card = index.data(CardSearchModel::CardInfoRole).value<QSharedPointer<CardInfo>>();

    if (!card) {
        painter->fillRect(rect, pal.color(QPalette::Base));
        painter->restore();
        return;
    }

    const QString cardName = card->getName();
    const QString manaCost = card->getManaCost();
    const QString typeLine = card->getCardType();
    const QString setCode = setCodeForCard(card);

    // When the row is too narrow for the full layout, use smaller mana pips and
    // a slimmer mana zone so the card name keeps as much room as possible.
    const bool compact = rect.width() < CompactRowWidth;
    const int symbolRadius = compact ? CompactSymbolRadius : SymbolRadius;
    const int symbolSpacing = compact ? CompactSymbolSpacing : SymbolSpacing;
    const int rightPad = compact ? CompactRightPad : RightPad;
    const int manaZoneWidth = compact ? CompactManaZoneWidth : ManaZoneWidth;

    const QColor accent = accentForColors(card->getColors());

    const QColor base = pal.color(QPalette::Base);
    const QColor textColor = pal.color(QPalette::Text);
    const QColor secondaryColor = pal.color(QPalette::PlaceholderText);

    QColor tinted = blend(base, accent, 0.40);

    if (hovered) {
        tinted = blend(tinted, Qt::white, 0.05);
    }

    const QRectF cardRect = rect.adjusted(3, 2, -3, -2);

    // -----------------------------------------------------------------------
    // Main card body
    // -----------------------------------------------------------------------

    QPainterPath path;
    path.addRoundedRect(cardRect, 7, 7);

    painter->save();
    painter->setClipPath(path);

    QLinearGradient bodyGrad(cardRect.topLeft(), cardRect.bottomLeft());

    bodyGrad.setColorAt(0.0, blend(tinted, Qt::white, 0.10));
    bodyGrad.setColorAt(1.0, blend(tinted, Qt::black, 0.18));

    painter->fillPath(path, bodyGrad);

    // -----------------------------------------------------------------------
    // Accent strip
    // -----------------------------------------------------------------------

    QRectF accentRect(cardRect.left(), cardRect.top(), AccentBarWidth, cardRect.height());

    QLinearGradient accentGrad(accentRect.topLeft(), accentRect.bottomLeft());

    accentGrad.setColorAt(0.0, blend(accent, Qt::white, 0.20));
    accentGrad.setColorAt(1.0, blend(accent, Qt::black, 0.25));

    painter->fillRect(accentRect, accentGrad);

    // -----------------------------------------------------------------------
    // Right mana zone
    // -----------------------------------------------------------------------

    const QRectF manaZone(cardRect.right() - manaZoneWidth, cardRect.top(), manaZoneWidth, cardRect.height());

    QLinearGradient manaGrad(manaZone.topLeft(), manaZone.bottomLeft());

    manaGrad.setColorAt(0, QColor(0, 0, 0, 18));
    manaGrad.setColorAt(1, QColor(0, 0, 0, 42));

    painter->fillRect(manaZone, manaGrad);

    painter->restore();

    // -----------------------------------------------------------------------
    // Border
    // -----------------------------------------------------------------------

    QColor border = blend(accent, Qt::black, 0.45);

    if (hovered) {
        border = blend(border, Qt::white, 0.18);
    }

    painter->setPen(QPen(border, 1.2));
    painter->drawPath(path);

    // -----------------------------------------------------------------------
    // Selection glow
    // -----------------------------------------------------------------------

    if (selected) {
        QColor glow = pal.color(QPalette::Highlight);
        glow.setAlpha(30);

        painter->fillPath(path, glow);

        painter->setPen(QPen(pal.color(QPalette::Highlight), 2));
        painter->drawPath(path);
    }

    // -----------------------------------------------------------------------
    // Mana cost
    // -----------------------------------------------------------------------

    const int costLeft = drawManaCost(painter, cardRect.toRect(), manaCost, symbolRadius, symbolSpacing, rightPad);

    // -----------------------------------------------------------------------
    // Card name + type line + set code
    // -----------------------------------------------------------------------

    const int textLeft = cardRect.left() + AccentBarWidth + TextLeftPad;
    const int textRight = costLeft - TextRightGap;
    const int textWidth = qMax(0, textRight - textLeft);

    // If even the compact layout leaves too little room for a readable name,
    // drop the type-line band and let the name use the full row height.
    const bool showInfoBand = textWidth >= MinNameWidth;

    // -----------------------------------------------------------------------
    // Card name (top band)
    // -----------------------------------------------------------------------

    {
        const QRect nameRect = showInfoBand ? QRect(textLeft, rect.top() + 2, textWidth, 20)
                                            : QRect(textLeft, rect.top() + 2, textWidth, rect.height() - 4);

        QFont f = option.font;
        f.setPixelSize(13);
        f.setBold(true);

        painter->setFont(f);

        const QString nameText = QFontMetrics(f).elidedText(cardName, Qt::ElideRight, nameRect.width());

        painter->setPen(QColor(0, 0, 0, 140));
        painter->drawText(nameRect.translated(0, 1), Qt::AlignLeft | Qt::AlignVCenter, nameText);

        painter->setPen(textColor);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);
    }

    // -----------------------------------------------------------------------
    // Type line + set code (bottom band)
    // -----------------------------------------------------------------------

    if (showInfoBand) {
        const QRect infoRect(textLeft, rect.top() + 22, textWidth, rect.height() - 24);

        QFont f = option.font;
        f.setPixelSize(10);

        painter->setFont(f);

        QString infoLine = typeLine;

        if (!setCode.isEmpty()) {
            infoLine += " \u00b7 " + setCode;
        }

        const QString infoText = QFontMetrics(f).elidedText(infoLine, Qt::ElideRight, infoRect.width());

        painter->setPen(QColor(0, 0, 0, 120));
        painter->drawText(infoRect.translated(0, 1), Qt::AlignLeft | Qt::AlignVCenter, infoText);

        painter->setPen(secondaryColor);
        painter->drawText(infoRect, Qt::AlignLeft | Qt::AlignVCenter, infoText);
    }

    painter->restore();
}
