#include "card_info_frame_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../game_graphics/board/abstract_card_item.h"
#include "../../card_picture_loader/card_picture_to_load.h"
#include "card_info_picture_widget.h"
#include "card_info_text_widget.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/utility/card_ref.h>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>

struct CardRef;

CardInfoFrameWidget::CardInfoFrameWidget(QWidget *parent)
    : QTabWidget(parent), viewTransformationButton(nullptr), cardTextOnly(false)
{
    setContentsMargins(3, 3, 3, 3);
    pic = new CardInfoPictureWidget();
    pic->setObjectName("pic");
    connect(pic, &CardInfoPictureWidget::cardChanged, this,
            qOverload<const ExactCard &>(&CardInfoFrameWidget::setCard));

    text = new CardInfoTextWidget();
    text->setObjectName("text");
    connect(text, &CardInfoTextWidget::linkActivated, this, qOverload<const QString &>(&CardInfoFrameWidget::setCard));

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

    tab1Layout = QtUtils::makeLayout<QVBoxLayout>();
    tab1Layout->setObjectName("tab1Layout");
    tab1Layout->setSpacing(0);
    tab1->setLayout(tab1Layout);

    tab2Layout = QtUtils::makeLayout<QVBoxLayout>();
    tab2Layout->setObjectName("tab2Layout");
    tab2Layout->setSpacing(0);
    tab2->setLayout(tab2Layout);

    splitter = new QSplitter();
    splitter->setObjectName("splitter");
    splitter->setOrientation(Qt::Vertical);

    tab3Layout = QtUtils::makeLayout<QVBoxLayout>();
    tab3Layout->setObjectName("tab3Layout");
    tab3Layout->setSpacing(0);
    tab3Layout->addWidget(splitter);
    tab3->setLayout(tab3Layout);

    cardPageActions = new QWidget(this);
    cardPageActions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *actionsLayout = new QHBoxLayout(cardPageActions);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    openCardPageButton = new QPushButton(cardPageActions);
    openCardPageButton->setObjectName("openCardPageButton");
    connect(openCardPageButton, &QPushButton::clicked, this, &CardInfoFrameWidget::openCardPage);
    actionsLayout->addWidget(openCardPageButton, 1);
    cardPageOptionsButton = new QToolButton(cardPageActions);
    cardPageOptionsButton->setObjectName("cardPageOptionsButton");
    cardPageOptionsButton->setText(QStringLiteral("..."));
    cardPageOptionsButton->setPopupMode(QToolButton::InstantPopup);
    auto *menu = new QMenu(cardPageOptionsButton);
    configureCardPageAction = menu->addAction(QString(), this, &CardInfoFrameWidget::configureCardPage);
    resetCardPageAction = menu->addAction(QString(), this, [] {
        SettingsCache::instance().cardsDisplay().setCardInfoUrlTemplate(
            CardsDisplaySettings::defaultCardInfoUrlTemplate());
    });
    cardPageOptionsButton->setMenu(menu);
    actionsLayout->addWidget(cardPageOptionsButton);
    auto &settings = SettingsCache::instance().cardsDisplay();
    connect(&settings, &CardsDisplaySettings::cardInfoUrlTemplateChanged, this,
            &CardInfoFrameWidget::updateCardPageAction);
    connect(&settings, &CardsDisplaySettings::cardLangChanged, this, &CardInfoFrameWidget::updateCardPageAction);
    setViewMode(settings.getCardInfoViewMode());
}

void CardInfoFrameWidget::retranslateUi()
{
    setTabText(ImageOnlyView, tr("Image"));
    setTabText(TextOnlyView, tr("Description"));
    setTabText(ImageAndTextView, tr("Both"));

    openCardPageButton->setText(tr("Open Card Page"));
    cardPageOptionsButton->setToolTip(tr("Configure card page"));
    cardPageOptionsButton->setAccessibleName(tr("Configure card page"));
    configureCardPageAction->setText(tr("Configure URL..."));
    resetCardPageAction->setText(tr("Reset to Scryfall"));
    updateCardPageAction();

    if (viewTransformationButton) {
        viewTransformationButton->setText(tr("View transformation"));
    }
}

void CardInfoFrameWidget::setViewTransformationButtonVisibility(bool visible)
{
    if (!viewTransformationButton && visible) {
        viewTransformationButton = new QPushButton();
        viewTransformationButton->setObjectName("viewTransformationButton");
        connect(viewTransformationButton, &QPushButton::clicked, this, &CardInfoFrameWidget::viewTransformation);
        refreshLayout();
    } else if (viewTransformationButton && !visible) {
        // Deleting a widget automatically removes it from its parent
        viewTransformationButton->deleteLater();
        viewTransformationButton = nullptr;
    }
}

/**
 * Adds the widgets to the layouts that are relevant to the currently active tab.
 *
 * QWidgets can only have one parent, so we need to re-parent the shared widgets whenever we switch tabs.
 */
void CardInfoFrameWidget::refreshLayout()
{
    switch (currentIndex()) {
        case ImageOnlyView:
        case TextOnlyView:
            // We need to always parent all widgets, even the ones that aren't visible,
            // since an unparented widget becomes free-floating.
            tab1Layout->addWidget(pic, 1);
            if (viewTransformationButton) {
                tab1Layout->addWidget(viewTransformationButton);
            }
            tab2Layout->addWidget(text, 1);
            break;
        case ImageAndTextView:
            splitter->addWidget(pic);
            if (viewTransformationButton) {
                splitter->addWidget(viewTransformationButton);
            }
            splitter->addWidget(text);
            break;
        default:
            break;
    }
    auto *layout = currentIndex() == ImageOnlyView  ? tab1Layout
                   : currentIndex() == TextOnlyView ? tab2Layout
                                                    : tab3Layout;
    layout->addWidget(cardPageActions);
    retranslateUi();
}

void CardInfoFrameWidget::setViewMode(int mode)
{
    if (currentIndex() != mode) {
        setCurrentIndex(mode);
    }

    refreshLayout();

    SettingsCache::instance().cardsDisplay().setCardInfoViewMode(mode);
}

static bool hasTransformation(const CardInfo &info)
{
    for (const auto &cardRelation : info.getAllRelatedCards()) {
        if (cardRelation->getDoesTransform()) {
            return true;
        }
    }
    return false;
}

void CardInfoFrameWidget::setCard(const ExactCard &card)
{
    if (exactCard) {
        disconnect(exactCard.getCardPtr().data(), nullptr, this, nullptr);
    }

    exactCard = card;

    if (exactCard) {
        connect(exactCard.getCardPtr().data(), &QObject::destroyed, this, &CardInfoFrameWidget::clearCard);
    }

    setViewTransformationButtonVisibility(hasTransformation(exactCard.getInfo()));

    text->setCard(exactCard);
    pic->setCard(exactCard);
    updateCardPageAction();
}

void CardInfoFrameWidget::setCard(const QString &cardName)
{
    setCard(CardDatabaseManager::query()->guessCard({cardName}));
}

void CardInfoFrameWidget::setCard(const CardRef &cardRef)
{
    setCard(CardDatabaseManager::query()->getCard(cardRef));
}

void CardInfoFrameWidget::setCard(AbstractCardItem *card)
{
    if (card) {
        setCard(card->getCard());
    }
}

void CardInfoFrameWidget::viewTransformation()
{
    if (exactCard) {
        const auto &cardRelations = exactCard.getInfo().getAllRelatedCards();
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
    setCard(ExactCard());
}

// Treat templates as website addresses, never as commands or local file paths.
static bool isCardPageUrl(const QString &value)
{
    const QUrl url(value, QUrl::StrictMode);
    return url.isValid() && (url.scheme() == "https" || url.scheme() == "http") && !url.host().isEmpty() &&
           url.userInfo().isEmpty();
}

QString CardInfoFrameWidget::cardPageUrl() const
{
    if (!exactCard) {
        return {};
    }
    const auto value = CardPictureToLoad::transformUrlForCard(
        SettingsCache::instance().cardsDisplay().getCardInfoUrlTemplate(), exactCard);
    static const QRegularExpression unresolvedPlaceholder("![^!]+!");
    if (unresolvedPlaceholder.match(value).hasMatch() || !isCardPageUrl(value)) {
        return {};
    }
    return value;
}

void CardInfoFrameWidget::updateCardPageAction()
{
    const auto url = cardPageUrl();
    openCardPageButton->setEnabled(!url.isEmpty());
    openCardPageButton->setToolTip(!exactCard      ? tr("Select a card to open its web page.")
                                   : url.isEmpty() ? tr("This card does not provide the fields required by the URL. "
                                                        "Use Configure URL to choose another template.")
                                                   : url);
}

void CardInfoFrameWidget::openCardPage()
{
    const auto url = cardPageUrl();
    if (!url.isEmpty() && !QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode))) {
        QMessageBox::warning(this, tr("Open Card Page"), tr("The default browser could not open the card page."));
    }
}

void CardInfoFrameWidget::configureCardPage()
{
    auto &settings = SettingsCache::instance().cardsDisplay();
    bool accepted = false;
    const QString value = QInputDialog::getText(this, tr("Card Page URL"),
                                                tr("Enter an http or https URL using the image-download placeholders.\n"
                                                   "Examples: !name!, !setcode_lower!, !set:num!, !sflang!\n"
                                                   "Card properties: !prop:...!  Printing properties: !set:...!\n"
                                                   "The same substring and padding rules apply."),
                                                QLineEdit::Normal, settings.getCardInfoUrlTemplate(), &accepted)
                              .trimmed();
    if (!accepted) {
        return;
    }
    // Validate the address even when no card is selected or a printing lacks a field.
    static const QRegularExpression placeholder("![^!]+!");
    QString address = value;
    address.replace(placeholder, QStringLiteral("card"));
    if (!isCardPageUrl(address)) {
        QMessageBox::warning(this, tr("Card Page URL"),
                             tr("Enter a valid http or https website URL without a username or password."));
        return;
    }
    settings.setCardInfoUrlTemplate(value);
}
