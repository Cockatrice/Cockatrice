/**
 * @file printing_selector_card_overlay_widget.h
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H

#include "libcockatrice/card/printing/exact_card.h"

#include <QWidget>
#include <qtmetamacros.h>

class QAction;
class QMenu;
class AbstractTabDeckEditor;
class AllZonesCardAmountWidget;
class CardInfoPictureWidget;
class DeckStateManager;
class QLabel;
class QPoint;
class QSlider;

class PrintingSelectorCardOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardOverlayWidget(QWidget *parent,
                                               AbstractTabDeckEditor *_deckEditor,
                                               DeckStateManager *_deckStateManager,
                                               QSlider *_cardSizeSlider,
                                               const ExactCard &_rootCard);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void customMenu(QPoint point);

signals:
    void cardPreferenceChanged();

public slots:
    void updateCardAmounts(int mainboardAmount, int sideboardAmount, int tokensboardAmount);

private slots:
    void updateVisibility();
    void updatePinBadgeVisibility();

private:
    void initializePinBadge();
    void loadCustomImage();
    void showPreviewForAction(QAction *action);
    void refreshPreview();
    void hidePreview();
    CardInfoPictureWidget *cardInfoPicture;
    AllZonesCardAmountWidget *allZonesCardAmountWidget;
    QLabel *pinBadge = nullptr;
    AbstractTabDeckEditor *deckEditor;
    ExactCard rootCard;
    QLabel *cardOverridePreviewLabel = nullptr;
    ExactCard hoveredOverrideCard;
    QMenu *previewSourceMenu = nullptr;
    QAction *hoveredOverrideAction = nullptr;
};

#endif // PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
