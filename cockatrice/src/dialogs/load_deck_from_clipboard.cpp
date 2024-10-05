#include "load_deck_from_clipboard.h"

#include "../deck/loader.h"
#include "../settings/cache.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>

LoadDeckFromClipboard::LoadDeckFromClipboard(QWidget *parent) : QDialog(parent), deckList(nullptr)
{
    contentsEdit = new QPlainTextEdit;

    refreshButton = new QPushButton(tr("&Refresh"));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(actRefresh()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(refreshButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(contentsEdit);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Load deck from clipboard"));
    resize(500, 500);

    actRefresh();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();
}

void LoadDeckFromClipboard::actRefresh()
{
    contentsEdit->setPlainText(QApplication::clipboard()->text());
}

void LoadDeckFromClipboard::refreshShortcuts()
{
    refreshButton->setShortcut(
        SettingsCache::instance().shortcuts().getSingleShortcut("DlgLoadDeckFromClipboard/refreshButton"));
}

void LoadDeckFromClipboard::actOK()
{
    QString buffer = contentsEdit->toPlainText();
    QTextStream stream(&buffer);

    auto *deckLoader = new DeckLoader;
    if (buffer.contains("<cockatrice_deck version=\"1\">")) {
        if (deckLoader->loadFromString_Native(buffer)) {
            deckList = deckLoader;
            accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
            delete deckLoader;
        }
    } else if (deckLoader->loadFromStream_Plain(stream)) {
        deckList = deckLoader;
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
        delete deckLoader;
    }
}
