#include "dlg_load_deck_from_website.h"

#include "../../deck_loader/card_node_function.h"
#include "parsers/deck_link_to_api_transformer.h"
#include "parsers/interface_json_deck_parser.h"
#include "parsers/manabox_json_deck_parser.h"

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>
#include <libcockatrice/card/import/card_name_normalizer.h>
#include <libcockatrice/network/network_manager.h>
#include <version_string.h>

namespace
{
constexpr qint64 maxDeckBytes = 16 * 1024 * 1024;
}

DlgLoadDeckFromWebsite::DlgLoadDeckFromWebsite(QWidget *parent) : QDialog(parent)
{
    nam = NetworkManagerFactory::create(this);
    layout = new QVBoxLayout(this);

    instructionLabel = new QLabel(this);
    layout->addWidget(instructionLabel);

    urlEdit = new QLineEdit(this);
    urlEdit->setText(QApplication::clipboard()->text());
    layout->addWidget(urlEdit);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgLoadDeckFromWebsite::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgLoadDeckFromWebsite::reject);
    layout->addWidget(buttonBox);

    retranslateUi();
    if (testValidUrl()) {
        QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
    }
}

void DlgLoadDeckFromWebsite::retranslateUi()
{
    setWindowTitle(tr("Load Deck from Website"));
    if (activeReply) {
        instructionLabel->setText(tr("Downloading deck... You can cancel at any time."));
    } else {
        instructionLabel->setText(tr("Paste a link to a decklist site here to import it.\n"
                                     "(Archidekt, Deckstats, ManaBox, Moxfield, and TappedOut are supported.)"));
    }
}

bool DlgLoadDeckFromWebsite::testValidUrl()
{
    ParsedDeckInfo info;
    return DeckLinkToApiTransformer::parseDeckUrl(urlEdit->text(), info);
}

void DlgLoadDeckFromWebsite::setLoading(bool loading)
{
    urlEdit->setEnabled(!loading);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!loading);
    retranslateUi();
}

void DlgLoadDeckFromWebsite::showImportError(const QString &message)
{
    urlEdit->setFocus();
    urlEdit->selectAll();
    auto *messageBox = new QMessageBox(QMessageBox::Warning, tr("Load Deck from Website"), message,
                                       QMessageBox::Ok, this);
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open();
}

void DlgLoadDeckFromWebsite::accept()
{
    if (activeReply) {
        return;
    }

    ParsedDeckInfo info;
    if (!DeckLinkToApiTransformer::parseDeckUrl(urlEdit->text(), info)) {
        showImportError(tr("The provided URL is not recognized as a valid deck URL.\n"
                           "Valid deck URLs look like this:\n\n"
                           "https://archidekt.com/decks/9999999\n"
                           "https://deckstats.net/decks/99999/9999999-your-deck-name/en\n"
                           "https://manabox.app/decks/XYZxx-XYZ99Yyy-xyzXzzz\n"
                           "https://moxfield.com/decks/XYZxx-XYZ99Yyy-xyzXzzz\n"
                           "https://tappedout.net/mtg-decks/your-deck-name/"));
        return;
    }

    QNetworkRequest request(QUrl(info.fullUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    downloadData.clear();
    activeReply = nam->get(request);
    auto *reply = activeReply;
    reply->setReadBufferSize(64 * 1024);
    setLoading(true);

    // Bound the entire download, including connections that never emit finished.
    auto *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, reply, [reply] {
        reply->setProperty("importTimedOut", true);
        reply->abort();
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, reply] {
        if (reply != activeReply) {
            return;
        }
        const qint64 remaining = maxDeckBytes - downloadData.size();
        const QByteArray chunk = reply->read(remaining + 1);
        if (chunk.size() > remaining) {
            reply->setProperty("importTooLarge", true);
            reply->abort();
            return;
        }
        downloadData += chunk;
    });
    connect(reply, &QNetworkReply::downloadProgress, reply, [reply](qint64 received, qint64 total) {
        if (received > maxDeckBytes || total > maxDeckBytes) {
            reply->setProperty("importTooLarge", true);
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, timeout, provider = info.provider] {
        timeout->stop();
        finishImport(reply, provider);
    });
    timeout->start(30000);
}

void DlgLoadDeckFromWebsite::reject()
{
    if (activeReply) {
        auto *reply = activeReply;
        activeReply = nullptr;
        downloadData.clear();
        // abort() can emit finished synchronously. Disconnect first so cancellation
        // cannot parse data, show an error, or accept the dialog.
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
    QDialog::reject();
}

void DlgLoadDeckFromWebsite::finishImport(QNetworkReply *reply, DeckProvider provider)
{
    if (reply != activeReply) {
        return;
    }
    activeReply = nullptr;
    setLoading(false);
    reply->deleteLater();
    QByteArray responseData;
    responseData.swap(downloadData);

    if (reply->property("importTimedOut").toBool()) {
        showImportError(tr("The deck download timed out. Please try again."));
        return;
    }
    if (reply->property("importTooLarge").toBool()) {
        showImportError(tr("The downloaded deck data is too large."));
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        showImportError(tr("Network error: %1").arg(reply->errorString()));
        return;
    }

    const QByteArray tail = reply->readAll();
    if (tail.size() > maxDeckBytes - responseData.size()) {
        showImportError(tr("The downloaded deck data is too large."));
        return;
    }
    responseData += tail;
    if (responseData.trimmed().isEmpty()) {
        showImportError(tr("Received empty deck data."));
        return;
    }

    DeckList importedDeck;
    if (provider == DeckProvider::Deckstats || provider == DeckProvider::TappedOut) {
        // A website error or login page is not a plain-text card list.
        if (responseData.trimmed().startsWith('<')) {
            showImportError(tr("The website returned a web page instead of a deck. Check that the deck is public."));
            return;
        }
        QString deckText = QString::fromUtf8(responseData);
        QTextStream stream(&deckText);
        if (!importedDeck.loadFromStream_Plain(stream, false, CardNameNormalizer())) {
            showImportError(tr("Failed to parse deck data."));
            return;
        }
        importedDeck.forEachCard(CardNodeFunction::ResolveProviderId());
    } else {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            showImportError(tr("Failed to parse deck data: %1").arg(parseError.errorString()));
            return;
        }
        const auto parser = createParserForProvider(provider);
        if (!doc.isObject() || !parser) {
            showImportError(tr("The website did not return a valid deck."));
            return;
        }
        importedDeck = parser->parse(doc.object());
    }

    if (importedDeck.getCardList().isEmpty()) {
        showImportError(tr("The website did not return any playable cards. "
                           "Check that the deck is public and not empty."));
        return;
    }

    // Commit only a complete successful import. Failed requests remain editable
    // and can be retried without replacing the previously accepted deck.
    deck = importedDeck;
    QDialog::accept();
}

QSharedPointer<IJsonDeckParser> DlgLoadDeckFromWebsite::createParserForProvider(DeckProvider provider)
{
    switch (provider) {
        case DeckProvider::Archidekt:
            return QSharedPointer<IJsonDeckParser>(new ArchidektJsonParser());
        case DeckProvider::ManaBox:
            return QSharedPointer<IJsonDeckParser>(new ManaBoxJsonParser());
        case DeckProvider::Moxfield:
            return QSharedPointer<IJsonDeckParser>(new MoxfieldJsonParser());
        default:
            return QSharedPointer<IJsonDeckParser>(nullptr);
    }
}
