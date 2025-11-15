#include "dlg_load_deck_from_clipboard.h"

#include "../../../client/settings/cache_settings.h"
#include "../../deck_loader/deck_loader.h"
#include "dlg_settings.h"

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
AbstractDlgDeckTextEdit::AbstractDlgDeckTextEdit(QWidget *parent) : QDialog(parent)
{
    contentsEdit = new QPlainTextEdit;

    refreshButton = new QPushButton(tr("&Refresh"));
    connect(refreshButton, &QPushButton::clicked, this, &AbstractDlgDeckTextEdit::actRefresh);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(refreshButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AbstractDlgDeckTextEdit::actOK);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AbstractDlgDeckTextEdit::reject);

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

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &AbstractDlgDeckTextEdit::refreshShortcuts);
    refreshShortcuts();
}

void AbstractDlgDeckTextEdit::refreshShortcuts()
{
    refreshButton->setShortcut(
        SettingsCache::instance().shortcuts().getSingleShortcut("DlgLoadDeckFromClipboard/refreshButton"));
}

/**
 * Replaces the contents of the contentsEdit with the given text.
 * @param text The text
 */
void AbstractDlgDeckTextEdit::setText(const QString &text)
{
    contentsEdit->setPlainText(text);
}

/**
 * Tries to load the current contents of the contentsEdit into the DeckLoader
 *
 * @param deckLoader The DeckLoader to load the deck into
 * @return Whether the loading was successful
 */
bool AbstractDlgDeckTextEdit::loadIntoDeck(DeckLoader *deckLoader) const
{
    QString buffer = contentsEdit->toPlainText();

    if (buffer.contains("<cockatrice_deck version=\"1\">")) {
        return deckLoader->getDeckList()->loadFromString_Native(buffer);
    }

    QTextStream stream(&buffer);

    if (deckLoader->getDeckList()->loadFromStream_Plain(stream, true)) {
        if (loadSetNameAndNumberCheckBox->isChecked()) {
            deckLoader->resolveSetNameAndNumberToProviderID();
        } else {
            deckLoader->clearSetNamesAndNumbers();
        }
        return true;
    }

    return false;
}

void AbstractDlgDeckTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() & Qt::ControlModifier) {
        event->accept();
        actOK();
        return;
    }
    QDialog::keyPressEvent(event);
}

/**
 * Creates the dialog window for the "Load deck from clipboard" action
 *
 * @param parent The parent widget
 */
DlgLoadDeckFromClipboard::DlgLoadDeckFromClipboard(QWidget *parent) : AbstractDlgDeckTextEdit(parent), deckList(nullptr)
{
    setWindowTitle(tr("Load deck from clipboard"));

    DlgLoadDeckFromClipboard::actRefresh();
}

void DlgLoadDeckFromClipboard::actRefresh()
{
    setText(QApplication::clipboard()->text());
}

void DlgLoadDeckFromClipboard::actOK()
{
    deckList = new DeckLoader(this);

    if (loadIntoDeck(deckList)) {
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
    }
}

/**
 * Creates the dialog window for the "Edit deck in clipboard" action
 *
 * @param deckList The existing deck in the deck editor. Copies the instance
 * @param _annotated Whether to add annotations to the text that is loaded from the deck
 * @param parent The parent widget
 */
DlgEditDeckInClipboard::DlgEditDeckInClipboard(const DeckLoader &deckList, bool _annotated, QWidget *parent)
    : AbstractDlgDeckTextEdit(parent), annotated(_annotated)
{
    setWindowTitle(tr("Edit deck in clipboard"));

    deckLoader = new DeckLoader(deckList);
    deckLoader->setParent(this);

    DlgEditDeckInClipboard::actRefresh();
}

/**
 * Loads the contents of the DeckList into a String. Always loads it with addSetNameAndNumber=true
 * @param deckList The deck to load
 * @param addComments Whether to add annotations
 * @return A QString
 */
static QString deckListToString(const DeckLoader *deckList, bool addComments)
{
    QString buffer;
    QTextStream stream(&buffer);
    deckList->saveToStream_Plain(stream, addComments);
    return buffer;
}

void DlgEditDeckInClipboard::actRefresh()
{
    setText(deckListToString(deckLoader, annotated));
}

void DlgEditDeckInClipboard::actOK()
{
    if (loadIntoDeck(deckLoader)) {
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
    }
}