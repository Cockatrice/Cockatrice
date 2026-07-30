#ifndef DLG_REGISTER_H
#define DLG_REGISTER_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMap>

class HandlePublicServers;
class QLabel;
class QPushButton;
class QRadioButton;
class UserConnection_Information;

class DlgRegister : public QDialog
{
    Q_OBJECT
public:
    explicit DlgRegister(QWidget *parent = nullptr);
    ~DlgRegister() override;

    [[nodiscard]] QString getHost() const
    {
        return hostEdit->text();
    }
    [[nodiscard]] int getPort() const
    {
        return portEdit->text().toInt();
    }
    [[nodiscard]] QString getPlayerName() const
    {
        return playernameEdit->text();
    }
    [[nodiscard]] QString getPassword() const
    {
        return passwordEdit->text();
    }
    [[nodiscard]] QString getEmail() const
    {
        return emailEdit->text();
    }
    [[nodiscard]] QString getCountry() const
    {
        return countryEdit->currentIndex() == 0 ? "" : countryEdit->currentText();
    }
    [[nodiscard]] QString getRealName() const
    {
        return realnameEdit->text();
    }

public slots:
    void downloadThePublicServers();

private slots:
    void actOk();
    void previousHostSelected(bool state);
    void newHostSelected(bool state);
    void updateDisplayInfo(const QString &saveName);
    void preRebuildComboBoxList();
    void rebuildComboBoxList(int failure = -1);
    void actRemoveSavedServer();

private:
    QRadioButton *newHostButton;
    QRadioButton *previousHostButton;
    QComboBox *previousHosts;
    QPushButton *btnDeleteServer;
    QPushButton *btnRefreshServers;
    HandlePublicServers *hps;

    QLabel *infoLabel, *hostLabel, *portLabel, *playernameLabel, *passwordLabel, *passwordConfirmationLabel,
        *emailLabel, *emailConfirmationLabel, *countryLabel, *realnameLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit, *passwordConfirmationEdit, *emailEdit,
        *emailConfirmationEdit, *realnameEdit;
    QComboBox *countryEdit;

    QMap<QString, std::pair<QString, UserConnection_Information>> savedHostList;
    const QString placeHolderText = QStringLiteral("Downloading...");
};

#endif // DLG_REGISTER_H
