/**
 * @file create_game_form_widget.h
 * @ingroup RoomWidgets
 */

#ifndef CREATE_GAME_FORM_WIDGET_H
#define CREATE_GAME_FORM_WIDGET_H

#include "game_creation_options.h"

#include <QMap>
#include <QWidget>
#include <libcockatrice/utility/macros.h>
#include <qtmetamacros.h>

class QCheckBox;
class QLineEdit;
class QRadioButton;
class QSpinBox;

/**
 * @brief The editable create-game form, independent of settings, the network, and the room.
 *
 * The dialog loads @ref GameCreationOptions into it and reads them back; a future host
 * activity can do the same without duplicating the widget layout.
 */
class CreateGameFormWidget : public QWidget
{
    Q_OBJECT
public:
    CreateGameFormWidget(const QMap<int, QString> &gameTypes,
                         bool registeredUser,
                         bool judge,
                         QWidget *parent = nullptr);

    void setOptions(const GameCreationOptions &options);
    [[nodiscard]] GameCreationOptions options() const;
    [[nodiscard]] QString selectedGameTypesSettingString() const;
    void resetToDefaults();
    void setReadOnly(bool readOnly);
    void focusDescription();
    [[nodiscard]] bool isRememberSettingsChecked() const;
    void setRememberSettingsChecked(bool checked);

private slots:
    void spectatorsAllowedChanged(QT_STATE_CHANGED_T state);

private:
    bool registeredUser;
    QMap<int, QRadioButton *> gameTypeCheckBoxes;

    QLineEdit *descriptionEdit;
    QLineEdit *passwordEdit;
    QSpinBox *maxPlayersEdit;
    QSpinBox *startingLifeTotalEdit;
    QCheckBox *onlyBuddiesCheckBox;
    QCheckBox *onlyRegisteredCheckBox;
    QCheckBox *spectatorsAllowedCheckBox;
    QCheckBox *spectatorsNeedPasswordCheckBox;
    QCheckBox *spectatorsCanTalkCheckBox;
    QCheckBox *spectatorsSeeEverythingCheckBox;
    QCheckBox *createGameAsJudgeCheckBox;
    QCheckBox *createGameAsSpectatorCheckBox;
    QCheckBox *shareDecklistsOnLoadCheckBox;
    QCheckBox *shufflePlayersCheckBox;
    QCheckBox *rememberGameSettings;
};

#endif // CREATE_GAME_FORM_WIDGET_H
