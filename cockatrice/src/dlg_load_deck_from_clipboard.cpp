#include <QClipboard>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QApplication>
#include <QTextStream>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "dlg_load_deck_from_clipboard.h"
#include "deck_loader.h"
#include "settingscache.h"

DlgLoadDeckFromClipboard::DlgLoadDeckFromClipboard(QWidget *parent)
    : QDialog(parent), deckList(0)
{
    contentsEdit = new QPlainTextEdit;
    
    refreshButton = new QPushButton(tr("&Refresh"));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(actRefresh()));
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(refreshButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(contentsEdit);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Load deck from clipboard"));
    resize(500, 500);
    
    actRefresh();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
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
    
    DeckLoader *l = new DeckLoader;
    if (buffer.contains("<cockatrice_deck version=\"1\">"))
    {
        if (l->loadFromString_Native(buffer))
        {
            deckList = l;
            accept();
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
            delete l;
        }
    }
    else if (l->loadFromStream_Plain(stream)) {
        deckList = l;
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
        delete l;
    }
}
