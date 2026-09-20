#ifndef COCKATRICE_USER_LIST_DIALOG_H
#define COCKATRICE_USER_LIST_DIALOG_H

#include <QDialog>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QRadioButton;
class QSpinBox;
class QLineEdit;
class QCheckBox;

class BanDialog : public QDialog
{
    Q_OBJECT

    QLabel *daysLabel, *hoursLabel, *minutesLabel;
    QCheckBox *nameBanCheckBox, *ipBanCheckBox, *idBanCheckBox, *deleteMessages;
    QLineEdit *nameBanEdit, *ipBanEdit, *idBanEdit;
    QSpinBox *daysEdit, *hoursEdit, *minutesEdit;
    QRadioButton *permanentRadio, *temporaryRadio;
    QPlainTextEdit *reasonEdit, *visibleReasonEdit;

private slots:
    void okClicked();
    void enableTemporaryEdits(bool enabled);

public:
    explicit BanDialog(const ServerInfo_User &info, QWidget *parent = nullptr);
    [[nodiscard]] QString getBanName() const;
    [[nodiscard]] QString getBanIP() const;
    [[nodiscard]] QString getBanId() const;
    [[nodiscard]] int getMinutes() const;
    [[nodiscard]] QString getReason() const;
    [[nodiscard]] QString getVisibleReason() const;
    [[nodiscard]] int getDeleteMessages() const;
};

class WarningDialog : public QDialog
{
    Q_OBJECT

    QLabel *descriptionLabel;
    QLineEdit *nameWarning;
    QComboBox *warningOption;
    QLineEdit *warnClientID;
    QCheckBox *deleteMessages;

private slots:
    void okClicked();

public:
    WarningDialog(const QString &userName, const QString &clientID, QWidget *parent = nullptr);
    [[nodiscard]] QString getName() const;
    [[nodiscard]] QString getWarnID() const;
    [[nodiscard]] QString getReason() const;
    [[nodiscard]] int getDeleteMessages() const;
    void addWarningOption(const QString &warning, int startingIl = 1);
};

class AdminNotesDialog : public QDialog
{
    Q_OBJECT

    QString userName;
    QPlainTextEdit *notes;

public:
    explicit AdminNotesDialog(const QString &_userName, const QString &_notes, QWidget *_parent = nullptr);
    [[nodiscard]] QString getName() const
    {
        return userName;
    }
    [[nodiscard]] QString getNotes() const;
};

#endif // COCKATRICE_USER_LIST_DIALOG_H
