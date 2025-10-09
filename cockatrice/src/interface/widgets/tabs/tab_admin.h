/**
 * @file tab_admin.h
 * @ingroup ServerTabs
 * @brief TODO: Document this.
 */

#ifndef TAB_ADMIN_H
#define TAB_ADMIN_H

#include "tab.h"

#include <QDialog>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>

class AbstractClient;

class QGroupBox;
class QPushButton;
class QSpinBox;
class QLineEdit;

class ShutdownDialog : public QDialog
{
    Q_OBJECT
private:
    QLineEdit *reasonEdit;
    QSpinBox *minutesEdit;

public:
    explicit ShutdownDialog(QWidget *parent = nullptr);
    QString getReason() const;
    int getMinutes() const;
};

class TabAdmin : public Tab
{
    Q_OBJECT
private:
    bool locked;
    AbstractClient *client;
    bool fullAdmin;
    QPushButton *updateServerMessageButton, *shutdownServerButton, *reloadConfigButton, *grantReplayAccessButton,
        *activateUserButton;
    QGroupBox *adminGroupBox, *moderatorGroupBox;
    QPushButton *unlockButton, *lockButton;
    QLineEdit *replayIdToGrant, *userToActivate;
signals:
    void adminLockChanged(bool lock);
private slots:
    void actUpdateServerMessage();
    void actShutdownServer();
    void actReloadConfig();
    void actGrantReplayAccess();
    void actForceActivateUser();
    void grantReplayAccessProcessResponse(const Response &response);
    void activateUserProcessResponse(const Response &response);

    void actUnlock();
    void actLock();

public:
    TabAdmin(TabSupervisor *_tabSupervisor, AbstractClient *_client, bool _fullAdmin);
    void retranslateUi() override;
    QString getTabText() const override
    {
        return tr("Administration");
    }
    bool getLocked() const
    {
        return locked;
    }
};

#endif
