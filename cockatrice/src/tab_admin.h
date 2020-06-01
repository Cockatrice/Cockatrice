#ifndef TAB_ADMIN_H
#define TAB_ADMIN_H

#include "tab.h"

#include <QDialog>

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
    ShutdownDialog(QWidget *parent = nullptr);
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
    QPushButton *updateServerMessageButton, *shutdownServerButton, *reloadConfigButton;
    QGroupBox *adminGroupBox;
    QPushButton *unlockButton, *lockButton;
signals:
    void adminLockChanged(bool lock);
private slots:
    void actUpdateServerMessage();
    void actShutdownServer();
    void actReloadConfig();

    void actUnlock();
    void actLock();

public:
    TabAdmin(TabSupervisor *_tabSupervisor, AbstractClient *_client, bool _fullAdmin, QWidget *parent = nullptr);
    void retranslateUi();
    QString getTabText() const
    {
        return tr("Administration");
    }
    bool getLocked() const
    {
        return locked;
    }
};

#endif
