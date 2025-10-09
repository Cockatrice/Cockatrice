/**
 * @file dlg_create_game.h
 * @ingroup RoomDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_CREATEGAME_H
#define DLG_CREATEGAME_H

#include <QDialog>
#include <QMap>
#include <libcockatrice/utility/macros.h>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class Response;
class ServerInfo_Game;
class TabRoom;

class DlgCreateGame : public QDialog
{
    Q_OBJECT
public:
    DlgCreateGame(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent = nullptr);
    DlgCreateGame(const ServerInfo_Game &game, const QMap<int, QString> &_gameTypes, QWidget *parent = nullptr);
private slots:
    void actOK();
    void actReset();
    void checkResponse(const Response &response);
    void spectatorsAllowedChanged(QT_STATE_CHANGED_T state);

private:
    TabRoom *room;
    QMap<int, QString> gameTypes;
    QMap<int, QRadioButton *> gameTypeCheckBoxes;

    QGroupBox *generalGroupBox, *spectatorsGroupBox, *gameSetupOptionsGroupBox;
    QLabel *descriptionLabel, *passwordLabel, *maxPlayersLabel, *startingLifeTotalLabel, *shareDecklistsOnLoadLabel;
    QLineEdit *descriptionEdit, *passwordEdit;
    QSpinBox *maxPlayersEdit, *startingLifeTotalEdit;
    QCheckBox *onlyBuddiesCheckBox, *onlyRegisteredCheckBox;
    QCheckBox *spectatorsAllowedCheckBox, *spectatorsNeedPasswordCheckBox, *spectatorsCanTalkCheckBox,
        *spectatorsSeeEverythingCheckBox, *createGameAsSpectatorCheckBox;
    QCheckBox *shareDecklistsOnLoadCheckBox;
    QDialogButtonBox *buttonBox;
    QPushButton *clearButton;
    QCheckBox *rememberGameSettings;

    void sharedCtor();
};

#endif
