#ifndef DLG_LOAD_DECK_FROM_CLIPBOARD_H
#define DLG_LOAD_DECK_FROM_CLIPBOARD_H

#include <QCheckBox>
#include <QDialog>

class DeckLoader;
class QPlainTextEdit;
class QPushButton;

class DlgLoadDeckFromClipboard : public QDialog
{
    Q_OBJECT
private slots:
    void actOK();
    void actRefresh();
    void refreshShortcuts();

private:
    DeckLoader *deckList;
    QPlainTextEdit *contentsEdit;
    QPushButton *refreshButton;
    QCheckBox *loadSetNameAndNumberCheckBox;

public:
    explicit DlgLoadDeckFromClipboard(QWidget *parent = nullptr);
    explicit DlgLoadDeckFromClipboard(const DeckLoader &deck, QWidget *parent = nullptr);

    /**
     * Gets the loaded deck. Only call this method after this dialog window has been successfully exec'd.
     *
     * The returned DeckLoader is parented to this object; make sure to take ownership of the DeckLoader if you intend
     * to use it, since otherwise it will get destroyed once this dlg is destroyed
     * @return The DeckLoader
     */
    DeckLoader *getDeckList() const
    {
        return deckList;
    }

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif
