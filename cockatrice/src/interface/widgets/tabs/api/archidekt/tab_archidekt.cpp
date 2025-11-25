#include "tab_archidekt.h"

#include "../../../cards/additional_info/mana_symbol_widget.h"
#include "../../tab_supervisor.h"
#include "api_response/archidekt_deck_listing_api_response.h"
#include "display/archidekt_api_response_deck_display_widget.h"
#include "display/archidekt_api_response_deck_listings_display_widget.h"

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
#include <QResizeEvent>
#include <QUrlQuery>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>

TabArchidekt::TabArchidekt(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
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

    decksPushButton = new QPushButton(navigationContainer);
    connect(decksPushButton, &QPushButton::clicked, this, &TabArchidekt::getTopDecks);

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

    searchOptionsContainer = new QWidget(container);
    searchOptionsLayout = new QHBoxLayout(searchOptionsContainer);
    mainLayout->addWidget(searchOptionsContainer);

    // Colors

    QHBoxLayout *colorLayout = new QHBoxLayout();
    QString colorIdentity = "WUBRG"; // Optionally include "C" for colorless once we have a symbol for it

    for (const QChar &color : colorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(searchOptionsContainer, color, false, true);
        manaSymbol->setFixedWidth(25);
        colorLayout->addWidget(manaSymbol);

        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this, [=](QChar c, bool active) {
            if (active) {
                activeColors.insert(c);
            } else {
                activeColors.remove(c);
            }
        });
    }

    searchOptionsLayout->addLayout(colorLayout);

    logicalAndCheck = new QCheckBox("Require ALL colors", searchOptionsContainer);
    searchOptionsLayout->addWidget(logicalAndCheck);

    // Formats

    auto formatSettingsWidget = new SettingsButtonWidget(this);

    QStringList formatNames = {"Standard",      "Modern",         "Commander", "Legacy",     "Vintage",
                               "Pauper",        "Custom",         "Frontier",  "Future Std", "Penny Dreadful",
                               "1v1 Commander", "Dual Commander", "Brawl"};

    for (int i = 0; i < formatNames.size(); ++i) {
        QCheckBox *formatCheckBox = new QCheckBox(formatNames[i], searchOptionsContainer);
        formatChecks << formatCheckBox;
        formatSettingsWidget->addSettingsWidget(formatCheckBox);
    }

    searchOptionsLayout->addWidget(formatSettingsWidget);

    // Deck Name

    nameField = new QLineEdit(searchOptionsContainer);
    nameField->setPlaceholderText(tr("Deck name contains..."));
    searchOptionsLayout->addWidget(nameField);

    // Owner Name

    ownerField = new QLineEdit(searchOptionsContainer);
    ownerField->setPlaceholderText(tr("Owner name contains..."));
    searchOptionsLayout->addWidget(ownerField);

    // Contained cards

    cardsField = new QLineEdit(searchOptionsContainer);
    cardsField->setPlaceholderText("Cards (comma separated)");
    searchOptionsLayout->addWidget(cardsField);

    // Commanders

    commandersField = new QLineEdit(searchOptionsContainer);
    commandersField->setPlaceholderText("Commanders (comma separated)");
    searchOptionsLayout->addWidget(commandersField);

    // Page size

    pageSizeLabel = new QLabel(searchOptionsContainer);
    searchOptionsLayout->addWidget(pageSizeLabel);

    pageSizeSpin = new QSpinBox(searchOptionsContainer);
    pageSizeSpin->setRange(1, 200);
    pageSizeSpin->setValue(50);
    searchOptionsLayout->addWidget(pageSizeSpin);

    settingsButton = new SettingsButtonWidget(this);

    cardSizeSlider = new CardSizeWidget(this);

    settingsButton->addSettingsWidget(cardSizeSlider);

    navigationLayout->addWidget(decksPushButton);
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
    mainLayout->setStretch(1, 0);
    mainLayout->setStretch(2, 1); // currentPageDisplay expands as much as possible

    setCentralWidget(container);

    TabArchidekt::retranslateUi();

    getTopDecks();
}

void TabArchidekt::retranslateUi()
{
    decksPushButton->setText(tr("&Decks"));
    searchBar->setPlaceholderText(tr("Search for a card ..."));
    searchPushButton->setText(tr("Search"));
    pageSizeLabel->setText(tr("Max. Results:"));
}

QString TabArchidekt::buildSearchUrl()
{
    QUrlQuery query;

    // required
    query.addQueryItem("name", nameField->text());

    // colors
    QStringList selectedColors;
    for (QChar c : activeColors) {
        switch (c.unicode()) {
            case 'W':
                selectedColors << "White";
                break;
            case 'U':
                selectedColors << "Blue";
                break;
            case 'B':
                selectedColors << "Black";
                break;
            case 'G':
                selectedColors << "Green";
                break;
            case 'R':
                selectedColors << "Red";
                break;
            case 'C':
                selectedColors << "Colorless";
                break;
            default:
                break;
        }
    }

    if (!selectedColors.isEmpty())
        query.addQueryItem("colors", selectedColors.join(","));

    // logicalAnd
    if (logicalAndCheck->isChecked())
        query.addQueryItem("logicalAnd", "true");

    // owner
    if (!ownerField->text().isEmpty())
        query.addQueryItem("owner", ownerField->text());

    // cards
    if (!cardsField->text().isEmpty())
        query.addQueryItem("cards", cardsField->text());

    // commanders
    if (!commandersField->text().isEmpty())
        query.addQueryItem("commanders", commandersField->text());

    // formats
    QStringList formatIds;
    for (int i = 0; i < formatChecks.size(); ++i)
        if (formatChecks[i]->isChecked())
            formatIds << QString::number(i + 1);
    if (!formatIds.isEmpty())
        query.addQueryItem("formats", formatIds.join(","));

    // page size
    if (pageSizeSpin->value() != 50)
        query.addQueryItem("pageSize", QString::number(pageSizeSpin->value()));

    // build final URL
    QUrl url("https://archidekt.com/api/decks/v3/");
    url.setQuery(query);

    return url.toString();
}

void TabArchidekt::doSearch()
{
    QString url = buildSearchUrl();
    QNetworkRequest req{QUrl(url)};
    networkManager->get(req);
}

void TabArchidekt::actNavigatePage(QString url)
{
    QNetworkRequest request{QUrl(url)};

    networkManager->get(request);
}

void TabArchidekt::getTopDecks()
{
    QNetworkRequest request{QUrl(archidektApiLink)};

    networkManager->get(request);
}

void TabArchidekt::processApiJson(QNetworkReply *reply)
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
    if (responseUrl.startsWith("https://archidekt.com/api/decks/v3/")) {
        processTopDecksResponse(jsonObj);
    } else if (responseUrl.startsWith("https://archidekt.com/api/decks/")) {
        processDeckResponse(jsonObj);
    } else {
        prettyPrintJson(jsonObj, 4);
    }

    reply->deleteLater();
}

void TabArchidekt::processTopDecksResponse(QJsonObject reply)
{
    ArchidektDeckListingApiResponse deckData;
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

    auto display = new ArchidektApiResponseDeckListingsDisplayWidget(currentPageDisplay, deckData, cardSizeSlider);
    connect(display, &ArchidektApiResponseDeckListingsDisplayWidget::requestNavigation, this,
            &TabArchidekt::actNavigatePage);
    currentPageLayout->addWidget(display);

    mainLayout->addWidget(currentPageDisplay);

    // **Ensure layout stays correct**
    mainLayout->setStretch(0, 0); // Keep navigationContainer at the top
    mainLayout->setStretch(1, 0); // Keep searchOptionsContainer at the top
    mainLayout->setStretch(2, 1); // Make sure currentPageDisplay takes remaining space
}

void TabArchidekt::processDeckResponse(QJsonObject reply)
{
    ArchidektApiResponseDeck deckData;
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

    auto display = new ArchidektApiResponseDeckDisplayWidget(currentPageDisplay, deckData, cardSizeSlider);
    connect(display, &ArchidektApiResponseDeckDisplayWidget::requestNavigation, this, &TabArchidekt::actNavigatePage);
    connect(display, &ArchidektApiResponseDeckDisplayWidget::openInDeckEditor, tabSupervisor,
            &TabSupervisor::openDeckInNewTab);
    currentPageLayout->addWidget(display);

    mainLayout->addWidget(currentPageDisplay);

    // **Ensure layout stays correct**
    mainLayout->setStretch(0, 0); // Keep navigationContainer at the top
    mainLayout->setStretch(1, 0); // Keep searchOptionsContainer at the top
    mainLayout->setStretch(2, 1); // Make sure currentPageDisplay takes remaining space
}

void TabArchidekt::prettyPrintJson(const QJsonValue &value, int indentLevel)
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
