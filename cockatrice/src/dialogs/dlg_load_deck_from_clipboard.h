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
    DeckLoader *getDeckList() const
    {
        return deckList;
    }

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif
