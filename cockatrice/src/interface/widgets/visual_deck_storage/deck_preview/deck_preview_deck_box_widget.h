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
#include <QWidget>
#include <qtmetamacros.h>

class DeckPreviewCardPictureWidget;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class RaiseOnHover;

/**
 * @brief A deck as a box: a painted body with the deck name and card count on a
 * lid band, and the banner card art inset as a window.
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

    /** @brief The area left for the window once the lid and the body margins are taken. */
    [[nodiscard]] QRect windowArea() const;

    /** @brief The largest card-aspect rect inside the window area, so the pixmap fills it. */
    [[nodiscard]] QRect artRect() const;

    void layoutPicture();

    /**
     * @brief Rebuilds the accessible description from the deck name's companions:
     * the selected state and the card count.
     *
     * The lid shows both as pixels only, so a screen reader needs them spelled out.
     */
    void updateAccessibleDescription();

    [[nodiscard]] QFont nameFont() const;
    [[nodiscard]] QColor countColor() const;

    DeckPreviewCardPictureWidget *picture;
    RaiseOnHover *raiseOnHover;
    QString deckName;
    int cardCount = -1;
    int scaleFactor = 100;
    bool selected = false;
};

#endif // DECK_PREVIEW_DECK_BOX_WIDGET_H
