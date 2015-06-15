#include "cardframe.h"

#include "carditem.h"
#include "carddatabase.h"
#include "main.h"
#include "cardinfopicture.h"
#include "cardinfotext.h"
#include "settingscache.h"

#include <QVBoxLayout>

CardFrame::CardFrame(int width, int height,
                        const QString &cardName, QWidget *parent)
    : QTabWidget(parent)
    , info(0)
    , cardTextOnly(false)
{
    setMaximumWidth(width);
    setMinimumWidth(width);
    setMinimumHeight(height);

    pic = new CardInfoPicture(width);
    text = new CardInfoText();

    tab1 = new QWidget(this);
    tab2 = new QWidget(this);
    tab3 = new QWidget(this);
    insertTab(ImageOnlyView, tab1, QString());
    insertTab(TextOnlyView, tab2, QString());
    insertTab(ImageAndTextView, tab3, QString());
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(setViewMode(int)));

    tab1Layout = new QVBoxLayout();
    tab1Layout->setContentsMargins(0, 0, 0, 0);
    tab1Layout->setSpacing(0);
    tab1->setLayout(tab1Layout);

    tab2Layout = new QVBoxLayout();
    tab2Layout->setContentsMargins(0, 0, 0, 0);
    tab2Layout->setSpacing(0);
    tab2->setLayout(tab2Layout);

    tab3Layout = new QVBoxLayout();
    tab3Layout->setContentsMargins(0, 0, 0, 0);
    tab3Layout->setSpacing(0);
    tab3->setLayout(tab3Layout);

    setViewMode(settingsCache->getCardInfoViewMode());

    setCard(db->getCard(cardName));
}

void CardFrame::retranslateUi()
{
    setTabText(ImageOnlyView, tr("Image"));
    setTabText(TextOnlyView, tr("Description"));
    setTabText(ImageAndTextView, tr("Both"));
}

void CardFrame::setViewMode(int mode)
{
    if(currentIndex() != mode)
        setCurrentIndex(mode);

    switch(mode)
    {
        case ImageOnlyView:
        case TextOnlyView:
            tab1Layout->addWidget(pic);
            tab2Layout->addWidget(text);
            break;
        case ImageAndTextView:
            tab3Layout->addWidget(pic);
            tab3Layout->addWidget(text);
            break;
    }

    settingsCache->setCardInfoViewMode(mode);
}

void CardFrame::setCard(CardInfo *card)
{
    if (info)
        disconnect(info, 0, this, 0);
    info = card;
    connect(info, SIGNAL(destroyed()), this, SLOT(clear()));
    text->setCard(info);
    pic->setCard(info);
}

void CardFrame::setCard(const QString &cardName)
{
    setCard(db->getCard(cardName));
}

void CardFrame::setCard(AbstractCardItem *card)
{
    setCard(card->getInfo());
}

void CardFrame::clear()
{
    setCard(db->getCard());
}
