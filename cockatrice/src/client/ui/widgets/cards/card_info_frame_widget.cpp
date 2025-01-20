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

CardPictureAndButtonWidget::CardPictureAndButtonWidget(QWidget *parent) : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    pic = new CardInfoPictureWidget();
    pic->setObjectName("pic");
    connect(pic, &CardInfoPictureWidget::cardChanged, this, [this](auto card) { emit cardChanged(card); });

    setMinimumHeight(pic->minimumHeight());

    layout()->addWidget(pic);

    retranslateUi();
}

void CardPictureAndButtonWidget::retranslateUi()
{
    if (viewTransformationButton) {
        viewTransformationButton->setText(tr("View transformation"));
    }
}

QPushButton *CardPictureAndButtonWidget::createViewTransformationButton()
{
    auto *pushButton = new QPushButton();
    pushButton->setObjectName("viewTransformationButton");
    pushButton->setContentsMargins(0, 0, 0, 0);
    connect(pushButton, &QPushButton::clicked, this, [this] { emit transformationButtonClicked(); });
    return pushButton;
}

void CardPictureAndButtonWidget::setTransformButtonVisibility(bool visible)
{
    if (visible && !viewTransformationButton) {
        viewTransformationButton = createViewTransformationButton();
        layout()->addWidget(viewTransformationButton);
        retranslateUi();
    } else if (!visible && viewTransformationButton) {
        layout()->removeWidget(viewTransformationButton);
        viewTransformationButton->deleteLater();
        viewTransformationButton = nullptr;
    }
}

static bool hasTransformation(const CardInfoPtr &info)
{
    if (!info) {
        return false;
    }

    const auto &relatedCards = info->getAllRelatedCards();
    return std::any_of(relatedCards.cbegin(), relatedCards.cend(),
                       [](const auto &cardRelation) { return cardRelation->getDoesTransform(); });
}

void CardPictureAndButtonWidget::setCard(const CardInfoPtr &card)
{
    pic->setCard(card);
    setTransformButtonVisibility(hasTransformation(card));
}

CardInfoFrameWidget::CardInfoFrameWidget(const QString &cardName, QWidget *parent)
    : QTabWidget(parent), info(nullptr), cardTextOnly(false)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardPictureAndButtonWidget();
    pic->setObjectName("pic");
    connect(pic, &CardPictureAndButtonWidget::cardChanged, this, qOverload<CardInfoPtr>(&CardInfoFrameWidget::setCard));
    connect(pic, &CardPictureAndButtonWidget::transformationButtonClicked, this,
            &CardInfoFrameWidget::viewTransformation);

    text = new CardInfoTextWidget();
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
            tab2Layout->addWidget(text);
            break;
        case ImageAndTextView:
            splitter->addWidget(pic);
            splitter->addWidget(text);
            break;
        default:
            break;
    }

    SettingsCache::instance().setCardInfoViewMode(mode);
}

void CardInfoFrameWidget::setCard(CardInfoPtr card)
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
