#include "dlg_load_deck_from_clipboard.h"
#include "deck_loader.h"
#include "settingscache.h"
#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>

DlgLoadDeckFromClipboard::DlgLoadDeckFromClipboard(QWidget *parent) : QDialog(parent), deckList(nullptr)
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
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();
}

void DlgLoadDeckFromClipboard::actRefresh()
{
    contentsEdit->setPlainText(QApplication::clipboard()->text());
}

void DlgLoadDeckFromClipboard::refreshShortcuts()
{
    refreshButton->setShortcut(settingsCache->shortcuts().getSingleShortcut("DlgLoadDeckFromClipboard/refreshButton"));
}

void DlgLoadDeckFromClipboard::actOK()
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
