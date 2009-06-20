#ifndef DLG_CREATEGAME_H
#define DLG_CREATEGAME_H

#include <QDialog>
#include "client.h"

class QLabel;
class QLineEdit;
class QPushButton;

class DlgCreateGame : public QDialog {
	Q_OBJECT
public:
	DlgCreateGame(Client *_client, QWidget *parent = 0);
private slots:
	void actOK();
	void checkResponse(ServerResponse response);
private:
	Client *client;

	QLabel *descriptionLabel, *passwordLabel, *maxPlayersLabel;
	QLineEdit *descriptionEdit, *passwordEdit, *maxPlayersEdit;
	QPushButton *okButton, *cancelButton;
};

#endif

