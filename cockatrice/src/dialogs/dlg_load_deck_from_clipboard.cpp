#include "dlg_load_deck_from_clipboard.h"

#include "../deck/deck_loader.h"
#include "../settings/cache_settings.h"

#include <QApplication>
#include <QCheckBox>
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

    loadSetNameAndNumberCheckBox = new QCheckBox(tr("Parse Set Name and Number (if available)"));
    loadSetNameAndNumberCheckBox->setChecked(true);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loadSetNameAndNumberCheckBox);
    buttonLayout->addWidget(buttonBox);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(contentsEdit);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("Load deck from clipboard"));
    resize(500, 500);

    actRefresh();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();
}

void DlgLoadDeckFromClipboard::actRefresh()
{
    contentsEdit->setPlainText(QApplication::clipboard()->text());
}

void DlgLoadDeckFromClipboard::refreshShortcuts()
{
    refreshButton->setShortcut(
        SettingsCache::instance().shortcuts().getSingleShortcut("DlgLoadDeckFromClipboard/refreshButton"));
}

void DlgLoadDeckFromClipboard::actOK()
{
    QString buffer = contentsEdit->toPlainText();
    QTextStream stream(&buffer);

    auto *deckLoader = new DeckLoader;
    deckLoader->setParent(this);

    if (buffer.contains("<cockatrice_deck version=\"1\">")) {
        if (deckLoader->loadFromString_Native(buffer)) {
            deckList = deckLoader;
            accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
        }
    } else if (deckLoader->loadFromStream_Plain(stream)) {
        deckList = deckLoader;
        if (loadSetNameAndNumberCheckBox->isChecked()) {
            deckList->resolveSetNameAndNumberToProviderID();
        } else {
            deckList->clearSetNamesAndNumbers();
        }
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
    }
}

void DlgLoadDeckFromClipboard::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() & Qt::ControlModifier) {
        event->accept();
        actOK();
        return;
    }
    QDialog::keyPressEvent(event);
}