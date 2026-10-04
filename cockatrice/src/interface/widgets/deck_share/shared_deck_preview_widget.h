/**
 * @file shared_deck_preview_widget.h
 * @ingroup DeckShareWidgets
 */
//! \todo Document this file.

#ifndef SHARED_DECK_PREVIEW_WIDGET_H
#define SHARED_DECK_PREVIEW_WIDGET_H

#include <QWidget>

class ColorIdentityWidget;
class DeckPreviewCardPictureWidget;
class QCheckBox;
class QFrame;
class QKeyEvent;
class QLabel;
class QResizeEvent;
class CardDatabaseQuerier;

/**
 * @brief A selectable preview tile for a deck that has no local file.
 *
 * Renders a banner card picture (looked up by name in the card database), the
 * deck name, color identity and game format. Used to preview decks shared via a
 * cockatrice:// link (metadata from Command_DeckShareList) and the deck
 * currently open in the deck editor.
 *
 * Selection follows the deck-preview covenant: the tile reports its click
 * immediately (no double-click interval delay), a palette(highlight) frame
 * marks the selected tile, and Space/Enter toggles selection from the keyboard.
 * A double click selects the tile and emits activated() so the caller can open
 * just that deck.
 */
class SharedDeckPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SharedDeckPreviewWidget(QWidget *parent,
                                     const CardDatabaseQuerier *querier,
                                     const QString &deckName,
                                     const QString &bannerCardName,
                                     const QString &colorIdentity,
                                     const QString &gameFormat = QString(),
                                     const QString &deckToolTip = QString());

    [[nodiscard]] bool isSelected() const;
    void setSelected(bool selected);

    void setBaseAccessibleName(const QString &name);

signals:
    void selectionToggled(bool selected);
    void activated();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void toggleSelection();
    void activate();

private:
    void updateSelectionVisual(bool selected);
    void updateSelectionFrameGeometry();

    DeckPreviewCardPictureWidget *bannerCardDisplayWidget;
    ColorIdentityWidget *colorIdentityWidget;
    QLabel *gameFormatLabel;
    QCheckBox *selectionCheckBox;
    QFrame *selectionFrame;
    QString baseAccessibleName;
};

#endif // SHARED_DECK_PREVIEW_WIDGET_H