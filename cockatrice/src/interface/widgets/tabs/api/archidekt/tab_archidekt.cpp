#include "tab_archidekt.h"

#include "../../../../../client/settings/cache_settings.h"
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
#include <version_string.h>

TabArchidekt::TabArchidekt(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    networkManager = new QNetworkAccessManager(this);
    networkManager->setTransferTimeout(); // Use Qt's default timeout
    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(processApiJson(QNetworkReply *)));

    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true); // We only want it to fire once after inactivity
    searchDebounceTimer->setInterval(300);    // 300ms debounce

    connect(searchDebounceTimer, &QTimer::timeout, this, [this]() { doSearchImmediate(); });

    container = new QWidget(this);
    mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    container->setLayout(mainLayout);

    navigationContainer = new QWidget(container);
    navigationContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    navigationLayout = new QHBoxLayout(navigationContainer);
    navigationLayout->setSpacing(3);
    navigationContainer->setLayout(navigationLayout);

    // Sort by

    orderByCombo = new QComboBox(navigationContainer);
    orderByCombo->addItems({"name", "updatedAt", "createdAt", "viewCount", "size", "edhBracket"});
    orderByCombo->setCurrentText("updatedAt"); // Pre-select updatedAt

    // Asc/Desc toggle
    orderDirButton = new QPushButton(tr("Desc."), navigationContainer);
    orderDirButton->setCheckable(true); // checked = DESC, unchecked = ASC
    orderDirButton->setChecked(true);

    connect(orderByCombo, &QComboBox::currentTextChanged, this, &TabArchidekt::doSearch);
    connect(orderDirButton, &QPushButton::clicked, this, [this](bool checked) {
        orderDirButton->setText(checked ? tr("Desc.") : tr("Asc."));
        doSearch();
    });

    // Colors
    QHBoxLayout *colorLayout = new QHBoxLayout();
    QString colorIdentity = "WUBRG"; // Optionally include "C" for colorless once we have a symbol for it

    for (const QChar &color : colorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(navigationContainer, color, false, true);
        manaSymbol->setFixedWidth(25);
        colorLayout->addWidget(manaSymbol);

        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this, [this](QChar c, bool active) {
            if (active) {
                activeColors.insert(c);
            } else {
                activeColors.remove(c);
            }
            doSearch();
        });
    }

    logicalAndCheck = new QCheckBox("Require ALL colors", navigationContainer);

    // Formats

    formatLabel = new QLabel(this);

    formatSettingsWidget = new SettingsButtonWidget(this);

    QStringList formatNames = {"Standard",      "Modern",         "Commander", "Legacy",     "Vintage",
                               "Pauper",        "Custom",         "Frontier",  "Future Std", "Penny Dreadful",
                               "1v1 Commander", "Dual Commander", "Brawl"};

    for (int i = 0; i < formatNames.size(); ++i) {
        QCheckBox *formatCheckBox = new QCheckBox(formatNames[i], navigationContainer);
        connect(formatCheckBox, &QCheckBox::clicked, this, &TabArchidekt::doSearch);
        formatChecks << formatCheckBox;
        formatSettingsWidget->addSettingsWidget(formatCheckBox);
    }

    // EDH Bracket
    edhBracketCombo = new QComboBox(navigationContainer);
    edhBracketCombo->addItem(tr("Any Bracket"));
    edhBracketCombo->addItems({"1", "2", "3", "4", "5"});

    connect(edhBracketCombo, &QComboBox::currentTextChanged, this, &TabArchidekt::doSearch);

    // Search for Card Packages instead of Decks
    packagesCheck = new QCheckBox("Packages", navigationContainer);

    connect(packagesCheck, &QCheckBox::clicked, this, [this]() {
        bool disable = packagesCheck->isChecked();
        for (auto *cb : formatChecks)
            cb->setEnabled(!disable);
        commandersField->setEnabled(!disable);
        deckTagNameField->setEnabled(!disable);
        edhBracketCombo->setCurrentIndex(0);
        edhBracketCombo->setEnabled(!disable);
        doSearch();
    });

    // Deck Name
    nameField = new QLineEdit(navigationContainer);
    nameField->setPlaceholderText(tr("Deck name contains..."));

    // Owner Name
    ownerField = new QLineEdit(navigationContainer);
    ownerField->setPlaceholderText(tr("Owner name contains..."));

    // Contained cards
    cardsField = new QLineEdit(navigationContainer);
    cardsField->setPlaceholderText("Deck contains card...");

    // Commanders
    commandersField = new QLineEdit(navigationContainer);
    commandersField->setPlaceholderText("Deck has commander...");

    // DB supplemented card search
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

    cardsField->setCompleter(completer);
    commandersField->setCompleter(completer);

    connect(cardsField, &QLineEdit::textChanged, searchModel, &CardSearchModel::updateSearchResults);

    connect(cardsField, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty())
            completer->complete();
    });

    connect(commandersField, &QLineEdit::textChanged, searchModel, &CardSearchModel::updateSearchResults);

    connect(commandersField, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty())
            completer->complete();
    });

    // Tag Name
    deckTagNameField = new QLineEdit(navigationContainer);
    deckTagNameField->setPlaceholderText("Deck tag");

    connect(deckTagNameField, &QLineEdit::textChanged, this, &TabArchidekt::doSearch);

    // Search button
    searchPushButton = new QPushButton(navigationContainer);
    searchPushButton->setText("Search");

    connect(searchPushButton, &QPushButton::clicked, this, &TabArchidekt::doSearch);

    // Card Size settings
    settingsButton = new SettingsButtonWidget(this);
    cardSizeSlider = new CardSizeWidget(this, nullptr, SettingsCache::instance().getArchidektPreviewSize());
    connect(cardSizeSlider, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setArchidektPreviewCardSize);
    settingsButton->addSettingsWidget(cardSizeSlider);

    // Min deck size
    minDeckSizeLabel = new QLabel(navigationContainer);

    minDeckSizeSpin = new QSpinBox(navigationContainer);
    minDeckSizeSpin->setSpecialValueText(tr("Disabled"));
    minDeckSizeSpin->setRange(0, 200);
    minDeckSizeSpin->setValue(0);

    // Size logic
    minDeckSizeLogicCombo = new QComboBox(navigationContainer);
    minDeckSizeLogicCombo->addItems({"Exact", "≥", "≤"}); // Exact = unset, ≥ = GTE, ≤ = LTE
    minDeckSizeLogicCombo->setCurrentIndex(1);            // default GTE

    connect(minDeckSizeSpin, qOverload<int>(&QSpinBox::valueChanged), this, &TabArchidekt::doSearch);
    connect(minDeckSizeLogicCombo, &QComboBox::currentTextChanged, this, &TabArchidekt::doSearch);

    // Page number
    pageLabel = new QLabel(navigationContainer);

    pageSpin = new QSpinBox(navigationContainer);
    pageSpin->setRange(1, 9999);
    pageSpin->setValue(1);

    connect(pageSpin, qOverload<int>(&QSpinBox::valueChanged), this, &TabArchidekt::doSearch);

    // Page display
    currentPageDisplay = new QWidget(container);
    currentPageLayout = new QVBoxLayout(currentPageDisplay);
    currentPageLayout->setContentsMargins(0, 0, 0, 0);
    currentPageDisplay->setLayout(currentPageLayout);

    // Layout composition

    // Sort section
    navigationLayout->addWidget(orderByCombo);
    navigationLayout->addWidget(orderDirButton);

    // Colors section
    navigationLayout->addLayout(colorLayout);
    navigationLayout->addWidget(logicalAndCheck);

    // Formats section
    navigationLayout->addWidget(formatSettingsWidget);
    navigationLayout->addWidget(formatLabel);

    // EDH Bracket
    navigationLayout->addWidget(edhBracketCombo);

    // Packages toggle
    navigationLayout->addWidget(packagesCheck);

    // Deck name
    navigationLayout->addWidget(nameField);

    // Owner name
    navigationLayout->addWidget(ownerField);

    // Contained cards
    navigationLayout->addWidget(cardsField);

    // Commanders
    navigationLayout->addWidget(commandersField);

    // Deck tag
    navigationLayout->addWidget(deckTagNameField);

    // Search button
    navigationLayout->addWidget(searchPushButton);

    // Card size settings
    navigationLayout->addWidget(settingsButton);

    // Min. # of cards in deck
    navigationLayout->addWidget(minDeckSizeLabel);
    navigationLayout->addWidget(minDeckSizeSpin);
    navigationLayout->addWidget(minDeckSizeLogicCombo);

    // Page number
    navigationLayout->addWidget(pageLabel);
    navigationLayout->addWidget(pageSpin);

    mainLayout->addWidget(navigationContainer);
    mainLayout->addWidget(currentPageDisplay);

    // Ensure navigation stays at the top and currentPageDisplay takes remaining space
    mainLayout->setStretch(0, 0); // navigationContainer gets minimum space
    mainLayout->setStretch(1, 1); // currentPageDisplay expands as much as possible

    setCentralWidget(container);

    TabArchidekt::retranslateUi();

    getTopDecks();
}

void TabArchidekt::retranslateUi()
{
    searchPushButton->setText(tr("Search"));
    formatLabel->setText(tr("Formats"));
    minDeckSizeLabel->setText(tr("Min. # of Cards:"));
    pageLabel->setText(tr("Page:"));
}

QString TabArchidekt::buildSearchUrl()
{
    QUrlQuery query;

    // orderBy (field + direction)
    {
        QString field = orderByCombo->currentText();
        if (!field.isEmpty()) {
            bool desc = orderDirButton->isChecked();
            QString final = desc ? "-" + field : field;
            query.addQueryItem("orderBy", final);
        }
    }

    // Colors
    QStringList selectedColors;
    for (QChar c : activeColors) {
        selectedColors.append(c);
    }
    if (!selectedColors.isEmpty()) {
        query.addQueryItem("colors", selectedColors.join(","));
    }

    // logicalAnd
    if (logicalAndCheck->isChecked()) {
        query.addQueryItem("logicalAnd", "true");
    }

    // Formats
    if (!packagesCheck->isChecked()) {
        QStringList formatIds;
        for (int i = 0; i < formatChecks.size(); ++i)
            if (formatChecks[i]->isChecked()) {
                formatIds << QString::number(i + 1);
            }

        if (!formatIds.isEmpty()) {
            query.addQueryItem("deckFormat", formatIds.join(","));
        }
    }

    // edhBracket
    if (!packagesCheck->isChecked()) {
        if (!edhBracketCombo->currentText().isEmpty()) {
            if (edhBracketCombo->currentText() != tr("Any Bracket")) {
                query.addQueryItem("edhBracket", edhBracketCombo->currentText());
            }
        }
    }

    // Search for card packages instead of decks
    if (packagesCheck->isChecked()) {
        query.addQueryItem("packages", "true");
    }

    // Name
    if (!nameField->text().isEmpty()) {
        query.addQueryItem("name", nameField->text());
    }

    // owner
    if (!ownerField->text().isEmpty()) {
        query.addQueryItem("ownerUsername", ownerField->text());
    }

    // cards
    if (!cardsField->text().isEmpty()) {
        query.addQueryItem("cardName", cardsField->text());
    }

    // Commander Name
    if (!packagesCheck->isChecked()) {
        if (!commandersField->text().isEmpty()) {
            query.addQueryItem("commanderName", commandersField->text());
        }
    }

    // deckTagName
    if (!packagesCheck->isChecked()) {
        if (!deckTagNameField->text().isEmpty()) {
            query.addQueryItem("deckTagName", deckTagNameField->text());
        }
    }

    // page number
    if (pageSpin->value() <= 1) {
        query.addQueryItem("page", QString::number(pageSpin->value()));
    }

    // Min deck size
    if (minDeckSizeSpin->value() != 0) {
        query.addQueryItem("size", QString::number(minDeckSizeSpin->value()));

        QString logic = "GTE"; // default
        QString selected = minDeckSizeLogicCombo->currentText();
        if (selected == "≥")
            logic = "GTE";
        else if (selected == "≤")
            logic = "LTE";
        else
            logic = ""; // Exact = unset

        if (!logic.isEmpty()) {
            query.addQueryItem("sizeLogic", logic);
        }
    }

    // build final URL
    QUrl url("https://archidekt.com/api/decks/v3/");
    url.setQuery(query);

    return url.toString();
}

void TabArchidekt::doSearch()
{
    searchDebounceTimer->start();
}

void TabArchidekt::doSearchImmediate()
{
    QString url = buildSearchUrl();
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    networkManager->get(req);
}

void TabArchidekt::actNavigatePage(QString url)
{
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    networkManager->get(request);
}

void TabArchidekt::getTopDecks()
{
    QNetworkRequest request{QUrl(buildSearchUrl())};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
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
    mainLayout->setStretch(1, 1); // Make sure currentPageDisplay takes remaining space
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
    mainLayout->setStretch(1, 1); // Make sure currentPageDisplay takes remaining space
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
