#include "cardframe.h"

#include "carditem.h"
#include "carddatabase.h"
#include "main.h"
#include "cardinfopicture.h"
#include "cardinfotext.h"
#include "settingscache.h"

#include <QVBoxLayout>
#include <QTabBar>

CardFrame::CardFrame(int width, int height,
                        const QString &cardName, QWidget *parent)
    : QFrame(parent)
    , info(0)
    , cardTextOnly(false)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setMaximumWidth(width);
    setMinimumWidth(width);
    setMinimumHeight(height);

    pic = new CardInfoPicture(width);
    text = new CardInfoText();

    tabBar = new QTabBar(this);
    tabBar->setDrawBase(false);
    tabBar->insertTab(ImageOnlyView, QString());
    tabBar->insertTab(TextOnlyView, QString());
    tabBar->insertTab(ImageAndTextView, QString());
    connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(setViewMode(int)));

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(tabBar);
    layout->addWidget(pic);
    layout->addWidget(text);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    setViewMode(settingsCache->getCardInfoViewMode());

    setCard(db->getCard(cardName));
}

void CardFrame::retranslateUi()
{
    tabBar->setTabText(ImageOnlyView, tr("Image"));
    tabBar->setTabText(TextOnlyView, tr("Description"));
    tabBar->setTabText(ImageAndTextView, tr("Both"));
}

void CardFrame::setViewMode(int mode)
{
    if(tabBar->currentIndex() != mode)
        tabBar->setCurrentIndex(mode);

    switch(mode)
    {
        case ImageOnlyView:
            pic->setVisible(true);
            text->setVisible(false);
            break;
        case TextOnlyView:
            pic->setVisible(false);
            text->setVisible(true);
            break;
        case ImageAndTextView:
            pic->setVisible(true);
            text->setVisible(true);
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
