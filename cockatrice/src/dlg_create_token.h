#ifndef DLG_CREATETOKEN_H
#define DLG_CREATETOKEN_H

#include <QDialog>
#include <QLineEdit>

class QLabel;
class QComboBox;
class QPushButton;

class DlgCreateToken : public QDialog {
	Q_OBJECT
public:
	DlgCreateToken(QWidget *parent = 0);
	QString getName() const;
	QString getColor() const;
	QString getPT() const;
	QString getAnnotation() const;
private slots:
	void actOk();
private:
	QLabel *nameLabel, *colorLabel, *ptLabel, *annotationLabel;
	QComboBox *colorEdit;
	QLineEdit *nameEdit, *ptEdit, *annotationEdit;
	QPushButton *okButton, *cancelButton;
};

#endif
