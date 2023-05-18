#ifndef DLG_CONNECT_H
#define DLG_CONNECT_H

#include "handle_public_servers.h"

#include <QDialog>

class QCheckBox;
class RemoteClient;
class QComboBox;
class QDialogButtonBox;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QVBoxLayout;

class DlgConnect : public QDialog
{
    Q_OBJECT
signals:
    void sigStartForgotPasswordRequest();

public:
    explicit DlgConnect(QWidget *parent, RemoteClient *_client);
    ~DlgConnect() override;

private slots:
    void actOk();

    void downloadThePublicServers();
    void passwordSaved(int state);
    // TODO
    // void previousHostSelected(bool state);
    // void newHostSelected(bool state);
    void actForgotPassword();
    void selectServer(int index);
    void updateDisplayInfo();
    void rebuildComboBoxList(int failure = -1);
    void addServer();
    void connected();
    // void registerError(Response::ResponseCode r, QString reasonStr, quint32 endTime);
    // void registerAccepted();
    // void registerAcceptedNeedsActivate();
    // void forgotPasswordSuccess();
    // void forgotPasswordError();
    // void promptForgotPasswordReset();
    // void promptForgotPasswordChallenge();

private:
    enum Mode {
      Login,
      Register,
      Restore
    };

    Mode mode;
    QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *passwordConfirmationLabel, *emailLabel,
        *saveLabel, *countryLabel, *realNameLabel, *serverIssuesLabel, *serverContactLabel, *serverContactLink;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit, *saveEdit, *passwordConfirmationEdit, *emailEdit,
        *emailConfirmationEdit, *realNameEdit;
    QComboBox *countryEdit;
    QCheckBox *savePasswordCheckBox, *autoConnectCheckBox;
    QComboBox *previousHosts;
    QRadioButton *newHostButton, *previousHostButton;
    QPushButton *btnConnect, *btnForgotPassword, *btnRefreshServers;
    QDialogButtonBox *buttonBox;

    RemoteClient *client;
    QString currentSaveName;
    HandlePublicServers hps;
    bool autoConnectEnabled, savePasswordEnabled, editServerEnabled;

    void setupUI();
    void updateSavePasswordEnabled();
    void updateEditServer();
};
#endif
