/**
 * @file dlg_invite_to_game.h
 * @ingroup RoomDialogs
 */
//! \todo Document this file.

#ifndef DLG_INVITE_TO_GAME_H
#define DLG_INVITE_TO_GAME_H

#include <QDialog>
#include <QStringList>

class QLineEdit;
class QPushButton;
class TabSupervisor;
class UserListWidget;

class DlgInviteToGame : public QDialog
{
    Q_OBJECT
public:
    DlgInviteToGame(TabSupervisor *_tabSupervisor,
                    const QString &_inviteUrl,
                    bool _onlyBuddies,
                    const QStringList &_excludeUserNames,
                    QWidget *parent = nullptr);

private slots:
    void searchTextChanged(const QString &text);
    void inviteCurrentUser(const QString &userName);

private:
    TabSupervisor *tabSupervisor;
    QString inviteUrl;
    bool onlyBuddies;
    QStringList excludeUserNames;
    QString currentUserName;
    QLineEdit *searchEdit;
    UserListWidget *userList;
    QPushButton *inviteButton;
    QPushButton *cancelButton;

    void retranslateUi();
};

#endif
