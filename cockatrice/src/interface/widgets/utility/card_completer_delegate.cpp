#include "card_completer_delegate.h"

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
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

struct ManaColor
{
    QColor fill;
    QColor rim;
    QColor text;
};

ManaColor manaColour(QChar symbol)
{
    switch (symbol.unicode()) {
        case 'W':
            return {QColor(246, 224, 130), QColor(200, 170, 60), QColor(100, 60, 0)};
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
        return QColor(190, 130, 25);
    }

    if (seen.isEmpty()) {
        return QColor(100, 115, 135);
    }

    return manaColour(*seen.begin()).fill;
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
    return {480, CardRowHeight};
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
        (symbol.length() == 1 && QString("WUBRG").contains(symbol)) ? manaColour(symbol[0]) : manaColour(QChar('X'));

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

int CardCompleterDelegate::drawManaCost(QPainter *p, const QRect &row, const QString &manaCost, int radius) const
{
    if (manaCost.isEmpty()) {
        return row.right();
    }

    QStringList symbols;

    if (manaCost.contains('{')) {
        for (const QString &tok : manaCost.split('}', Qt::SkipEmptyParts)) {
            symbols << tok.mid(tok.indexOf('{') + 1);
        }
    } else {
        QString cur;

        for (const QChar c : manaCost) {
            if (c.isDigit()) {
                cur += c;
            } else {
                if (!cur.isEmpty()) {
                    symbols << cur;
                    cur.clear();
                }

                symbols << QString(c);
            }
        }

        if (!cur.isEmpty()) {
            symbols << cur;
        }
    }

    const int diam = radius * 2;

    const int totalW = symbols.size() * diam + (symbols.size() - 1) * SymbolSpacing;

    const int rightPad = 14;

    int x = row.right() - rightPad - totalW + radius;

    const int cy = row.center().y();

    for (const QString &sym : symbols) {
        drawManaSymbol(p, {x, cy}, sym, radius);
        x += diam + SymbolSpacing;
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

    const QColor accent = accentForColors(card->getColors());

    const QColor base = pal.color(QPalette::Base);
    const QColor textColor = pal.color(QPalette::Text);
    const QColor secondaryColor = pal.color(QPalette::PlaceholderText);

    QColor tinted = blend(base, accent, 0.18);

    if (hovered) {
        tinted = blend(tinted, Qt::white, 0.05);
    }

    const QRectF cardRect = rect.adjusted(3, 2, -3, -2);

    // -----------------------------------------------------------------------
    // Main card body
    // -----------------------------------------------------------------------

    QPainterPath path;
    path.addRoundedRect(cardRect, 7, 7);

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

    const QRectF manaZone(cardRect.right() - 110, cardRect.top(), 110, cardRect.height());

    QLinearGradient manaGrad(manaZone.topLeft(), manaZone.bottomLeft());

    manaGrad.setColorAt(0, QColor(0, 0, 0, 18));
    manaGrad.setColorAt(1, QColor(0, 0, 0, 42));

    painter->fillRect(manaZone, manaGrad);

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
        glow.setAlpha(55);

        painter->fillPath(path, glow);

        painter->setPen(QPen(pal.color(QPalette::Highlight), 2));
        painter->drawPath(path);
    }

    // -----------------------------------------------------------------------
    // Mana cost
    // -----------------------------------------------------------------------

    const int costLeft = drawManaCost(painter, cardRect.toRect(), manaCost, SymbolRadius);

    // -----------------------------------------------------------------------
    // Card name + type line + set code
    // -----------------------------------------------------------------------

    const int textLeft = cardRect.left() + AccentBarWidth + 12;
    const int textRight = costLeft - 8;
    const int textWidth = qMax(0, textRight - textLeft);

    // -----------------------------------------------------------------------
    // Card name (top band)
    // -----------------------------------------------------------------------

    {
        const QRect nameRect(textLeft, rect.top() + 2, textWidth, 20);

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

    {
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
