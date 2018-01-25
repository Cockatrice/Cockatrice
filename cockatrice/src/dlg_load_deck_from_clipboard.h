#ifndef DLG_LOAD_DECK_FROM_CLIPBOARD_H
#define DLG_LOAD_DECK_FROM_CLIPBOARD_H

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

public:
    explicit DlgLoadDeckFromClipboard(QWidget *parent = nullptr);
    DeckLoader *getDeckList() const
    {
        return deckList;
    }
};

#endif
