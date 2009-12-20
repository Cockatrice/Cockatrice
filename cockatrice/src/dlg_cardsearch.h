#ifndef DLG_CARDSEARCH_H
#define DLG_CARDSEARCH_H

#include <QDialog>
#include <QList>
#include <QSet>

class QLineEdit;
class QCheckBox;

class DlgCardSearch : public QDialog {
	Q_OBJECT
private:
	QLineEdit *cardNameEdit, *cardTextEdit;
	QList<QCheckBox *> cardTypeCheckBoxes, cardColorCheckBoxes;
public:
	DlgCardSearch(QWidget *parent = 0);
	QString getCardName() const;
	QString getCardText() const;
	QSet<QString> getCardTypes() const;
	QSet<QString> getCardColors() const;
};

#endif