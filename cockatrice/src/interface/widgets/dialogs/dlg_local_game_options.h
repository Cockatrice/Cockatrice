/**
 * @file dlg_local_game_options.h
 * @ingroup RoomDialogs
 * @brief Dialog for configuring local game options.
 */

#ifndef DLG_LOCAL_GAME_OPTIONS_H
#define DLG_LOCAL_GAME_OPTIONS_H

#include <QDialog>

class GameZoneOptionsWidget;
class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QSpinBox;

class DlgLocalGameOptions : public QDialog
{
    Q_OBJECT
public:
    explicit DlgLocalGameOptions(QWidget *parent = nullptr);

    int getNumberPlayers() const;
    int getStartingLifeTotal() const;
    bool getEnableCommandZone() const;
    bool getEnableCompanionZone() const;
    bool getEnableBackgroundZone() const;
    bool getSpectatorsSeeEverything() const;

private slots:
    void actOK();

private:
    QGroupBox *generalGroupBox;
    QGroupBox *gameSetupOptionsGroupBox;

    QLabel *numberPlayersLabel;
    QSpinBox *numberPlayersEdit;

    QLabel *startingLifeTotalLabel;
    QSpinBox *startingLifeTotalEdit;

    GameZoneOptionsWidget *zoneOptionsWidget;
    QCheckBox *spectatorsSeeEverythingCheckBox;
    QCheckBox *rememberSettingsCheckBox;

    QDialogButtonBox *buttonBox;
};

#endif // DLG_LOCAL_GAME_OPTIONS_H
