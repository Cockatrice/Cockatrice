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
	QString getPassword();
private:
	QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel;
	QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit;
	QPushButton *okButton, *cancelButton;
};

#endif
