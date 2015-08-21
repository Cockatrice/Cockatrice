#ifndef DLG_LOAD_DECK_FROM_CLIPBOARD_H
#define DLG_LOAD_DECK_FROM_CLIPBOARD_H

#include <QDialog>

class DeckLoader;
class QPlainTextEdit;
class QPushButton;

class DlgLoadDeckFromClipboard : public QDialog {
    Q_OBJECT
private slots:
    void actOK();
    void actRefresh();
    void refreshShortcuts();
private:
    DeckLoader *deckList;
public:
    DlgLoadDeckFromClipboard(QWidget *parent = 0);
    DeckLoader *getDeckList() const { return deckList; }
private:
    QPlainTextEdit *contentsEdit;
    QPushButton *refreshButton;
};

#endif
