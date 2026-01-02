/**
 * @file all_zones_card_amount_widget.h
 * @ingroup CardExtraInfoWidgets
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef ALL_ZONES_CARD_AMOUNT_WIDGET_H
#define ALL_ZONES_CARD_AMOUNT_WIDGET_H
#include "../../deck_loader/deck_loader.h"
#include "card_amount_widget.h"

#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>

class AllZonesCardAmountWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AllZonesCardAmountWidget(QWidget *parent,
                                      DeckStateManager *deckStateManager,
                                      QSlider *cardSizeSlider,
                                      const ExactCard &rootCard);
    int getMainboardAmount();
    int getSideboardAmount();
    bool isNonZero();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif

public slots:
    void adjustFontSize(int scalePercentage);
    void setAmounts(int mainboardAmount, int sideboardAmount);

private:
    QVBoxLayout *layout;
    QSlider *cardSizeSlider;
    QLabel *zoneLabelMainboard;
    CardAmountWidget *buttonBoxMainboard;
    QLabel *zoneLabelSideboard;
    CardAmountWidget *buttonBoxSideboard;
};

#endif // ALL_ZONES_CARD_AMOUNT_WIDGET_H
