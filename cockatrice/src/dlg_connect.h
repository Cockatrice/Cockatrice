#ifndef DLG_CONNECT_H
#define DLG_CONNECT_H

#include "handle_public_servers.h"
#include "userconnection_information.h"
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;
class QRadioButton;

class DeleteHighlightedItemWhenShiftDelPressedEventFilter : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

class DlgConnect : public QDialog
{
    Q_OBJECT
signals:
    void sigStartForgotPasswordRequest();
    void sigPublicServersDownloaded();

public:
    explicit DlgConnect(QWidget *parent = nullptr);
    ~DlgConnect();
    QString getHost() const;
    int getPort() const
    {
        return portEdit->text().toInt();
    }
    QString getPlayerName() const
    {
        return playernameEdit->text();
    }
    QString getPassword() const
    {
        return passwordEdit->text();
    }

private slots:
    void actOk();
    void actCancel();
    void actSaveConfig();
    void passwordSaved(int state);
    void previousHostSelected(bool state);
    void newHostSelected(bool state);
    void actForgotPassword();
    void updateDisplayInfo(const QString &saveName);
    void preRebuildComboBoxList();
    void rebuildComboBoxList(int failure = -1);
    void downloadThePublicServers();

private:
    QGridLayout *newHostLayout, *connectionLayout, *buttons, *loginLayout, *grid;
    QHBoxLayout *newHolderLayout;
    QGroupBox *loginGroupBox, *btnGroupBox, *restrictionsGroupBox;
    QVBoxLayout *mainLayout;
    QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *saveLabel, *publicServersLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit, *saveEdit;
    QCheckBox *savePasswordCheckBox, *autoConnectCheckBox;
    QComboBox *previousHosts;
    QRadioButton *newHostButton, *previousHostButton;
    QPushButton *btnOk, *btnCancel, *btnForgotPassword, *btnRefreshServers;
    QMap<QString, std::pair<QString, UserConnection_Information>> savedHostList;
    HandlePublicServers *hps;
    const QString placeHolderText = tr("Downloading...");
};
#endif
