#ifndef DLG_CREATETOKEN_H
#define DLG_CREATETOKEN_H

#include <QDialog>
#include <QLineEdit>

class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
class DeckList;
class CardDatabaseModel;
class CardDatabaseDisplayModel;

class DlgCreateToken : public QDialog {
	Q_OBJECT
public:
	DlgCreateToken(DeckList *_deck, QWidget *parent = 0);
	QString getName() const;
	QString getColor() const;
	QString getPT() const;
	QString getAnnotation() const;
	bool getDestroy() const;
private slots:
	void actOk();
private:
	CardDatabaseModel *cardDatabaseModel;
	CardDatabaseDisplayModel *cardDatabaseDisplayModel;
	QLabel *nameLabel, *colorLabel, *ptLabel, *annotationLabel;
	QComboBox *colorEdit;
	QLineEdit *nameEdit, *ptEdit, *annotationEdit;
	QCheckBox *destroyCheckBox;
};

#endif
