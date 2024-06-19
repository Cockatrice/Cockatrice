#include "cardframe.h"

#include "cardinfopicture.h"
#include "cardinfotext.h"
#include "carditem.h"
#include "main.h"
#include "settingscache.h"

#include <QDebug>
#include <QSplitter>
#include <QVBoxLayout>
#include <utility>

CardFrame::CardFrame(const QString &cardName, QWidget *parent) : QTabWidget(parent), info(nullptr), cardTextOnly(false)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPicture();
    pic->setObjectName("pic");
    text = new CardInfoText();
    text->setObjectName("text");
    connect(text, SIGNAL(linkActivated(const QString &)), this, SLOT(setCard(const QString &)));

    tab1 = new QWidget(this);
    tab2 = new QWidget(this);
    tab3 = new QWidget(this);

    tab1->setObjectName("tab1");
    tab2->setObjectName("tab2");
    tab3->setObjectName("tab3");

    insertTab(ImageOnlyView, tab1, QString());
    insertTab(TextOnlyView, tab2, QString());
    insertTab(ImageAndTextView, tab3, QString());
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(setViewMode(int)));

    cardVersionSelector = new QComboBox(this);
    refreshCardVersionSelector(cardName);
    connect(cardVersionSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCardImage(int)));

    tab1Layout = new QVBoxLayout();
    tab1Layout->setObjectName("tab1Layout");
    tab1Layout->setContentsMargins(0, 0, 0, 0);
    tab1Layout->setSpacing(0);
    tab1->setLayout(tab1Layout);

    tab2Layout = new QVBoxLayout();
    tab2Layout->setObjectName("tab2Layout");
    tab2Layout->setContentsMargins(0, 0, 0, 0);
    tab2Layout->setSpacing(0);
    tab2->setLayout(tab2Layout);

    splitter = new QSplitter();
    splitter->setObjectName("splitter");
    splitter->setOrientation(Qt::Vertical);

    tab3Layout = new QVBoxLayout();
    tab3Layout->setObjectName("tab3Layout");
    tab3Layout->setContentsMargins(0, 0, 0, 0);
    tab3Layout->setSpacing(0);
    tab3Layout->addWidget(splitter);
    tab3->setLayout(tab3Layout);

    setViewMode(SettingsCache::instance().getCardInfoViewMode());

    setCard(db->getCard(cardName));
}

void CardFrame::updateCardImage(int)
{
    const auto &cardImageData = cardVersionSelector->currentData().value<CardImageData>();
    setCard(cardImageData);
}

void CardFrame::refreshCardVersionSelector(const QString &cardName) const
{
    cardVersionSelector->clear();
    const auto &results = db->getAllPrintingsOfCard(cardName);
    for (const auto &x : results) {
        for (const auto &y : x->getSets()) {
            CardImageData data{x, y.getPtr()->getShortName(), y.getProperty("num")};
            QVariant qVariant;
            qVariant.setValue(data);
            cardVersionSelector->addItem(y.getPtr()->getShortName() + " " + y.getProperty("num"), qVariant);
        }
    }
    cardVersionSelector->setDisabled(cardVersionSelector->count() < 2);
}

void CardFrame::retranslateUi()
{
    setTabText(ImageOnlyView, tr("Image"));
    setTabText(TextOnlyView, tr("Description"));
    setTabText(ImageAndTextView, tr("Both"));
}

void CardFrame::setViewMode(int mode)
{
    if (currentIndex() != mode)
        setCurrentIndex(mode);

    switch (mode) {
        case ImageOnlyView:
        case TextOnlyView:
            tab1Layout->addWidget(pic);
            tab1Layout->addWidget(cardVersionSelector);
            tab2Layout->addWidget(text);
            break;
        case ImageAndTextView:
            splitter->addWidget(pic);
            splitter->addWidget(cardVersionSelector);
            splitter->addWidget(text);
            break;
        default:
            break;
    }

    SettingsCache::instance().setCardInfoViewMode(mode);
}

void CardFrame::setCard(const CardImageData &cardImageData)
{
    if (info) {
        disconnect(info.data(), nullptr, this, nullptr);
    }

    info = cardImageData.cardInfoPtr;

    if (info) {
        connect(info.data(), SIGNAL(destroyed()), this, SLOT(clearCard()));
        qDebug() << "Loading in Card" << info->getName() << cardImageData.cardSetName << cardImageData.cardNumber;
        info->setPixmapCacheKey(QLatin1String("card_") + info->getName() + "_" + cardImageData.cardSetName + "_" +
                                cardImageData.cardNumber);
        info->setPrintingSetName(cardImageData.cardSetName);
        info->setPrintingNumber(cardImageData.cardNumber);
    }

    text->setCard(info);
    pic->setCard(info);
}

void CardFrame::setCard(CardInfoPtr card)
{
    if (info) {
        disconnect(info.data(), nullptr, this, nullptr);
    }

    info = std::move(card);

    if (info) {
        connect(info.data(), SIGNAL(destroyed()), this, SLOT(clearCard()));
    }

    text->setCard(info);
    pic->setCard(info);
}

void CardFrame::setCard(const QString &cardName, CardImageData cardMetaData)
{
    refreshCardVersionSelector(cardName);
    // setCard(db->guessCard(cardName));
    setCard(db->getCard(cardName, cardMetaData.cardSetName, cardMetaData.cardNumber));
}

void CardFrame::setCard(AbstractCardItem *card)
{
    if (card) {
        setCard(card->getInfo());
    }
}

void CardFrame::clearCard()
{
    setCard((CardInfoPtr) nullptr);
}
