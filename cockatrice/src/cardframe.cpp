#include "cardframe.h"

#include "carditem.h"
#include "carddatabase.h"
#include "main.h"
#include "cardinfopicture.h"
#include "cardinfotext.h"
#include "settingscache.h"

#include <QSplitter>
#include <QVBoxLayout>

CardFrame::CardFrame(const QString &cardName, QWidget *parent)
    : QTabWidget(parent), info(nullptr), cardTextOnly(false)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPicture();
    pic->setObjectName("pic");
    text = new CardInfoText();
    text->setObjectName("text");

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
            splitter->addWidget(pic);
            splitter->addWidget(text);
            break;
    }

    settingsCache->setCardInfoViewMode(mode);
}

void CardFrame::setCard(CardInfo *card)
{
    if (info)
        disconnect(info, nullptr, this, nullptr);
    info = card;
    if(info)
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
    setCard((CardInfo*) nullptr);
}
