#ifndef DLG_CREATEGAME_H
#define DLG_CREATEGAME_H

#include <QDialog>
#include <QMap>

class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QRadioButton;
class QGroupBox;
class QSpinBox;
class QDialogButtonBox;
class TabRoom;

class Response;
class ServerInfo_Game;

class DlgCreateGame : public QDialog {
    Q_OBJECT
public:
    DlgCreateGame(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent = 0);
    DlgCreateGame(const ServerInfo_Game &game, const QMap<int, QString> &_gameTypes, QWidget *parent = 0);
private slots:
    void actOK();
    void actReset();
    void checkResponse(const Response &response);
    void spectatorsAllowedChanged(int state);
private:
    TabRoom *room;
    QMap<int, QString> gameTypes;
    QMap<int, QRadioButton *> gameTypeCheckBoxes;

    QGroupBox *spectatorsGroupBox;
    QLabel *descriptionLabel, *passwordLabel, *maxPlayersLabel;
    QLineEdit *descriptionEdit, *passwordEdit;
    QSpinBox *maxPlayersEdit;
    QCheckBox *onlyBuddiesCheckBox, *onlyRegisteredCheckBox;
    QCheckBox *spectatorsAllowedCheckBox, *spectatorsNeedPasswordCheckBox, *spectatorsCanTalkCheckBox, *spectatorsSeeEverythingCheckBox;
    QDialogButtonBox *buttonBox;
    QPushButton *clearButton;
    QCheckBox *rememberGameSettings;
    
    void sharedCtor();
};

#endif
