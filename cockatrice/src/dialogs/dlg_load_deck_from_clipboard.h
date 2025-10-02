/**
 * @file dlg_load_deck_from_clipboard.h
 * @ingroup LocalDeckStorageDialogs
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DLG_LOAD_DECK_FROM_CLIPBOARD_H
#define DLG_LOAD_DECK_FROM_CLIPBOARD_H

#include <QCheckBox>
#include <QDialog>

class DeckLoader;
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
     * The returned DeckLoader is parented to this object; make sure to take ownership of the DeckLoader if you intend
     * to use it, since otherwise it will get destroyed once this dlg is destroyed
     * @return The DeckLoader
     */
    virtual DeckLoader *getDeckList() const = 0;

protected:
    void setText(const QString &text);
    bool loadIntoDeck(DeckLoader *deckLoader) const;
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
    DeckLoader *deckList;

public:
    explicit DlgLoadDeckFromClipboard(QWidget *parent = nullptr);

    DeckLoader *getDeckList() const override
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
    DeckLoader *deckLoader;
    bool annotated;

public:
    explicit DlgEditDeckInClipboard(const DeckLoader &deckList, bool _annotated, QWidget *parent = nullptr);

    DeckLoader *getDeckList() const override
    {
        return deckLoader;
    }
};

#endif
