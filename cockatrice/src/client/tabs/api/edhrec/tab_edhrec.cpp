#include "tab_edhrec.h"

#include "api_response/edhrec_commander_api_response.h"
#include "edhrec_commander_api_response_display_widget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QResizeEvent>

TabEdhRec::TabEdhRec(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    networkManager = new QNetworkAccessManager(this);
    setCentralWidget(scrollArea);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    networkManager->setTransferTimeout(); // Use Qt's default timeout
#endif

    networkManager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(processApiJson(QNetworkReply *)));
}

void TabEdhRec::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (scrollArea) {
        if (scrollArea->widget()) {
            scrollArea->widget()->resize(event->size());
        }
    }
}

void TabEdhRec::retranslateUi()
{
}

void TabEdhRec::setCard(CardInfoPtr _cardToQuery, bool isCommander)
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

    networkManager->get(request);
}

void TabEdhRec::processApiJson(QNetworkReply *reply)
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

    // qDebug() << jsonObj;

    EdhrecCommanderApiResponse deckData;
    deckData.fromJson(jsonObj);

    displayWidget = new EdhrecCommanderApiResponseDisplayWidget(scrollArea, deckData);
    // flowWidget->addWidget(displayWidget);
    scrollArea->setWidget(displayWidget);

    reply->deleteLater();
    update();
}

void TabEdhRec::prettyPrintJson(const QJsonValue &value, int indentLevel)
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
