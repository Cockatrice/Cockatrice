#include "card_info_frame_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/cards/card_item.h"
#include "../../../../settings/cache_settings.h"
#include "card_info_display_widget.h"
#include "card_info_picture_widget.h"
#include "card_info_text_widget.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <utility>

CardInfoFrameWidget::CardInfoFrameWidget(const QString &cardName, QWidget *parent)
    : QTabWidget(parent), info(nullptr), cardTextOnly(false)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPictureWidget();
    pic->setObjectName("pic");
    connect(pic, &CardInfoPictureWidget::cardChanged, this, qOverload<CardInfoPtr>(&CardInfoFrameWidget::setCard));

    text = new CardInfoTextWidget();
    text->setObjectName("text");
    connect(text, SIGNAL(linkActivated(const QString &)), this, SLOT(setCard(const QString &)));

    viewTransformationButton = new QPushButton();
    viewTransformationButton->setObjectName("viewTransformationButton");
    connect(viewTransformationButton, &QPushButton::clicked, this, &CardInfoFrameWidget::viewTransformation);
    viewTransformationButton->setHidden(true);

    tab1 = new QWidget(this);
    tab2 = new QWidget(this);
    tab3 = new QWidget(this);

    tab1->setObjectName("tab1");
    tab2->setObjectName("tab2");
    tab3->setObjectName("tab3");

    insertTab(ImageOnlyView, tab1, QString());
    insertTab(TextOnlyView, tab2, QString());
    insertTab(ImageAndTextView, tab3, QString());
    connect(this, &CardInfoFrameWidget::currentChanged, this, &CardInfoFrameWidget::setViewMode);

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

    setCard(CardDatabaseManager::getInstance()->getCard(cardName));
}

void CardInfoFrameWidget::retranslateUi()
{
    setTabText(ImageOnlyView, tr("Image"));
    setTabText(TextOnlyView, tr("Description"));
    setTabText(ImageAndTextView, tr("Both"));
    viewTransformationButton->setText(tr("View transformation"));
}

void CardInfoFrameWidget::setViewMode(int mode)
{
    if (currentIndex() != mode)
        setCurrentIndex(mode);

    /* This weird code is to work around the following:
     * - Widgets can only have one parent; adding an already-parented widget will cause the new parent to "steal" it
     * from the old parent
     * - Unparented widgets become freefloating, which we do not want
     */
    switch (mode) {
        case ImageOnlyView:
        case TextOnlyView:
            tab1Layout->addWidget(pic);
            tab1Layout->addWidget(viewTransformationButton);
            tab2Layout->addWidget(text);
            break;
        case ImageAndTextView:
            splitter->addWidget(pic);
            splitter->addWidget(viewTransformationButton);
            splitter->addWidget(text);
            break;
        default:
            break;
    }

    SettingsCache::instance().setCardInfoViewMode(mode);
}

void CardInfoFrameWidget::setCard(CardInfoPtr card)
{
    viewTransformationButton->setHidden(true);
    if (info) {
        disconnect(info.data(), nullptr, this, nullptr);
    }

    info = std::move(card);

    if (info) {
        connect(info.data(), SIGNAL(destroyed()), this, SLOT(clearCard()));
    }

    if (info) {
        const auto &cardRelations = info->getAllRelatedCards();
        for (const auto &cardRelation : cardRelations) {
            if (cardRelation->getDoesTransform()) {
                viewTransformationButton->setHidden(false);
                break;
            }
        }
    }

    text->setCard(info);
    pic->setCard(info);
}

void CardInfoFrameWidget::setCard(const QString &cardName)
{
    setCard(CardDatabaseManager::getInstance()->guessCard(cardName));
}

void CardInfoFrameWidget::setCard(const QString &cardName, const QString &providerId)
{
    setCard(CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, providerId));
}

void CardInfoFrameWidget::setCard(AbstractCardItem *card)
{
    if (card) {
        setCard(card->getInfo());
    }
}

void CardInfoFrameWidget::viewTransformation()
{
    if (info) {
        const auto &cardRelations = info->getAllRelatedCards();
        for (const auto &cardRelation : cardRelations) {
            if (cardRelation->getDoesTransform()) {
                setCard(cardRelation->getName());
                break;
            }
        }
    }
}

void CardInfoFrameWidget::clearCard()
{
    setCard((CardInfoPtr) nullptr);
}
