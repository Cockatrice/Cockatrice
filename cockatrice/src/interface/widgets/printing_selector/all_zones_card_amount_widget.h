/**
 * @file all_zones_card_amount_widget.h
 * @ingroup CardExtraInfoWidgets
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef ALL_ZONES_CARD_AMOUNT_WIDGET_H
#define ALL_ZONES_CARD_AMOUNT_WIDGET_H
#include <QWidget>
#include <qtmetamacros.h>

class CardAmountWidget;
class DeckStateManager;
class ExactCard;
class QLabel;
class QSlider;
class QVBoxLayout;

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
    int getTokensboardAmount();
    bool isNonZero();

    void enterEvent(QEnterEvent *event) override;

public slots:
    void adjustFontSize(int scalePercentage);
    void setAmounts(int mainboardAmount, int sideboardAmount, int tokensboardAmount);

private:
    QVBoxLayout *layout;
    QSlider *cardSizeSlider;
    QLabel *zoneLabelMainboard;
    CardAmountWidget *buttonBoxMainboard;
    QLabel *zoneLabelSideboard;
    CardAmountWidget *buttonBoxSideboard;
    QLabel *zoneLabelTokensboard;
    CardAmountWidget *buttonBoxTokensboard;
};

#endif // ALL_ZONES_CARD_AMOUNT_WIDGET_H
