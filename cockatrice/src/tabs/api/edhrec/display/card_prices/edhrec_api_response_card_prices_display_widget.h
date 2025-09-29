/**
 * @file edhrec_api_response_card_prices_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_API_RESPONSE_CARD_PRICES_DISPLAY_WIDGET_H
#define EDHREC_API_RESPONSE_CARD_PRICES_DISPLAY_WIDGET_H

#include "../../api_response/card_prices/edhrec_api_response_card_prices.h"

#include <QGridLayout>
#include <QLabel>
#include <QWidget>

class EdhrecApiResponseCardPricesDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    EdhrecApiResponseCardPricesDisplayWidget(QWidget *parent, const CardPrices &cardPrices);

public slots:
    void retranslateUi();

private:
    CardPrices cardPrices;
    QGridLayout *layout;
    QLabel *cardHoarderLabel;
    QLabel *cardHoarderPrice;
    QLabel *cardKingdomLabel;
    QLabel *cardKingdomPrice;
    QLabel *cardMarketLabel;
    QLabel *cardMarketPrice;
    QLabel *face2faceLabel;
    QLabel *face2facePrice;
    QLabel *manaPoolLabel;
    QLabel *manaPoolPrice;
    QLabel *mtgStocksLabel;
    QLabel *mtgStocksPrice;
    QLabel *scgLabel;
    QLabel *scgPrice;
    QLabel *tcglLabel;
    QLabel *tcglPrice;
    QLabel *tcgplayerLabel;
    QLabel *tcgplayerPrice;
};

#endif // EDHREC_API_RESPONSE_CARD_PRICES_DISPLAY_WIDGET_H
