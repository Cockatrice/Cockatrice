/**
 * @file dlg_create_game.h
 * @ingroup RoomDialogs
 */

#ifndef DLG_CREATEGAME_H
#define DLG_CREATEGAME_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <qtmetamacros.h>

class CreateGameFormWidget;
class QDialogButtonBox;
class QLabel;
class QPushButton;
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

private:
    TabRoom *room;
    CreateGameFormWidget *form = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
    QPushButton *clearButton = nullptr;
    QLabel *errorLabel = nullptr;

    void showError(const QString &message);
    void clearError();
};

#endif // DLG_CREATEGAME_H
