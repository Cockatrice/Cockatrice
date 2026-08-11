#ifndef TAB_MODERATION_H
#define TAB_MODERATION_H

#include "tab.h"

#include <libcockatrice/protocol/pb/response.pb.h>

class AbstractClient;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTextEdit;

/**
 * Staff investigation tool. Lets moderators look up a user's account data,
 * alternate accounts, login sessions, and staff login activity, and offers
 * the password-reset and remove-avatar actions.
 */
class TabModeration : public Tab
{
    Q_OBJECT
public:
    explicit TabModeration(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &initialUser = {});
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Moderation");
    }
    void investigate(const QString &userName);

private slots:
    void investigateUser();
    void userInfoResponse(const Response &response);
    void sessionsResponse(const Response &response);
    void altsResponse(const Response &response);
    void moderatorLoginsResponse(const Response &response);
    void resetPassword();
    void resetPasswordResponse(const Response &response);
    void removeAvatar();
    void removeAvatarResponse(const Response &response);

private:
    void requestUserInfo(const QString &userName);
    void requestSessions(const QString &userName);
    void requestAlts(const QString &userName);
    void requestModeratorLogins();
    void clearUserData();
    [[nodiscard]] QString formatEpoch(quint64 ts) const;

    AbstractClient *client;
    QString currentUser;

    QLineEdit *searchEdit;
    QPushButton *investigateButton;
    QPushButton *resetPasswordButton;
    QPushButton *removeAvatarButton;

    QGroupBox *userInfoGroup;
    QLabel *userInfoNameLabel;
    QLabel *userInfoNameValue;
    QLabel *userInfoRegisteredLabel;
    QLabel *userInfoRegisteredValue;
    QLabel *userInfoLastLoginLabel;
    QLabel *userInfoLastLoginValue;
    QLabel *userInfoStatusLabel;
    QLabel *userInfoStatusValue;
    QLabel *userInfoCountsLabel;
    QLabel *userInfoCountsValue;
    QLabel *userInfoNotesLabel;
    QTextEdit *userInfoNotesEdit;

    QGroupBox *altsGroup;
    QTableWidget *altsTable;
    QGroupBox *sessionsGroup;
    QTableWidget *sessionsTable;
    QGroupBox *staffGroup;
    QTableWidget *staffTable;
    QPushButton *refreshStaffButton;
};

#endif // TAB_MODERATION_H
