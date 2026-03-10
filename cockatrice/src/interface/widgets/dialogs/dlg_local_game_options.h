/**
 * @file dlg_local_game_options.h
 * @ingroup RoomDialogs
 * @brief Dialog for configuring local game options.
 *
 * Provides a user interface for setting up local games with configurable
 * number of players and starting life total.
 */

#ifndef DLG_LOCAL_GAME_OPTIONS_H
#define DLG_LOCAL_GAME_OPTIONS_H

#include <QDialog>

struct LocalGameOptions
{
    int numberPlayers = 1;
    int startingLifeTotal = 20;
};

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

    [[nodiscard]] LocalGameOptions getOptions() const;

private slots:
    void actOK();

private:
    QGroupBox *generalGroupBox;
    QGroupBox *gameSetupOptionsGroupBox;

    QLabel *numberPlayersLabel;
    QSpinBox *numberPlayersEdit;

    QLabel *startingLifeTotalLabel;
    QSpinBox *startingLifeTotalEdit;

    QCheckBox *rememberSettingsCheckBox;

    QDialogButtonBox *buttonBox;
};

#endif // DLG_LOCAL_GAME_OPTIONS_H
