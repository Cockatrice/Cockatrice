#include "tab_edhrec_main.h"

#include "../../../../../client/settings/cache_settings.h"
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

#include <QCompleter>
#include <QDebug>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QRegularExpression>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <version_string.h>

static bool canBeCommander(const CardInfoPtr &cardInfo)
{
    return ((cardInfo->getCardType().contains("Legendary", Qt::CaseInsensitive) &&
             cardInfo->getCardType().contains("Creature", Qt::CaseInsensitive))) ||
           cardInfo->getText().contains("can be your commander", Qt::CaseInsensitive);
}

TabEdhRecMain::TabEdhRecMain(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    networkManager = new QNetworkAccessManager(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    networkManager->setTransferTimeout(); // Use Qt's default timeout
#endif

    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(processApiJson(QNetworkReply *)));

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
    auto *searchModel = new CardSearchModel(displayModel, this);

    auto *proxyModel = new CardCompleterProxyModel(this);
    proxyModel->setSourceModel(searchModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(Qt::DisplayRole);

    auto *completer = new QCompleter(proxyModel, this);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setMaxVisibleItems(10);
    searchBar->setCompleter(completer);

    // Update suggestions dynamically
    connect(searchBar, &QLineEdit::textChanged, searchModel, &CardSearchModel::updateSearchResults);
    connect(searchBar, &QLineEdit::textChanged, this, [=](const QString &text) {
        // Ensure substring matching
        QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));

        if (!text.isEmpty()) {
            completer->complete(); // Force the dropdown to appear
        }
    });

    searchPushButton = new QPushButton(navigationContainer);
    connect(searchPushButton, &QPushButton::clicked, this, [=, this]() { doSearch(); });

    settingsButton = new SettingsButtonWidget(this);

    cardSizeSlider = new CardSizeWidget(this, nullptr, SettingsCache::instance().getEDHRecCardSize());
    connect(cardSizeSlider, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setEDHRecCardSize);

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
    mainLayout->addWidget(currentPageDisplay);

    // Ensure navigation stays at the top and currentPageDisplay takes remaining space
    mainLayout->setStretch(0, 0); // navigationContainer gets minimum space
    mainLayout->setStretch(1, 1); // currentPageDisplay expands as much as possible

    setCentralWidget(container);

    TabEdhRecMain::retranslateUi();

    getTopCards();
}

void TabEdhRecMain::retranslateUi()
{
    cardsPushButton->setText(tr("&Cards"));
    topCommandersPushButton->setText(tr("Top Commanders"));
    tagsPushButton->setText(tr("Tags"));
    searchBar->setPlaceholderText(tr("Search for a card ..."));
    searchPushButton->setText(tr("Search"));
}

void TabEdhRecMain::doSearch()
{
    CardInfoPtr searchedCard = CardDatabaseManager::query()->getCardInfo(searchBar->text());
    if (!searchedCard) {
        return;
    }

    setCard(searchedCard, canBeCommander(searchedCard));
}

void TabEdhRecMain::setCard(CardInfoPtr _cardToQuery, bool isCommander)
{
    cardToQuery = _cardToQuery;

    if (!cardToQuery) {
        qDebug() << "Invalid card information provided.";
        return;
    }

    QString cardName = cardToQuery->getName();
    QString formattedName = cardName.toLower().replace(" ", "-").remove(QRegularExpression("[^a-z0-9\\-]"));

    QString url;
    if (isCommander) {
        url = QString("https://json.edhrec.com/pages/commanders/%1.json").arg(formattedName);
    } else {
        url = QString("https://json.edhrec.com/pages/cards/%1.json").arg(formattedName);
    }

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    networkManager->get(request);
}

void TabEdhRecMain::actNavigatePage(QString url)
{
    QNetworkRequest request{QUrl("https://json.edhrec.com/pages" + url + ".json")};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    networkManager->get(request);
}

void TabEdhRecMain::getTopCards()
{
    QNetworkRequest request{QUrl("https://json.edhrec.com/pages/top/year.json")};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    networkManager->get(request);
}

void TabEdhRecMain::getTopCommanders()
{
    QNetworkRequest request{QUrl("https://json.edhrec.com/pages/commanders/year.json")};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));

    networkManager->get(request);
}

void TabEdhRecMain::getTopTags()
{
    QNetworkRequest request{QUrl("https://json.edhrec.com/pages/tags.json")};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    networkManager->get(request);
}

void TabEdhRecMain::processApiJson(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error occurred:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (!jsonDoc.isObject()) {
        qDebug() << "Invalid JSON response received.";
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // Get the actual URL from the reply
    QString responseUrl = reply->url().toString();

    // Check if the response URL matches a commander request
    if (responseUrl.startsWith("https://json.edhrec.com/pages/commanders/year.json")) {
        processTopCommandersResponse(jsonObj);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/commanders/")) {
        qInfo() << "Received top kek";
        processCommanderResponse(jsonObj, responseUrl);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/cards/")) {
        processCommanderResponse(jsonObj);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/tags/")) {
        processCommanderResponse(jsonObj);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/tags.json")) {
        processTopTagsResponse(jsonObj);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/top/year.json")) {
        processTopCardsResponse(jsonObj);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/combos/")) {
        qInfo() << "Received combos";
        processCommanderResponse(jsonObj);
    } else if (responseUrl.startsWith("https://json.edhrec.com/pages/average-decks/")) {
        processAverageDeckResponse(jsonObj);
    } else {
        prettyPrintJson(jsonObj, 4);
    }

    reply->deleteLater();
}

void TabEdhRecMain::processTopCardsResponse(QJsonObject reply)
{
    EdhrecTopCardsApiResponse deckData;
    deckData.fromJson(reply);

    // **Remove previous page display to prevent stacking**
    if (currentPageDisplay) {
        mainLayout->removeWidget(currentPageDisplay);
        delete currentPageDisplay;
        currentPageDisplay = nullptr;
    }

    // **Create new currentPageDisplay**
    currentPageDisplay = new QWidget(container);
    currentPageLayout = new QVBoxLayout(currentPageDisplay);
    currentPageDisplay->setLayout(currentPageLayout);

    auto display = new EdhrecTopCardsApiResponseDisplayWidget(currentPageDisplay, deckData);
    currentPageLayout->addWidget(display);

    mainLayout->addWidget(currentPageDisplay);

    // **Ensure layout stays correct**
    mainLayout->setStretch(0, 0); // Keep navigationContainer at the top
    mainLayout->setStretch(1, 1); // Make sure currentPageDisplay takes remaining space
}

void TabEdhRecMain::processTopTagsResponse(QJsonObject reply)
{
    EdhrecTopTagsApiResponse deckData;
    deckData.fromJson(reply);

    // **Remove previous page display to prevent stacking**
    if (currentPageDisplay) {
        mainLayout->removeWidget(currentPageDisplay);
        delete currentPageDisplay;
        currentPageDisplay = nullptr;
    }

    // **Create new currentPageDisplay**
    currentPageDisplay = new QWidget(container);
    currentPageLayout = new QVBoxLayout(currentPageDisplay);
    currentPageDisplay->setLayout(currentPageLayout);

    auto display = new EdhrecTopTagsApiResponseDisplayWidget(currentPageDisplay, deckData);
    currentPageLayout->addWidget(display);

    mainLayout->addWidget(currentPageDisplay);

    // **Ensure layout stays correct**
    mainLayout->setStretch(0, 0); // Keep navigationContainer at the top
    mainLayout->setStretch(1, 1); // Make sure currentPageDisplay takes remaining space
}

void TabEdhRecMain::processTopCommandersResponse(QJsonObject reply)
{
    EdhrecTopCommandersApiResponse deckData;
    deckData.fromJson(reply);

    // **Remove previous page display to prevent stacking**
    if (currentPageDisplay) {
        mainLayout->removeWidget(currentPageDisplay);
        delete currentPageDisplay;
        currentPageDisplay = nullptr;
    }

    // **Create new currentPageDisplay**
    currentPageDisplay = new QWidget(container);
    currentPageLayout = new QVBoxLayout(currentPageDisplay);
    currentPageDisplay->setLayout(currentPageLayout);

    auto display = new EdhrecTopCommandersApiResponseDisplayWidget(currentPageDisplay, deckData);
    currentPageLayout->addWidget(display);

    mainLayout->addWidget(currentPageDisplay);

    // **Ensure layout stays correct**
    mainLayout->setStretch(0, 0); // Keep navigationContainer at the top
    mainLayout->setStretch(1, 1); // Make sure currentPageDisplay takes remaining space
}

void TabEdhRecMain::processCommanderResponse(QJsonObject reply, QString responseUrl)
{
    EdhrecCommanderApiResponse deckData;
    deckData.fromJson(reply);

    // **Remove previous page display to prevent stacking**
    if (currentPageDisplay) {
        mainLayout->removeWidget(currentPageDisplay);
        delete currentPageDisplay;
        currentPageDisplay = nullptr;
    }

    // **Create new currentPageDisplay**
    currentPageDisplay = new QWidget(container);
    currentPageLayout = new QVBoxLayout(currentPageDisplay);
    currentPageDisplay->setLayout(currentPageLayout);

    auto display = new EdhrecCommanderApiResponseDisplayWidget(currentPageDisplay, deckData, responseUrl);
    currentPageLayout->addWidget(display);

    mainLayout->addWidget(currentPageDisplay);

    // **Ensure layout stays correct**
    mainLayout->setStretch(0, 0); // Keep navigationContainer at the top
    mainLayout->setStretch(1, 1); // Make sure currentPageDisplay takes remaining space
}

void TabEdhRecMain::processAverageDeckResponse(QJsonObject reply)
{
    EdhrecAverageDeckApiResponse deckData;
    deckData.fromJson(reply);
    tabSupervisor->openDeckInNewTab(deckData.deck.deckLoader);
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
