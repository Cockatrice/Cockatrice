#ifndef DLG_CONNECT_H
#define DLG_CONNECT_H

#include "userconnection_information.h"
#include <QDialog>
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
    void rebuildComboBoxList();
    void actFinishParsingDownloadedData();
    void downloadPublicServers();

private:
    QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *saveLabel, *publicServersLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit, *saveEdit;
    QCheckBox *savePasswordCheckBox, *autoConnectCheckBox;
    QComboBox *previousHosts;
    QRadioButton *newHostButton, *previousHostButton;
    QPushButton *btnOk, *btnCancel, *btnForgotPassword, *btnRefreshServers;
    QMap<QString, UserConnection_Information> savedHostList;
    const QString placeHolderText = tr("Downloading...");
};
#endif
