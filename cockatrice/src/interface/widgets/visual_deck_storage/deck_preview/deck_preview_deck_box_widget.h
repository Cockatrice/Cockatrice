/**
 * @file deck_preview_deck_box_widget.h
 * @ingroup VisualDeckPreviewWidgets
 * @brief One deck in the Visual Deck Storage, rendered as a deck box.
 */

#ifndef DECK_PREVIEW_DECK_BOX_WIDGET_H
#define DECK_PREVIEW_DECK_BOX_WIDGET_H

#include <QColor>
#include <QFont>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <qtmetamacros.h>

class DeckPreviewCardPictureWidget;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QResizeEvent;
class RaiseOnHover;

/**
 * @brief A deck as a box: a painted body with the deck name and card count on a
 * lid band, the deck's colour identity as a slanted mana band under them, and the
 * banner card art inset as a window.
 *
 * The box keeps the footprint the card-size slider gives a card picture, so the
 * grid does not grow: the artwork shrinks into the window instead. Every colour
 * comes from the palette or the theme app colours, so a theme can recolor the box
 * without touching this code.
 *
 * The box owns the hover lift for the whole surface. The picture inside it is a
 * passive render target that never raises on its own, otherwise the artwork would
 * slide up under the lid.
 */
class DeckPreviewDeckBoxWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewDeckBoxWidget(QWidget *parent = nullptr);

    /** @brief The banner card artwork displayed in the box window. */
    [[nodiscard]] DeckPreviewCardPictureWidget *cardPicture() const
    {
        return picture;
    }

    /** @brief The deck name shown on the lid; elided when it does not fit. */
    void setDeckName(const QString &name);

    /** @brief The card count shown on the lid; @p count < 0 hides it (deck still loading). */
    void setCardCount(int count);

    /**
     * @brief The deck's colour identity, shown as a mana strip under the name.
     *
     * The string follows the model's format: WUBRG letters in WUBRG order. The strip
     * draws all five colours dimmed when the setting asks for unused identities, and
     * only the deck's colours at full strength otherwise.
     */
    void setColorIdentity(const QString &identity);

    /**
     * @brief Gives the mana strip a row of its own, or takes that row back.
     *
     * The lid only grows while the strip is drawn, so turning the colour identity
     * setting off restores the single-row lid and its larger artwork window.
     */
    void setColorIdentityVisible(bool visible);

    void setSelected(bool selected);
    [[nodiscard]] bool isSelected() const
    {
        return selected;
    }

    /** @brief Enables or disables the hover lift. */
    void setRaiseOnEnterEnabled(bool enabled);

    /**
     * @brief Sizes the box like a card picture at @p scale, so the grid geometry is
     * unchanged by the box chrome.
     */
    void setScaleFactor(int scale);
    [[nodiscard]] QSize sizeHint() const override;

signals:
    /**
     * @brief A left click anywhere on the box.
     *
     * The lid and the body margins are part of the click surface, so selecting a
     * deck does not depend on hitting the artwork.
     */
    void clicked();

    /** @brief A left double click anywhere on the box. */
    void doubleClicked();

    /** @brief A right click anywhere on the box; the owner pops the deck menu. */
    void contextMenuRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    [[nodiscard]] int lidHeight() const;
    [[nodiscard]] int sideInset() const;
    [[nodiscard]] int lidGap() const;
    [[nodiscard]] int bottomInset() const;
    [[nodiscard]] qreal cornerRadius() const;

    /** @brief The height of the name-and-count row, the lid's first band. */
    [[nodiscard]] int nameRowHeight() const;
    /** @brief The inset the name and the count share from the body edge. */
    [[nodiscard]] int lidPadding() const;
    /** @brief The thickness of the mana band, which is also the height of one segment. */
    [[nodiscard]] int manaBandHeight() const;

    /**
     * @brief The colours the band draws, in WUBRG order.
     *
     * All five while unused identities are shown — a colourless deck included, which is
     * what the standalone colour identity row does — otherwise only the colours the deck
     * has, and nothing at all when the band is hidden or the deck has none to show. The
     * band shares its whole width between them, so it is continuous in every case.
     */
    [[nodiscard]] QStringList manaSymbols() const;

    /**
     * @brief Paints the mana band under the name: one continuous strip of colour
     * segments, each with the mana icon centred on its own colour.
     *
     * Unused segments are faded by the opacity setting, and only drawn at all when the
     * draw-unused-identities setting is on.
     */
    void paintManaBand(QPainter &painter) const;

    /** @brief The area left for the window once the lid and the body margins are taken. */
    [[nodiscard]] QRect windowArea() const;

    /** @brief The largest card-aspect rect inside the window area, so the pixmap fills it. */
    [[nodiscard]] QRect artRect() const;

    void layoutPicture();

    /**
     * @brief Rebuilds the accessible description from the deck name's companions:
     * the selected state, the card count and the colour identity.
     *
     * The lid shows all three as pixels only, so a screen reader needs them spelled out.
     */
    void updateAccessibleDescription();

    [[nodiscard]] QFont nameFont() const;
    [[nodiscard]] QColor countColor() const;

    DeckPreviewCardPictureWidget *picture;
    RaiseOnHover *raiseOnHover;
    QString deckName;
    QString colorIdentity;
    int cardCount = -1;
    int scaleFactor = 100;
    bool selected = false;
    bool colorIdentityVisible = false;
};

#endif // DECK_PREVIEW_DECK_BOX_WIDGET_H
