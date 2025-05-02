#include "dlg_get_card_prices.h"

#include "../client/ui/widgets/cards/card_info_picture_widget.h"
#include "../client/ui/widgets/general/layout_containers/flow_widget.h"
#include "../deck/deck_loader.h"
#include "../game/cards/card_database_manager.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QScrollArea>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <limits>

DlgGetCardPrices::DlgGetCardPrices(QWidget *parent, DeckListModel *_model) : QDialog(parent), model(_model)
{
    setMinimumSize(600, 500);
    setAcceptDrops(true);

    instructionLabel = new QLabel(this);
    instructionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    minTotalLabel = new QLabel(this);
    maxTotalLabel = new QLabel(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    excludeLandsCheckBox = new QCheckBox("Exclude Lands", this);
    excludeLandsCheckBox->setChecked(true);
    connect(excludeLandsCheckBox, &QCheckBox::toggled, this, &DlgGetCardPrices::onExcludeLandsChanged);

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"Card Name", "Actual", "Min", "Max"});

    tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setSortingEnabled(true);

    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &DlgGetCardPrices::onSelectionChanged);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(tableWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgGetCardPrices::actOK);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgGetCardPrices::reject);

    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressBar = new QProgressBar(this);
    progressLabel = new QLabel("0/0 cards loaded", this);

    uuidProgressBar = new QProgressBar(this);
    uuidProgressLabel = new QLabel("0/0 prices fetched", this);

    progressBar->setMinimum(0);
    progressBar->setMaximum(0); // Set to 0 until we know how many cards
    progressBar->setTextVisible(false);

    uuidProgressBar->setMinimum(0);
    uuidProgressBar->setMaximum(0);
    uuidProgressBar->setTextVisible(false);

    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(progressBar);
    progressLayout->addWidget(uuidProgressLabel);
    progressLayout->addWidget(uuidProgressBar);

    mainLayout->addWidget(instructionLabel);
    mainLayout->addWidget(minTotalLabel);
    mainLayout->addWidget(maxTotalLabel);
    mainLayout->addWidget(excludeLandsCheckBox);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(buttonBox);
    mainLayout->addLayout(progressLayout);

    retranslateUi();

    connect(this, &DlgGetCardPrices::allRequestsFinished, this, &DlgGetCardPrices::onAllRequestsFinished);

    startCardPriceRequests();
}

void DlgGetCardPrices::startCardPriceRequests()
{
    totalCardsToProcess = 0;
    finishedCardCount = 0;
    totalRequests = 0;
    finishedRequests = 0;

    tableWidget->clearContents();
    tableWidget->setRowCount(0);
    totalPrice = 0.0;
    totalMinPrice = 0.0;
    totalMaxPrice = 0.0;

    printingsByCardName.clear();
    uuidPrices.clear();
    uuidToCardName.clear();
    pendingPerCard.clear();

    if (!model)
        return;

    DeckList *decklist = model->getDeckList();
    if (!decklist)
        return;

    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return;

    int requestDelay = 0;

    for (auto *i : *listRoot) {
        auto *countCurrentZone = dynamic_cast<InnerDecklistNode *>(i);
        if (!countCurrentZone)
            continue;

        for (auto *cardNode : *countCurrentZone) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(cardNode);
            if (!currentCard)
                continue;

            QString cardName = currentCard->getName();

            if (excludeLandsCheckBox->isChecked()) {
                CardInfoPtr card = CardDatabaseManager::getInstance()->getCard(cardName);
                if (card->getMainCardType() == "Land")
                    continue;
            }

            CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(cardName);
            if (!info)
                continue;

            QString actualUuid = currentCard->getCardProviderId();
            QList<QString> &uuidList = printingsByCardName[cardName];

            CardInfoPerSetMap allSets = info->getSets();
            for (QList cardsInSet : allSets) {
                for (CardInfoPerSet cardInSet : cardsInSet) {
                    QString uuid = cardInSet.getProperty("uuid");
                    uuidList.append(uuid);
                    uuidToCardName[uuid] = cardName;
                }
            }

            // Make sure no duplicates
            uuidList.removeDuplicates();

            pendingPerCard[cardName] = uuidList.size();

            totalCardsToProcess++;
            progressBar->setMaximum(totalCardsToProcess);
            progressBar->setValue(0);
            progressLabel->setText(QString("0/%1 cards loaded").arg(totalCardsToProcess));
            finishedCardCount = 0;

            for (const QString &uuid : uuidList) {
                QTimer::singleShot(requestDelay++ * 100, this, [this, uuid]() { sendCardPriceRequest(uuid); });
                totalRequests++;
                uuidProgressBar->setMaximum(totalRequests);
            }

            // Store actual UUID for later comparison
            actualUuidsByCardName[cardName] = actualUuid;
        }
    }
}

void DlgGetCardPrices::sendCardPriceRequest(const QString &cardUuid)
{
    QUrl url(QString("https://api.scryfall.com/cards/" + cardUuid));
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager.get(request);

    reply->setProperty("cardUuid", cardUuid);
    connect(reply, &QNetworkReply::finished, this, &DlgGetCardPrices::onCardPriceReply);

    pendingRequests++;
}

void DlgGetCardPrices::onCardPriceReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    QString uuid = reply->property("cardUuid").toString();
    QString cardName = uuidToCardName.value(uuid);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (jsonDoc.isObject()) {
            QJsonObject obj = jsonDoc.object();
            if (obj.contains("prices") && obj["prices"].isObject()) {
                QJsonObject prices = obj["prices"].toObject();
                double eurValue = prices["eur"].toString().toDouble();
                uuidPrices[uuid] = eurValue;
            }
        }
    } else {
        qWarning() << "Failed to get price for" << uuid << reply->errorString();
    }

    reply->deleteLater();

    finishedRequests++;
    uuidProgressBar->setValue(finishedRequests);
    uuidProgressLabel->setText(QString("%1/%2 prices fetched").arg(finishedRequests).arg(totalRequests));

    if (--pendingPerCard[cardName] == 0) {
        finishedCardCount++;
        progressBar->setValue(finishedCardCount);
        progressLabel->setText(QString("%1/%2 cards loaded").arg(finishedCardCount).arg(totalCardsToProcess));

        const QList<QString> &uuids = printingsByCardName.value(cardName);
        QString actualUuid = actualUuidsByCardName.value(cardName);

        double minPrice = std::numeric_limits<double>::max();
        double maxPrice = std::numeric_limits<double>::lowest();
        double actualPrice = -1.0;

        bool hasValidPrice = false;

        for (const QString &u : uuids) {
            double price = uuidPrices.value(u, -1.0);
            if (price <= 0.0)
                continue;

            hasValidPrice = true;

            if (u == actualUuid)
                actualPrice = price;

            if (price <= minPrice)
                minPrice = price;
            if (price >= maxPrice)
                maxPrice = price;
        }

        if (!hasValidPrice) {
            // No prices found at all
            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);
            tableWidget->setItem(row, 0, new CardPriceTableWidgetItem(cardName));
            tableWidget->setItem(row, 1, new CardPriceTableWidgetItem("N/A"));
            tableWidget->setItem(row, 2, new CardPriceTableWidgetItem("N/A"));
            tableWidget->setItem(row, 3, new CardPriceTableWidgetItem("N/A"));
        } else {
            if (actualPrice < 0.0)
                actualPrice = minPrice;  // fallback

            totalPrice += actualPrice;
            totalMinPrice += minPrice;
            totalMaxPrice += maxPrice;

            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);
            tableWidget->setItem(row, 0, new CardPriceTableWidgetItem(cardName));
            tableWidget->setItem(row, 1, new CardPriceTableWidgetItem(QString::number(actualPrice, 'f', 2)));
            tableWidget->setItem(row, 2, new CardPriceTableWidgetItem(QString::number(minPrice, 'f', 2)));
            tableWidget->setItem(row, 3, new CardPriceTableWidgetItem(QString::number(maxPrice, 'f', 2)));
        }
    }

    pendingRequests--;
    if (pendingRequests == 0) {
        emit allRequestsFinished();
    }
}

void DlgGetCardPrices::onAllRequestsFinished()
{
    instructionLabel->setText(tr("Total Price: %1 EUR (%2%)")
                              .arg(totalPrice, 0, 'f', 2)
                              .arg(100.0, 0, 'f', 2));

    minTotalLabel->setText(tr("Minimum Possible Total: %1 EUR").arg(totalMinPrice, 0, 'f', 2));
    maxTotalLabel->setText(tr("Maximum Possible Total: %1 EUR").arg(totalMaxPrice, 0, 'f', 2));
}

void DlgGetCardPrices::onSelectionChanged()
{
    double selectedTotal = 0.0;
    for (QModelIndex index : tableWidget->selectionModel()->selectedRows()) {
        QString priceStr = tableWidget->item(index.row(), 1)->text();
        selectedTotal += priceStr.toDouble();
    }

    double percentage = (totalPrice > 0.0) ? (selectedTotal / totalPrice) * 100.0 : 0.0;
    instructionLabel->setText(
        tr("Selected cards total: %1 EUR (%2%)").arg(selectedTotal, 0, 'f', 2).arg(percentage, 0, 'f', 2));
}

void DlgGetCardPrices::onExcludeLandsChanged(bool checked)
{
    Q_UNUSED(checked)
    startCardPriceRequests();
}

void DlgGetCardPrices::retranslateUi()
{
    instructionLabel->setText(tr("Fetching card prices... Please wait."));
}

void DlgGetCardPrices::actOK()
{
    accept();
}
