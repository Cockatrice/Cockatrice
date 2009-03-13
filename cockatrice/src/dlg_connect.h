#ifndef DLG_CONNECT_H
#define DLG_CONNECT_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

class DlgConnect : public QDialog {
	Q_OBJECT
public:
	DlgConnect(QWidget *parent = 0);
	QString getHost();
	int getPort();
	QString getPlayerName();
private:
	QLabel *hostLabel, *portLabel, *playernameLabel;
	QLineEdit *hostEdit, *portEdit, *playernameEdit;
	QPushButton *okButton, *cancelButton;
};

#endif
