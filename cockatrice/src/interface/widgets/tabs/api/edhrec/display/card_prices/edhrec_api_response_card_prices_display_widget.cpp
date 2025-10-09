#include "edhrec_api_response_card_prices_display_widget.h"

EdhrecApiResponseCardPricesDisplayWidget::EdhrecApiResponseCardPricesDisplayWidget(QWidget *parent,
                                                                                   const CardPrices &_cardPrices)
    : QWidget(parent), cardPrices(_cardPrices)
{
    layout = new QGridLayout(this);
    setLayout(layout);

    cardHoarderLabel = new QLabel(this);
    cardHoarderPrice = new QLabel(QString::number(cardPrices.getCardhoarder().value("price").toDouble()), this);
    cardKingdomLabel = new QLabel(this);
    cardKingdomPrice = new QLabel(QString::number(cardPrices.getCardkingdom().value("price").toDouble()), this);
    cardMarketLabel = new QLabel(this);
    cardMarketPrice = new QLabel(QString::number(cardPrices.getCardmarket().value("price").toDouble()), this);
    face2faceLabel = new QLabel(this);
    face2facePrice = new QLabel(QString::number(cardPrices.getFace2face().value("price").toDouble()), this);
    manaPoolLabel = new QLabel(this);
    manaPoolPrice = new QLabel(QString::number(cardPrices.getManapool().value("price").toDouble()), this);
    mtgStocksLabel = new QLabel(this);
    mtgStocksPrice = new QLabel(QString::number(cardPrices.getMtgstocks().value("price").toDouble()), this);
    scgLabel = new QLabel(this);
    scgPrice = new QLabel(QString::number(cardPrices.getScg().value("price").toDouble()), this);
    tcglLabel = new QLabel(this);
    tcglPrice = new QLabel(QString::number(cardPrices.getTcgl().value("price").toDouble()), this);
    tcgplayerLabel = new QLabel(this);
    tcgplayerPrice = new QLabel(QString::number(cardPrices.getTcgplayer().value("price").toDouble()), this);

    layout->addWidget(cardHoarderLabel, 0, 0);
    layout->addWidget(cardHoarderPrice, 0, 1);
    layout->addWidget(cardKingdomLabel, 0, 2);
    layout->addWidget(cardKingdomPrice, 0, 3);

    layout->addWidget(cardMarketLabel, 1, 0);
    layout->addWidget(cardMarketPrice, 1, 1);
    layout->addWidget(face2faceLabel, 1, 2);
    layout->addWidget(face2facePrice, 1, 3);

    layout->addWidget(manaPoolLabel, 2, 0);
    layout->addWidget(manaPoolPrice, 2, 1);
    layout->addWidget(mtgStocksLabel, 2, 2);
    layout->addWidget(mtgStocksPrice, 2, 3);

    layout->addWidget(scgLabel, 3, 0);
    layout->addWidget(scgPrice, 3, 1);
    layout->addWidget(tcglLabel, 3, 2);
    layout->addWidget(tcglPrice, 3, 3);

    layout->addWidget(tcgplayerLabel, 4, 0);
    layout->addWidget(tcgplayerPrice, 4, 1);

    retranslateUi();
}

void EdhrecApiResponseCardPricesDisplayWidget::retranslateUi()
{
    cardHoarderLabel->setText(tr("Card Hoarder"));
    cardKingdomLabel->setText(tr("Card Kingdom"));
    cardMarketLabel->setText(tr("Card Market"));
    face2faceLabel->setText(tr("Face 2-Face"));
    manaPoolLabel->setText(tr("Mana Pool"));
    mtgStocksLabel->setText(tr("MTG Stocks"));
    scgLabel->setText(tr("Scg"));
    tcglLabel->setText(tr("Tcgl"));
    tcgplayerLabel->setText(tr("Tcgplayer"));
}
