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

/**
 * Creates the main layout and connects the signals that are common to all versions of this window
 */
void DlgLoadDeckFromClipboard::createMainLayout()
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

    resize(500, 500);

    actRefresh();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();
}

/**
 * Creates the dialog window for the "Load deck from clipboard" action
 *
 * @param parent The parent widget
 */
DlgLoadDeckFromClipboard::DlgLoadDeckFromClipboard(QWidget *parent) : QDialog(parent), deckList(nullptr)
{
    createMainLayout();

    setWindowTitle(tr("Load deck from clipboard"));
}

/**
 * Loads the contents of the DeckList into a String
 * @param deckList The deck to load
 * @return A QString
 */
static QString deckListToString(const DeckLoader *deckList)
{
    QString buffer;
    QTextStream stream(&buffer);
    deckList->saveToStream_Plain(stream);
    return buffer;
}

/**
 * Creates the dialog window for the "Edit deck in clipboard" action
 *
 * @param deck The existing deck in the deck editor
 * @param parent The parent widget
 */
DlgLoadDeckFromClipboard::DlgLoadDeckFromClipboard(const DeckLoader &deck, QWidget *parent)
    : DlgLoadDeckFromClipboard(parent)
{
    createMainLayout();

    setWindowTitle(tr("Edit deck in clipboard"));

    deckList = new DeckLoader(deck);
    deckList->setParent(this);

    contentsEdit->setPlainText(deckListToString(deckList));
}

void DlgLoadDeckFromClipboard::actRefresh()
{
    if (deckList) {
        contentsEdit->setPlainText(deckListToString(deckList));
    } else {
        contentsEdit->setPlainText(QApplication::clipboard()->text());
    }
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

    if (!deckList) {
        deckList = new DeckLoader;
        deckList->setParent(this);
    }

    if (buffer.contains("<cockatrice_deck version=\"1\">")) {
        if (deckList->loadFromString_Native(buffer)) {
            accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
        }
    } else if (deckList->loadFromStream_Plain(stream)) {
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