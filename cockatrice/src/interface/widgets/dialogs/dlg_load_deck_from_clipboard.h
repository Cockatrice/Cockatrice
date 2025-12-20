/**
 * @file dlg_load_deck_from_clipboard.h
 * @ingroup LocalDeckStorageDialogs
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DLG_LOAD_DECK_FROM_CLIPBOARD_H
#define DLG_LOAD_DECK_FROM_CLIPBOARD_H

#include "../../deck_loader/loaded_deck.h"

#include <QCheckBox>
#include <QDialog>

class QPlainTextEdit;
class QPushButton;

/**
 * Base class for dialog windows for actions that involve loading decks from text input.
 */
class AbstractDlgDeckTextEdit : public QDialog
{
    Q_OBJECT
private:
    QPlainTextEdit *contentsEdit;
    QPushButton *refreshButton;
    QCheckBox *loadSetNameAndNumberCheckBox;

private slots:
    void refreshShortcuts();

public:
    explicit AbstractDlgDeckTextEdit(QWidget *parent = nullptr);

    /**
     * Gets the loaded deck. Only call this method after this dialog window has been successfully exec'd.
     *
     * @return The loaded decklist
     */
    [[nodiscard]] virtual const DeckList &getDeckList() = 0;

protected:
    void setText(const QString &text);
    bool loadIntoDeck(DeckList &deckList) const;
    void keyPressEvent(QKeyEvent *event) override;

protected slots:
    virtual void actOK() = 0;
    virtual void actRefresh() = 0;
};

/**
 * Dialog window for the "Load deck from clipboard" action
 */
class DlgLoadDeckFromClipboard : public AbstractDlgDeckTextEdit
{
    Q_OBJECT
protected slots:
    void actOK() override;
    void actRefresh() override;

private:
    DeckList deckList;

public:
    explicit DlgLoadDeckFromClipboard(QWidget *parent = nullptr);

    [[nodiscard]] const DeckList &getDeckList() override
    {
        return deckList;
    }
};

/**
 * Dialog window for the "Edit deck in clipboard" action
 */
class DlgEditDeckInClipboard : public AbstractDlgDeckTextEdit
{
    Q_OBJECT
protected slots:
    void actOK() override;
    void actRefresh() override;

private:
    DeckList deckList;
    bool annotated;

public:
    explicit DlgEditDeckInClipboard(const DeckList &_deckList, bool _annotated, QWidget *parent = nullptr);

    [[nodiscard]] const DeckList &getDeckList() override
    {
        return deckList;
    }
};

#endif
