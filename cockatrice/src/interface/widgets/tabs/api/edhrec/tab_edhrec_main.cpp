#include "tab_edhrec_main.h"

#include "../../../../../client/settings/cache_settings.h"
#include "../../../cards/card_size_widget.h"
#include "../../../quick_settings/settings_button_widget.h"
#include "../../../utility/completer_utils.h"
#include "../../tab_supervisor.h"
#include "api_response/average_deck/edhrec_average_deck_api_response.h"
#include "api_response/commander/edhrec_commander_api_response.h"
#include "api_response/top_cards/edhrec_top_cards_api_response.h"
#include "api_response/top_commanders/edhrec_top_commanders_api_response.h"
#include "api_response/top_tags/edhrec_top_tags_api_response.h"
#include "display/commander/edhrec_commander_api_response_display_widget.h"
#include "display/top_cards/edhrec_top_cards_api_response_display_widget.h"
#include "display/top_commander/edhrec_top_commanders_api_response_display_widget.h"
#include "display/top_tags/edhrec_top_tags_api_response_display_widget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QRegularExpression>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/deck_list/loaded_deck.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <libcockatrice/models/database/card_database_display_model.h>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/network/network_manager.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <version_string.h>

static bool canBeCommander(const CardInfoPtr &cardInfo)
{
    return ((cardInfo->getCardType().contains("Legendary", Qt::CaseInsensitive) &&
             cardInfo->getCardType().contains("Creature", Qt::CaseInsensitive))) ||
           cardInfo->getText().contains("can be your commander", Qt::CaseInsensitive);
}

TabEdhRecMain::TabEdhRecMain(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    networkManager = NetworkManagerFactory::create(this);

    container = new QWidget(this);
    mainLayout = new QVBoxLayout(container);
    container->setLayout(mainLayout);

    navigationContainer = new QWidget(container);
    navigationContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    navigationLayout = new QHBoxLayout(navigationContainer);
    navigationLayout->setSpacing(5);
    navigationContainer->setLayout(navigationLayout);

    cardsPushButton = new QPushButton(navigationContainer);
    connect(cardsPushButton, &QPushButton::clicked, this, &TabEdhRecMain::getTopCards);
    topCommandersPushButton = new QPushButton(navigationContainer);
    connect(topCommandersPushButton, &QPushButton::clicked, this, &TabEdhRecMain::getTopCommanders);
    tagsPushButton = new QPushButton(navigationContainer);
    connect(tagsPushButton, &QPushButton::clicked, this, &TabEdhRecMain::getTopTags);

    searchBar = new QLineEdit(this);
    auto cardDatabaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
    auto displayModel = new CardDatabaseDisplayModel(this);
    displayModel->setSourceModel(cardDatabaseModel);

    const CardCompleterSetup cardSetup = createCardCompleter(displayModel, this);
    searchBar->setCompleter(cardSetup.completer);

    // Update suggestions dynamically
    connectCardCompleterSearch(searchBar, cardSetup);

    searchPushButton = new QPushButton(navigationContainer);
    connect(searchPushButton, &QPushButton::clicked, this, &TabEdhRecMain::doSearch);
    connect(searchBar, &QLineEdit::returnPressed, this, &TabEdhRecMain::doSearch);

    settingsButton = new SettingsButtonWidget(this);

    cardSizeSlider = new CardSizeWidget(this, nullptr, SettingsCache::instance().cardsDisplay().getEDHRecCardSize());
    connect(cardSizeSlider, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance().cardsDisplay(),
            &CardsDisplaySettings::setEDHRecCardSize);

    settingsButton->addSettingsWidget(cardSizeSlider);

    navigationLayout->addWidget(cardsPushButton);
    navigationLayout->addWidget(topCommandersPushButton);
    navigationLayout->addWidget(tagsPushButton);
    navigationLayout->addWidget(searchBar);
    navigationLayout->addWidget(searchPushButton);
    navigationLayout->addWidget(settingsButton);

    currentPageDisplay = new QWidget(container);
    currentPageLayout = new QVBoxLayout(currentPageDisplay);
    currentPageDisplay->setLayout(currentPageLayout);

    mainLayout->addWidget(navigationContainer);
    auto *requestStatusRow = new QHBoxLayout;
    requestStatusLabel = new QLabel(container);
    requestStatusLabel->setTextFormat(Qt::PlainText);
    requestStatusLabel->setWordWrap(true);
    retryButton = new QPushButton(container);
    retryButton->hide();
    connect(retryButton, &QPushButton::clicked, this, [this]() { startRequest(requestUrl, requestKind); });
    requestStatusRow->addWidget(requestStatusLabel, 1);
    requestStatusRow->addWidget(retryButton);
    mainLayout->addLayout(requestStatusRow);
    mainLayout->addWidget(currentPageDisplay);

    // Ensure navigation stays at the top and currentPageDisplay takes remaining space
    mainLayout->setStretch(0, 0); // navigationContainer gets minimum space
    mainLayout->setStretch(2, 1); // currentPageDisplay expands as much as possible

    setCentralWidget(container);

    TabEdhRecMain::retranslateUi();

    getTopCards();
}

TabEdhRecMain::~TabEdhRecMain()
{
    cancelRequest();
}

void TabEdhRecMain::cancelRequest()
{
    const QPointer<QNetworkReply> previousReply = activeReply;
    activeReply = nullptr;
    if (previousReply) {
        disconnect(previousReply, nullptr, this, nullptr);
        previousReply->abort();
        previousReply->deleteLater();
    }
}

void TabEdhRecMain::startRequest(QUrl url, RequestKind kind)
{
    cancelRequest();
    requestUrl = url;
    requestKind = kind;
    retryButton->hide();
    requestStatusLabel->setText(tr("Loading EDHREC…"));
    requestStatusLabel->show();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    auto *reply = networkManager->get(request);
    activeReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { processApiJson(reply); });
}

void TabEdhRecMain::showRequestError(const QString &error, bool canRetry)
{
    requestStatusLabel->setText(error);
    requestStatusLabel->show();
    retryButton->setVisible(canRetry);
}

void TabEdhRecMain::clearPage()
{
    QtUtils::clearLayoutRec(currentPageLayout, QtUtils::DeletePolicy::Immediate);
}

void TabEdhRecMain::clearCardSelection()
{
    cardToQuery.clear();
    emit tabTextChanged(this, getTabText());
}

void TabEdhRecMain::retranslateUi()
{
    cardsPushButton->setText(tr("&Cards"));
    topCommandersPushButton->setText(tr("Top Commanders"));
    tagsPushButton->setText(tr("Tags"));
    searchBar->setPlaceholderText(tr("Search for a card ..."));
    searchPushButton->setText(tr("Search"));
    retryButton->setText(tr("Retry"));
}

void TabEdhRecMain::doSearch()
{
    CardInfoPtr searchedCard = CardDatabaseManager::query()->getCardInfo(searchBar->text());
    if (!searchedCard) {
        cancelRequest();
        showRequestError(tr("No matching card was found. Choose a card from the search suggestions."), false);
        return;
    }

    setCard(searchedCard, canBeCommander(searchedCard));
}

void TabEdhRecMain::setCard(CardInfoPtr _cardToQuery, bool isCommander)
{
    cardToQuery = _cardToQuery;

    if (!cardToQuery) {
        cancelRequest();
        showRequestError(tr("No card was selected."), false);
        return;
    }

    emit tabTextChanged(this, getTabText());

    QString formattedName = cardToQuery->getName().toLower().replace("æ", "ae");
    formattedName = formattedName.normalized(QString::NormalizationForm_KD)
                        .replace(QRegularExpression("\\s+"), "-")
                        .remove(QRegularExpression("[^a-z0-9-]"))
                        .replace(QRegularExpression("-+"), "-")
                        .remove(QRegularExpression("^-|-$"));

    QString url;
    if (isCommander) {
        url = QString("https://json.edhrec.com/pages/commanders/%1.json").arg(formattedName);
    } else {
        url = QString("https://json.edhrec.com/pages/cards/%1.json").arg(formattedName);
    }

    startRequest(QUrl(url), RequestKind::Details);
}

void TabEdhRecMain::actNavigatePage(QString url)
{
    const QUrl target(url);
    if (!target.isValid() || (!target.isRelative() && target.host() != "edhrec.com" &&
                              target.host() != "www.edhrec.com" && target.host() != "json.edhrec.com")) {
        cancelRequest();
        showRequestError(tr("This EDHREC link is not supported."), false);
        return;
    }

    QString path = target.path();
    if (path.startsWith("/pages/")) {
        path.remove(0, 6);
    }
    if (path.endsWith(".json")) {
        path.chop(5);
    }
    if (!path.startsWith('/')) {
        path.prepend('/');
    }
    while (path.endsWith('/')) {
        path.chop(1);
    }

    RequestKind kind;
    if (path.startsWith("/average-decks/")) {
        kind = RequestKind::AverageDeck;
    } else if (path == "/commanders/year" || path == "/commanders/month" || path == "/commanders/week") {
        kind = RequestKind::TopCommanders;
    } else if (path.startsWith("/top/")) {
        kind = RequestKind::TopCards;
    } else if (path == "/tags") {
        kind = RequestKind::TopTags;
    } else if (path.startsWith("/commanders/") || path.startsWith("/cards/") || path.startsWith("/tags/") ||
               path.startsWith("/combos/")) {
        kind = RequestKind::Details;
    } else {
        cancelRequest();
        showRequestError(tr("This EDHREC page is not supported."), false);
        return;
    }

    QUrl apiUrl("https://json.edhrec.com");
    apiUrl.setPath("/pages" + path + ".json");
    apiUrl.setQuery(target.query());
    if (kind != RequestKind::AverageDeck) {
        clearCardSelection();
    }
    startRequest(apiUrl, kind);
}

void TabEdhRecMain::getTopCards()
{
    clearCardSelection();
    startRequest(QUrl("https://json.edhrec.com/pages/top/year.json"), RequestKind::TopCards);
}

void TabEdhRecMain::getTopCommanders()
{
    clearCardSelection();
    startRequest(QUrl("https://json.edhrec.com/pages/commanders/year.json"), RequestKind::TopCommanders);
}

void TabEdhRecMain::getTopTags()
{
    clearCardSelection();
    startRequest(QUrl("https://json.edhrec.com/pages/tags.json"), RequestKind::TopTags);
}

void TabEdhRecMain::processApiJson(QNetworkReply *reply)
{
    if (reply != activeReply) {
        reply->deleteLater();
        return;
    }
    activeReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        showRequestError(tr("Unable to load EDHREC: %1").arg(reply->errorString()));
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        showRequestError(tr("Invalid response from EDHREC."));
        return;
    }

    const QJsonObject jsonObj = jsonDoc.object();
    if (requestKind != RequestKind::AverageDeck &&
        !jsonObj.value("container").toObject().value("json_dict").isObject()) {
        showRequestError(tr("Invalid page from EDHREC."));
        return;
    }

    requestStatusLabel->hide();
    switch (requestKind) {
        case RequestKind::TopCards:
            processTopCardsResponse(jsonObj);
            break;
        case RequestKind::TopCommanders:
            processTopCommandersResponse(jsonObj);
            break;
        case RequestKind::TopTags:
            processTopTagsResponse(jsonObj);
            break;
        case RequestKind::Details:
            processCommanderResponse(jsonObj, reply->url().toString());
            break;
        case RequestKind::AverageDeck:
            processAverageDeckResponse(jsonObj);
            break;
    }
}

void TabEdhRecMain::processTopCardsResponse(QJsonObject reply)
{
    EdhrecTopCardsApiResponse deckData;
    deckData.fromJson(reply);
    clearPage();
    auto *display = new EdhrecTopCardsApiResponseDisplayWidget(currentPageDisplay, deckData);
    currentPageLayout->addWidget(display);
}

void TabEdhRecMain::processTopTagsResponse(QJsonObject reply)
{
    EdhrecTopTagsApiResponse deckData;
    deckData.fromJson(reply);
    clearPage();
    auto *display = new EdhrecTopTagsApiResponseDisplayWidget(currentPageDisplay, deckData);
    currentPageLayout->addWidget(display);
}

void TabEdhRecMain::processTopCommandersResponse(QJsonObject reply)
{
    EdhrecTopCommandersApiResponse deckData;
    deckData.fromJson(reply);
    clearPage();
    auto *display = new EdhrecTopCommandersApiResponseDisplayWidget(currentPageDisplay, deckData);
    currentPageLayout->addWidget(display);
}

void TabEdhRecMain::processCommanderResponse(QJsonObject reply, QString responseUrl)
{
    EdhrecCommanderApiResponse deckData;
    deckData.fromJson(reply);
    clearPage();
    auto *display = new EdhrecCommanderApiResponseDisplayWidget(currentPageDisplay, deckData, responseUrl);
    currentPageLayout->addWidget(display);
}

void TabEdhRecMain::processAverageDeckResponse(QJsonObject reply)
{
    EdhrecAverageDeckApiResponse deckData;
    if (!deckData.fromJson(reply)) {
        showRequestError(tr("EDHREC returned an invalid or empty average deck."));
        return;
    }
    tabSupervisor->openDeckInNewTab({deckData.deck.deck, {}});
}

void TabEdhRecMain::prettyPrintJson(const QJsonValue &value, int indentLevel)
{
    const QString indent(indentLevel * 2, ' '); // Adjust spacing as needed for pretty printing

    if (value.isObject()) {
        QJsonObject obj = value.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            qDebug().noquote() << indent + it.key() + ":";
            prettyPrintJson(it.value(), indentLevel + 1);
        }
    } else if (value.isArray()) {
        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i) {
            qDebug().noquote() << indent + QString("[%1]:").arg(i);
            prettyPrintJson(array[i], indentLevel + 1);
        }
    } else if (value.isString()) {
        qDebug().noquote() << indent + "\"" + value.toString() + "\"";
    } else if (value.isDouble()) {
        qDebug().noquote() << indent + QString::number(value.toDouble());
    } else if (value.isBool()) {
        qDebug().noquote() << indent + (value.toBool() ? "true" : "false");
    } else if (value.isNull()) {
        qDebug().noquote() << indent + "null";
    }
}
