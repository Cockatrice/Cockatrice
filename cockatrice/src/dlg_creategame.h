#ifndef DLG_CREATEGAME_H
#define DLG_CREATEGAME_H

#include <QDialog>
#include "abstractclient.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QSpinBox;

class DlgCreateGame : public QDialog {
	Q_OBJECT
public:
	DlgCreateGame(AbstractClient *_client, int _roomId, QWidget *parent = 0);
private slots:
	void actOK();
	void checkResponse(ResponseCode response);
	void spectatorsAllowedChanged(int state);
private:
	AbstractClient *client;
	int roomId;

	QGroupBox *spectatorsGroupBox;
	QLabel *descriptionLabel, *passwordLabel, *maxPlayersLabel;
	QLineEdit *descriptionEdit, *passwordEdit;
	QSpinBox *maxPlayersEdit;
	QCheckBox *spectatorsAllowedCheckBox, *spectatorsNeedPasswordCheckBox, *spectatorsCanTalkCheckBox, *spectatorsSeeEverythingCheckBox;
	QPushButton *okButton, *cancelButton;
};

#endif
