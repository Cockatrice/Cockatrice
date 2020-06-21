#include "dlg_load_url_deck.h"

#include "deck_loader.h"
#include "settingscache.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

DlgLoadURLDeck::DlgLoadURLDeck(QWidget *parent) : QDialog(parent), deckList(nullptr)
{
    urlLabel = new QLabel(tr("URL:"));
    urlEdit = new QLineEdit();
    urlLabel->setBuddy(urlEdit);
    urlEdit->setText("https://");

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(urlLabel, 0, 0);
    grid->addWidget(urlEdit, 0, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Load deck from URL"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(400);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(deckDownloadFinished(QNetworkReply *)));
}

void DlgLoadURLDeck::actOk()
{
    QUrl url(urlEdit->text());
    QNetworkRequest req(url);
    networkManager->get(req);
}

void DlgLoadURLDeck::deckDownloadFinished(QNetworkReply *reply)
{
    QString buffer = QString(reply->peek(reply->size()));
    QTextStream stream(reply);

    auto *deckLoader = new DeckLoader;
    if (buffer.contains("<cockatrice_deck version=\"1\">")) {
        if (deckLoader->loadFromString_Native(buffer)) {
            deckList = deckLoader;
            reply->deleteLater();
            accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
            delete deckLoader;
        }
    } else if (deckLoader->loadFromStream_Plain(stream)) {
        deckList = deckLoader;
        reply->deleteLater();
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
        delete deckLoader;
    }
    reply->deleteLater();
}
