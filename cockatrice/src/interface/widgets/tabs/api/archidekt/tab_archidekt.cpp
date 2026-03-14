#include "tab_archidekt.h"

#include "../../../../../client/settings/cache_settings.h"
#include "../../../cards/additional_info/mana_symbol_widget.h"
#include "../../tab_supervisor.h"
#include "api_response/archidekt_deck_listing_api_response.h"
#include "display/archidekt_api_response_deck_display_widget.h"
#include "display/archidekt_api_response_deck_listings_display_widget.h"

#include <QCompleter>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QUrlQuery>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <version_string.h>

TabArchidekt::TabArchidekt(TabSupervisor *_tabSupervisor)
    : Tab(_tabSupervisor), currentPage(1), isLoadingMore(false), isListMode(true)
{
    // Initialize network
    networkManager = new QNetworkAccessManager(this);
    networkManager->setTransferTimeout();
    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    connect(networkManager, &QNetworkAccessManager::finished, this, &TabArchidekt::processApiJson);

    // Initialize debounce timer
    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true);
    searchDebounceTimer->setInterval(300);
    connect(searchDebounceTimer, &QTimer::timeout, this, &TabArchidekt::doSearchImmediate);

    initializeUi();
    setupFilterWidgets();
    connectSignals();
    retranslateUi();

    getTopDecks();
}

void TabArchidekt::initializeUi()
{
    // Main container
    container = new QWidget(this);
    mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Primary toolbar (most important filters)
    primaryToolbar = new QWidget(container);
    primaryToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    primaryToolbarLayout = new QHBoxLayout(primaryToolbar);
    primaryToolbarLayout->setContentsMargins(6, 6, 6, 6);
    primaryToolbarLayout->setSpacing(6);

    // Sort controls
    sortByLabel = new QLabel(primaryToolbar);
    orderByCombo = new QComboBox(primaryToolbar);
    orderByCombo->addItems({"name", "updatedAt", "createdAt", "viewCount", "size", "edhBracket"});
    orderByCombo->setCurrentText("updatedAt");
    orderByCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    orderDirButton = new QPushButton(tr("Desc."), primaryToolbar);
    orderDirButton->setCheckable(true);
    orderDirButton->setChecked(true);
    orderDirButton->setFixedWidth(60);

    // Color filter (inline)
    QWidget *colorWidget = new QWidget(primaryToolbar);
    QHBoxLayout *colorLayout = new QHBoxLayout(colorWidget);
    colorLayout->setContentsMargins(0, 0, 0, 0);
    colorLayout->setSpacing(2);

    QString colorIdentity = "WUBRG";
    for (const QChar &color : colorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(colorWidget, color, false, true);
        manaSymbol->setFixedSize(28, 28);
        colorSymbols.append(manaSymbol);
        colorLayout->addWidget(manaSymbol);

        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this, [this](QChar c, bool active) {
            if (active)
                activeColors.insert(c);
            else
                activeColors.remove(c);
            doSearch();
        });
    }

    logicalAndCheck = new QCheckBox(tr("AND"), primaryToolbar);
    logicalAndCheck->setToolTip(tr("Require ALL selected colors"));

    // Common search fields
    nameField = new QLineEdit(primaryToolbar);
    nameField->setPlaceholderText(tr("Deck name..."));
    nameField->setMinimumWidth(150);

    ownerField = new QLineEdit(primaryToolbar);
    ownerField->setPlaceholderText(tr("Owner..."));
    ownerField->setMinimumWidth(120);

    // Filter by label
    filterByLabel = new QLabel(primaryToolbar);

    // Package toggle
    packagesCheck = new QCheckBox(tr("Packages"), primaryToolbar);

    // Search button
    searchButton = new QPushButton(tr("Search"), primaryToolbar);
    searchButton->setDefault(true);

    // Advanced filters toggle button
    advancedFiltersButton = new QPushButton(tr("Advanced Filters"), primaryToolbar);
    advancedFiltersButton->setCheckable(true);
    advancedFiltersButton->setChecked(false);

    // Settings
    settingsButton = new SettingsButtonWidget(primaryToolbar);
    cardSizeSlider = new CardSizeWidget(primaryToolbar, nullptr, SettingsCache::instance().getArchidektPreviewSize());
    settingsButton->addSettingsWidget(cardSizeSlider);

    // Assemble primary toolbar
    primaryToolbarLayout->addWidget(sortByLabel);
    primaryToolbarLayout->addWidget(orderByCombo);
    primaryToolbarLayout->addWidget(orderDirButton);

    // Add separator/spacing
    primaryToolbarLayout->addSpacing(12);

    primaryToolbarLayout->addWidget(filterByLabel);
    primaryToolbarLayout->addWidget(colorWidget);
    primaryToolbarLayout->addWidget(logicalAndCheck);
    primaryToolbarLayout->addWidget(nameField, 1);
    primaryToolbarLayout->addWidget(ownerField, 1);
    primaryToolbarLayout->addWidget(packagesCheck);
    primaryToolbarLayout->addWidget(searchButton, 1);
    primaryToolbarLayout->addWidget(advancedFiltersButton);
    primaryToolbarLayout->addWidget(settingsButton);

    // Secondary toolbar (advanced filters - initially hidden)
    secondaryToolbar = new QWidget(container);
    secondaryToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    secondaryToolbar->setVisible(false); // Start hidden
    secondaryToolbarLayout = new QHBoxLayout(secondaryToolbar);
    secondaryToolbarLayout->setContentsMargins(6, 3, 6, 6);
    secondaryToolbarLayout->setSpacing(6);

    // Scrollable results area
    scrollArea = new QScrollArea(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    resultsContainer = new QWidget();
    resultsLayout = new QVBoxLayout(resultsContainer);
    resultsLayout->setContentsMargins(0, 0, 0, 0);
    resultsLayout->setSpacing(0);

    scrollArea->setWidget(resultsContainer);

    scrollArea->viewport()->installEventFilter(this);

    mainLayout->addWidget(primaryToolbar);
    mainLayout->addWidget(secondaryToolbar);
    mainLayout->addWidget(scrollArea);

    setCentralWidget(container);
}

bool TabArchidekt::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == scrollArea->viewport() && event->type() == QEvent::Wheel) {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);

        if (wheelEvent->angleDelta().y() < 0 && !isLoadingMore && isListMode) {
            loadNextPage();
            wheelEvent->accept();
            return false; // allow scrolling
        }
    }

    // Always pass the event to the parent to handle normal scrolling
    return QWidget::eventFilter(obj, event);
}

void TabArchidekt::setupFilterWidgets()
{
    // Advanced filters (in secondary toolbar)

    // EDH Bracket
    auto *bracketLabel = new QLabel(tr("Bracket:"), secondaryToolbar);
    edhBracketCombo = new QComboBox(secondaryToolbar);
    edhBracketCombo->addItem(tr("Any"));
    edhBracketCombo->addItems({"1", "2", "3", "4", "5"});
    edhBracketCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // Format filter (collapsible)
    formatButton = new SettingsButtonWidget(secondaryToolbar);
    formatButton->setButtonText(tr("Formats"));
    formatButton->setButtonIcon(QPixmap("theme:icons/scale_balanced"));

    QWidget *formatContainer = new QWidget(secondaryToolbar);
    QGridLayout *formatLayout = new QGridLayout(formatContainer);
    formatLayout->setContentsMargins(4, 4, 4, 4);

    QStringList formatNames = {"Standard",      "Modern",         "Commander", "Legacy",     "Vintage",
                               "Pauper",        "Custom",         "Frontier",  "Future Std", "Penny Dreadful",
                               "1v1 Commander", "Dual Commander", "Brawl"};

    int row = 0, col = 0;
    for (const QString &formatName : formatNames) {
        auto *formatCheckBox = new QCheckBox(formatName, formatContainer);
        formatChecks << formatCheckBox;
        formatLayout->addWidget(formatCheckBox, row, col);
        connect(formatCheckBox, &QCheckBox::clicked, this, &TabArchidekt::doSearch);

        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }

    formatButton->addSettingsWidget(formatContainer);

    cardsField = new QLineEdit(secondaryToolbar);
    cardsField->setPlaceholderText(tr("Contains card..."));
    cardsField->setMinimumWidth(140);

    commandersField = new QLineEdit(secondaryToolbar);
    commandersField->setPlaceholderText(tr("Commander..."));
    commandersField->setMinimumWidth(140);

    deckTagNameField = new QLineEdit(secondaryToolbar);
    deckTagNameField->setPlaceholderText(tr("Tag..."));
    deckTagNameField->setMinimumWidth(100);

    // Deck size filter (collapsible)
    deckSizeButton = new SettingsButtonWidget(secondaryToolbar);
    deckSizeButton->setButtonText(tr("Deck Size"));

    QWidget *sizeContainer = new QWidget(secondaryToolbar);
    QHBoxLayout *sizeLayout = new QHBoxLayout(sizeContainer);
    sizeLayout->setContentsMargins(4, 4, 4, 4);

    minDeckSizeSpin = new QSpinBox(sizeContainer);
    minDeckSizeSpin->setSpecialValueText(tr("Any"));
    minDeckSizeSpin->setRange(0, 200);
    minDeckSizeSpin->setValue(0);

    minDeckSizeLogicCombo = new QComboBox(sizeContainer);
    minDeckSizeLogicCombo->addItems({"Exact", "≥", "≤"});
    minDeckSizeLogicCombo->setCurrentIndex(1);

    sizeLayout->addWidget(new QLabel(tr("Cards:"), sizeContainer));
    sizeLayout->addWidget(minDeckSizeSpin);
    sizeLayout->addWidget(minDeckSizeLogicCombo);

    deckSizeButton->addSettingsWidget(sizeContainer);

    // Setup card name autocomplete
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

    // Keep autocomplete working for both fields
    connect(cardsField, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->updateSearchResults(text);
        QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty())
            completer->complete();
    });

    connect(commandersField, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->updateSearchResults(text);
        QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty())
            completer->complete();
    });

    // Assemble secondary toolbar
    secondaryToolbarLayout->addWidget(bracketLabel);
    secondaryToolbarLayout->addWidget(edhBracketCombo);
    secondaryToolbarLayout->addWidget(formatButton);
    secondaryToolbarLayout->addWidget(cardsField);
    secondaryToolbarLayout->addWidget(commandersField);
    secondaryToolbarLayout->addWidget(deckTagNameField);
    secondaryToolbarLayout->addWidget(deckSizeButton);
    secondaryToolbarLayout->addStretch();
}

void TabArchidekt::connectSignals()
{
    // Advanced filters toggle
    connect(advancedFiltersButton, &QPushButton::clicked, this,
            [this](bool checked) { secondaryToolbar->setVisible(checked); });

    // These trigger immediate search (no debounce needed)
    connect(orderByCombo, &QComboBox::currentTextChanged, this, &TabArchidekt::doSearch);
    connect(orderDirButton, &QPushButton::clicked, [this](bool checked) {
        orderDirButton->setText(checked ? tr("Desc.") : tr("Asc."));
        doSearch();
    });

    connect(cardSizeSlider, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setArchidektPreviewCardSize);

    // Search button triggers immediate search
    connect(searchButton, &QPushButton::clicked, this, &TabArchidekt::doSearchImmediate);

    // These trigger search (but not text fields)
    connect(logicalAndCheck, &QCheckBox::clicked, this, &TabArchidekt::doSearch);
    connect(packagesCheck, &QCheckBox::clicked, [this]() {
        updatePackageModeState(packagesCheck->isChecked());
        doSearch();
    });

    // Format filters trigger search
    connect(edhBracketCombo, &QComboBox::currentTextChanged, this, &TabArchidekt::doSearch);
    connect(minDeckSizeSpin, qOverload<int>(&QSpinBox::valueChanged), this, &TabArchidekt::doSearch);
    connect(minDeckSizeLogicCombo, &QComboBox::currentTextChanged, this, &TabArchidekt::doSearch);

    // Allow Enter key in text fields to trigger search
    connect(nameField, &QLineEdit::returnPressed, this, &TabArchidekt::doSearchImmediate);
    connect(ownerField, &QLineEdit::returnPressed, this, &TabArchidekt::doSearchImmediate);
    connect(cardsField, &QLineEdit::returnPressed, this, &TabArchidekt::doSearchImmediate);
    connect(commandersField, &QLineEdit::returnPressed, this, &TabArchidekt::doSearchImmediate);
    connect(deckTagNameField, &QLineEdit::returnPressed, this, &TabArchidekt::doSearchImmediate);

    // Format checkboxes trigger search
    for (auto *formatCheck : formatChecks) {
        connect(formatCheck, &QCheckBox::clicked, this, &TabArchidekt::doSearch);
    }
}

void TabArchidekt::updatePackageModeState(bool isPackageMode)
{
    // Disable format-specific and commander-specific filters in package mode
    for (auto *cb : formatChecks) {
        cb->setEnabled(!isPackageMode);
    }

    edhBracketCombo->setEnabled(!isPackageMode);
    if (isPackageMode) {
        edhBracketCombo->setCurrentIndex(0);
    }

    commandersField->setEnabled(!isPackageMode);
    deckTagNameField->setEnabled(!isPackageMode);
}

void TabArchidekt::retranslateUi()
{
    sortByLabel->setText(tr("Sort by:"));
    orderDirButton->setText(orderDirButton->isChecked() ? tr("Desc.") : tr("Asc."));

    filterByLabel->setText(tr("Filter by:"));

    logicalAndCheck->setText(tr("AND"));
    logicalAndCheck->setToolTip(tr("Require ALL selected colors"));

    nameField->setPlaceholderText(tr("Deck name..."));
    ownerField->setPlaceholderText(tr("Owner..."));
    packagesCheck->setText(tr("Packages"));
    advancedFiltersButton->setText(tr("Advanced Filters"));

    cardsField->setPlaceholderText(tr("Contains card..."));
    commandersField->setPlaceholderText(tr("Commander..."));
    deckTagNameField->setPlaceholderText(tr("Tag..."));

    formatButton->setButtonText(tr("Formats"));
    deckSizeButton->setButtonText(tr("Deck Size"));

    searchButton->setText(tr("Search"));

    settingsButton->setToolTip(tr("Display Settings"));
}

QString TabArchidekt::buildSearchUrl()
{
    QUrlQuery query;

    // orderBy (field + direction)
    QString field = orderByCombo->currentText();
    if (!field.isEmpty()) {
        bool desc = orderDirButton->isChecked();
        QString final = desc ? "-" + field : field;
        query.addQueryItem("orderBy", final);
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

    // Formats (disabled in package mode)
    if (!packagesCheck->isChecked()) {
        QStringList formatIds;
        for (int i = 0; i < formatChecks.size(); ++i) {
            if (formatChecks[i]->isChecked()) {
                formatIds << QString::number(i + 1);
            }
        }

        if (!formatIds.isEmpty()) {
            query.addQueryItem("deckFormat", formatIds.join(","));
        }

        // edhBracket
        if (edhBracketCombo->currentIndex() > 0) {
            query.addQueryItem("edhBracket", edhBracketCombo->currentText());
        }
    }

    // Package mode
    if (packagesCheck->isChecked()) {
        query.addQueryItem("packages", "true");
    }

    // Name
    if (!nameField->text().isEmpty()) {
        query.addQueryItem("name", nameField->text());
    }

    // Owner
    if (!ownerField->text().isEmpty()) {
        query.addQueryItem("ownerUsername", ownerField->text());
    }

    // Cards
    if (!cardsField->text().isEmpty()) {
        query.addQueryItem("cards", cardsField->text());
    }

    // Commander (disabled in package mode)
    if (!packagesCheck->isChecked() && !commandersField->text().isEmpty()) {
        query.addQueryItem("commanderName", commandersField->text());
    }

    // Deck tag (disabled in package mode)
    if (!packagesCheck->isChecked() && !deckTagNameField->text().isEmpty()) {
        query.addQueryItem("deckTagName", deckTagNameField->text());
    }

    // Page number (for infinite scroll)
    query.addQueryItem("page", QString::number(currentPage));

    // Min deck size
    if (minDeckSizeSpin->value() != 0) {
        query.addQueryItem("size", QString::number(minDeckSizeSpin->value()));

        QString logic = "GTE";
        QString selected = minDeckSizeLogicCombo->currentText();
        if (selected == "≥")
            logic = "GTE";
        else if (selected == "≤")
            logic = "LTE";
        else
            logic = "";

        if (!logic.isEmpty()) {
            query.addQueryItem("sizeLogic", logic);
        }
    }

    QUrl url("https://archidekt.com/api/decks/v3/");
    url.setQuery(query);

    return url.toString();
}

void TabArchidekt::doSearch()
{
    // Reset to first page on new search
    currentPage = 1;
    // We're searching, so we'll be in list mode
    isListMode = true;
    // Don't debounce - only called by explicit user actions now
    doSearchImmediate();
}

void TabArchidekt::doSearchImmediate()
{
    QString url = buildSearchUrl();
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    networkManager->get(req);
}

void TabArchidekt::loadNextPage()
{
    if (isLoadingMore) {
        return;
    }

    isLoadingMore = true;
    currentPage++;

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
    currentPage = 1;
    QNetworkRequest request{QUrl(buildSearchUrl())};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    networkManager->get(request);
}

void TabArchidekt::processApiJson(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        isLoadingMore = false;
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (!jsonDoc.isObject()) {
        isLoadingMore = false;
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QString responseUrl = reply->url().toString();

    if (responseUrl.startsWith("https://archidekt.com/api/decks/v3/")) {
        processTopDecksResponse(jsonObj);
    } else if (responseUrl.startsWith("https://archidekt.com/api/decks/")) {
        processDeckResponse(jsonObj);
    } else {
        prettyPrintJson(jsonObj, 4);
    }

    isLoadingMore = false;
    reply->deleteLater();
}

void TabArchidekt::processTopDecksResponse(QJsonObject reply)
{
    ArchidektDeckListingApiResponse deckData;
    deckData.fromJson(reply);

    // New search → clear everything
    if (currentPage == 1) {
        QLayoutItem *item;
        while ((item = resultsLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }

        listingsWidget = new ArchidektApiResponseDeckListingsDisplayWidget(resultsContainer, deckData, cardSizeSlider);

        connect(listingsWidget, &ArchidektApiResponseDeckListingsDisplayWidget::requestNavigation, this,
                &TabArchidekt::actNavigatePage);

        resultsLayout->addWidget(listingsWidget);
        return;
    }

    // Infinite scroll → append
    if (listingsWidget) {
        listingsWidget->append(deckData);
    }
}

void TabArchidekt::processDeckResponse(QJsonObject reply)
{
    ArchidektApiResponseDeck deckData;
    deckData.fromJson(reply);

    // We're in single deck mode - disable infinite scroll
    isListMode = false;

    // Clear existing results for single deck view
    QLayoutItem *item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    auto display = new ArchidektApiResponseDeckDisplayWidget(resultsContainer, deckData, cardSizeSlider);
    connect(display, &ArchidektApiResponseDeckDisplayWidget::requestNavigation, this, &TabArchidekt::actNavigatePage);
    connect(display, &ArchidektApiResponseDeckDisplayWidget::requestSearch, this, &TabArchidekt::doSearchImmediate);
    connect(display, &ArchidektApiResponseDeckDisplayWidget::openInDeckEditor, tabSupervisor,
            &TabSupervisor::openDeckInNewTab);
    resultsLayout->addWidget(display);
}

void TabArchidekt::prettyPrintJson(const QJsonValue &value, int indentLevel)
{
    const QString indent(indentLevel * 2, ' ');

    if (value.isObject()) {
        QJsonObject obj = value.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            qInfo().noquote() << indent + it.key() + ":";
            prettyPrintJson(it.value(), indentLevel + 1);
        }
    } else if (value.isArray()) {
        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i) {
            qInfo().noquote() << indent + QString("[%1]:").arg(i);
            prettyPrintJson(array[i], indentLevel + 1);
        }
    } else if (value.isString()) {
        qInfo().noquote() << indent + "\"" + value.toString() + "\"";
    } else if (value.isDouble()) {
        qInfo().noquote() << indent + QString::number(value.toDouble());
    } else if (value.isBool()) {
        qInfo().noquote() << indent + (value.toBool() ? "true" : "false");
    } else if (value.isNull()) {
        qInfo().noquote() << indent + "null";
    }
}