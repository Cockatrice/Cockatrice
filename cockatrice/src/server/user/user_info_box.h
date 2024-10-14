#ifndef USERINFOBOX_H
#define USERINFOBOX_H

#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class ServerInfo_User;
class AbstractClient;
class Response;

class UserInfoBox : public QWidget
{
    Q_OBJECT
private:
    AbstractClient *client;
    bool editable;
    QLabel avatarPic, userLevelIcon, nameLabel, realNameLabel1, realNameLabel2, countryLabel1, countryLabel2,
        countryLabel3, userLevelLabel1, userLevelLabel2, accountAgeLabel1, accountAgeLabel2;
    QPushButton editButton, passwordButton, avatarButton;
    QPixmap avatarPixmap;

    static QString getAgeString(int ageSeconds);

public:
    UserInfoBox(AbstractClient *_client, bool editable, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    void retranslateUi();

    inline static QPair<int, int> getDaysAndYearsBetween(const QDate &then, const QDate &now)
    {
        int years = now.addDays(1 - then.dayOfYear()).year() - then.year(); // there is no yearsTo
        int days = then.addYears(years).daysTo(now);
        return {days, years};
    }
private slots:
    void processResponse(const Response &r);
    void processEditResponse(const Response &r);
    void processPasswordResponse(const Response &r);
    void processAvatarResponse(const Response &r);

    void actEdit();
    void actEditInternal(const Response &r);
    void actPassword();
    void actAvatar();
public slots:
    void updateInfo(const ServerInfo_User &user);
    void updateInfo(const QString &userName);

private:
    void resizeEvent(QResizeEvent *event) override;
    void changePassword(const QString &oldPassword, const QString &newPassword);
};

#endif
