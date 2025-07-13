#include "dlg_load_deck_from_website.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>

DlgLoadDeckFromWebsite::DlgLoadDeckFromWebsite(QWidget *parent) : QDialog(parent)
{
    nam = new QNetworkAccessManager(this);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    instructionLabel = new QLabel(this);
    layout->addWidget(instructionLabel);

    urlEdit = new QLineEdit(this);
    urlEdit->setText(QApplication::clipboard()->text());

    layout->addWidget(urlEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);

    if (testValidUrl()) {
        QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
        hide();
    }

    retranslateUi();
}

void DlgLoadDeckFromWebsite::retranslateUi()
{
    instructionLabel->setText(tr("Paste a link to a decklist site here to import it.\n(Archidekt, Deckstats, Moxfield, "
                                 "and TappedOut are supported.)"));
}

bool DlgLoadDeckFromWebsite::testValidUrl()
{
    ParsedDeckInfo info;
    return DeckLinkToApiTransformer::parseDeckUrl(urlEdit->text(), info);
}

void DlgLoadDeckFromWebsite::accept()
{
    ParsedDeckInfo info;
    if (DeckLinkToApiTransformer::parseDeckUrl(urlEdit->text(), info)) {
        qInfo() << info.baseUrl << info.deckID << info.fullUrl;

        auto jsonParser = createParserForProvider(info.provider);
        if (!jsonParser && info.provider != DeckProvider::Deckstats && info.provider != DeckProvider::TappedOut) {
            qWarning() << "No parser found for provider";
            QMessageBox::warning(this, tr("Load Deck from Website"), tr("No parser available for this deck provider."));
            QDialog::reject();
            return;
        }

        QNetworkRequest request(QUrl(info.fullUrl));
        QNetworkReply *reply = nam->get(request);

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Network error:" << reply->errorString();
            QMessageBox::warning(this, tr("Load Deck from Website"), tr("Network error: %1").arg(reply->errorString()));
            reply->deleteLater();
            QDialog::reject();
            return;
        }

        QByteArray responseData = reply->readAll();
        reply->deleteLater();

        // Special handling for Deckstats and TappedOut .txt
        if (info.provider == DeckProvider::Deckstats || info.provider == DeckProvider::TappedOut) {
            QString deckText = QString::fromUtf8(responseData);
            if (deckText.isEmpty()) {
                qWarning() << "Response is empty";
                QMessageBox::warning(this, tr("Load Deck from Website"), tr("Received empty deck data."));
                QDialog::reject();
                return;
            }

            // Parse the plain text deck here
            DeckLoader *loader = new DeckLoader();
            QTextStream stream(&deckText);
            loader->loadFromStream_Plain(stream, false);
            loader->resolveSetNameAndNumberToProviderID();
            deck = loader;

            QDialog::accept();
            return;
        }

        // Normal JSON parsing for other providers
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << parseError.errorString();
            QMessageBox::warning(this, tr("Load Deck from Website"),
                                 tr("Failed to parse deck data: %1").arg(parseError.errorString()));
            QDialog::reject();
            return;
        }

        deck = jsonParser->parse(doc.object());
        QDialog::accept();

    } else {
        qInfo() << "URL not recognized";
        QMessageBox::warning(this, tr("Load Deck from Website"),
                             tr("The provided URL is not recognized as a valid deck URL."));
        QDialog::reject();
    }
}

QSharedPointer<IJsonDeckParser> DlgLoadDeckFromWebsite::createParserForProvider(DeckProvider provider)
{
    switch (provider) {
        case DeckProvider::Archidekt:
            return QSharedPointer<IJsonDeckParser>(new ArchidektJsonParser());
        case DeckProvider::Moxfield:
            return QSharedPointer<IJsonDeckParser>(new MoxfieldJsonParser());
        default:
            return QSharedPointer<IJsonDeckParser>(nullptr);
    }
}