#ifndef DLG_CREATEGAME_H
#define DLG_CREATEGAME_H

#include <QDialog>
#include "client.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QSpinBox;

class DlgCreateGame : public QDialog {
	Q_OBJECT
public:
	DlgCreateGame(Client *_client, QWidget *parent = 0);
private slots:
	void actOK();
	void checkResponse(ResponseCode response);
	void spectatorsAllowedChanged(int state);
private:
	Client *client;

	QGroupBox *spectatorsGroupBox;
	QLabel *descriptionLabel, *passwordLabel, *maxPlayersLabel;
	QLineEdit *descriptionEdit, *passwordEdit;
	QSpinBox *maxPlayersEdit;
	QCheckBox *spectatorsAllowedCheckBox, *spectatorsNeedPasswordCheckBox, *spectatorsCanTalkCheckBox, *spectatorsSeeEverythingCheckBox;
	QPushButton *okButton, *cancelButton;
};

#endif
