#ifndef TAB_MODERATION_H
#define TAB_MODERATION_H

#include "tab.h"

#include <QString>
#include <QtGlobal>
#include <libcockatrice/protocol/pb/response_report_user_info.pb.h>
#include <qtmetamacros.h>

class AbstractClient;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTabWidget;
class QTextEdit;
class Response;
class TabSupervisor;

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
    void openAltsGraph();
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
    void applyUserInfo();
    void applyAltsCount();
    [[nodiscard]] QString formatEpoch(quint64 ts) const;

    AbstractClient *client;
    QString currentUser;
    Response_ReportUserInfo cachedUserInfo;
    bool cachedUserInfoValid = false;
    int cachedAltsCount = -1;

    QLineEdit *searchEdit;
    QPushButton *investigateButton;
    QPushButton *resetPasswordButton;
    QPushButton *removeAvatarButton;

    QGroupBox *userInfoGroup;
    QLabel *userInfoNameValue;
    QLabel *userInfoStatusValue;
    QLabel *sublineLabel;
    QGroupBox *cardReports;
    QGroupBox *cardWarns;
    QGroupBox *cardBans;
    QGroupBox *altsCard;
    QLabel *cardReportsValue;
    QLabel *cardWarnsValue;
    QLabel *cardBansValue;
    QLabel *altsCountValue;
    QLabel *userInfoNotesLabel;
    QTextEdit *userInfoNotesEdit;

    QTabWidget *sectionTabs;
    QTableWidget *altsTable;
    QTableWidget *sessionsTable;
    QTableWidget *staffTable;
    QPushButton *openAltsButton;
    QPushButton *refreshStaffButton;
};

#endif // TAB_MODERATION_H
