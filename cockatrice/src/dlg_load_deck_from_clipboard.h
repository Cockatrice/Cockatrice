#ifndef DLG_LOAD_DECK_FROM_CLIPBOARD_H
#define DLG_LOAD_DECK_FROM_CLIPBOARD_H

#include <QDialog>

class DeckList;
class QPlainTextEdit;
class QPushButton;

class DlgLoadDeckFromClipboard : public QDialog {
	Q_OBJECT
private slots:
	void actOK();
	void actRefresh();
private:
	DeckList *deckList;
public:
	DlgLoadDeckFromClipboard(QWidget *parent = 0);
	DeckList *getDeckList() const { return deckList; }
private:
	QPlainTextEdit *contentsEdit;
	QPushButton *refreshButton, *okButton, *cancelButton;
};

#endif
