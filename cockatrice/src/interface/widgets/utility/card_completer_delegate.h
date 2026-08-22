/**
 * @file card_completer_delegate.h
 * @ingroup UtilityWidgets
 */
//! \todo Document this file.

#ifndef CARD_COMPLETER_DELEGATE_H
#define CARD_COMPLETER_DELEGATE_H

#include <QCache>
#include <QColor>
#include <QPixmap>
#include <QStyledItemDelegate>

class CardInfo;

/**
 * @brief Paints styled card completer popup rows.
 *
 * Each row shows the card name, type line, set code and mana cost pips,
 * color-coded by the card's color identity. Card data is read directly from
 * the CardSearchModel::CardInfoRole so no extra database lookups are needed.
 */
class CardCompleterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CardCompleterDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Content width of one popup row; the completer popup is sized to fit it.
    static constexpr int PopupRowWidth = 480;

private:
    // Mana symbol pixmaps, loaded once and cached
    mutable QCache<QString, QPixmap> symbolCache;

    // Set short codes, resolved once per card name and cached
    mutable QCache<QString, QString> setCodeCache;

    // Resolve the card's color string ("RG", "W", "", ...) → accent QColor
    static QColor accentForColors(const QString &colors);

    // Draw a single mana symbol pip at centre point
    void drawManaSymbol(QPainter *p, QPoint centre, const QString &symbol, int radius) const;

    // Draw all mana pips for a cost string like "2RG" or "{2}{R}{G}"; split and
    // adventure costs ("1W // W") are drawn as separate groups. Returns the left-most x used
    int
    drawManaCost(QPainter *p, const QRect &row, const QString &manaCost, int radius, int spacing, int rightPad) const;

    // Load (or return cached) a mana icon pixmap; falls back to painted circle
    const QPixmap *cachedSymbolPixmap(const QString &symbol, int size) const;

    // Resolve the preferred printing's set short code for a card
    QString setCodeForCard(const QSharedPointer<CardInfo> &card) const;

    static constexpr int CardRowHeight = 40;
    static constexpr int AccentBarWidth = 5;
    static constexpr int SymbolRadius = 9;
    static constexpr int SymbolSpacing = 2;
    static constexpr int PartGap = 14;
};

#endif // CARD_COMPLETER_DELEGATE_H
