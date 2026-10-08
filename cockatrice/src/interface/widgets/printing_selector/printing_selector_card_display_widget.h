/**
 * @file printing_selector_card_display_widget.h
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H

#include "libcockatrice/card/printing/exact_card.h"

// IWYU pragma: keep
// ZoneCounts appears inside a QMap in a slot signature, so the moc-generated
// code needs the complete type.
#include "printing_selector.h"

#include <QWidget>
#include <qtmetamacros.h>

class AbstractTabDeckEditor;
class DeckStateManager;
class PrintingSelectorCardOverlayWidget;
class QSlider;
class QString;
class QVBoxLayout;
class SetNameAndCollectorsNumberDisplayWidget;
struct ZoneCounts;
template <class Key, class T> class QMap;

class PrintingSelectorCardDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    PrintingSelectorCardDisplayWidget(QWidget *parent,
                                      AbstractTabDeckEditor *deckEditor,
                                      DeckStateManager *deckStateManager,
                                      QSlider *cardSizeSlider,
                                      const ExactCard &rootCard);

public slots:
    void clampSetNameToPicture();
    void updateCardAmounts(const QMap<QString, ZoneCounts> &uuidToAmounts);

    void resizeEvent(QResizeEvent *event) override;

signals:
    void cardPreferenceChanged();

private:
    ExactCard rootCard;
    QVBoxLayout *layout;
    SetNameAndCollectorsNumberDisplayWidget *setNameAndCollectorsNumberDisplayWidget;
    PrintingSelectorCardOverlayWidget *overlayWidget;
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
