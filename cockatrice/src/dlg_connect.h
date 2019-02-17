#ifndef DLG_CONNECT_H
#define DLG_CONNECT_H

#include "handle_public_servers.h"
#include "userconnection_information.h"
#include <QDialog>
#include <QLineEdit>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QRadioButton;
class QVBoxLayout;

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
    ~DlgConnect() override;
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

public slots:
    void downloadThePublicServers();

private slots:
    void actOk();
    void actSaveConfig();
    void passwordSaved(int state);
    void previousHostSelected(bool state);
    void newHostSelected(bool state);
    void actForgotPassword();
    void updateDisplayInfo(const QString &saveName);
    void preRebuildComboBoxList();
    void rebuildComboBoxList(int failure = -1);

private:
    QGridLayout *newHostLayout, *connectionLayout, *loginLayout, *serverInfoLayout, *grid;
    QHBoxLayout *newHolderLayout;
    QGroupBox *loginGroupBox, *serverInfoGroupBox, *btnGroupBox, *restrictionsGroupBox;
    QVBoxLayout *mainLayout;
    QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *saveLabel, *serverIssuesLabel,
        *serverContactLabel, *serverContactLink;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit, *saveEdit;
    QCheckBox *savePasswordCheckBox, *autoConnectCheckBox;
    QComboBox *previousHosts;
    QRadioButton *newHostButton, *previousHostButton;
    QPushButton *btnConnect, *btnForgotPassword, *btnRefreshServers;
    QMap<QString, std::pair<QString, UserConnection_Information>> savedHostList;
    HandlePublicServers *hps;
    const QString placeHolderText = tr("Downloading...");
};
#endif
