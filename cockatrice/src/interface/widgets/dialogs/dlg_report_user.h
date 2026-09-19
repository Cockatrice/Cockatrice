#ifndef COCKATRICE_DLG_REPORT_USER_H
#define COCKATRICE_DLG_REPORT_USER_H

#include <QDialog>
#include <libcockatrice/protocol/pb/response.pb.h>

class AbstractClient;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QTextEdit;
class QLabel;

class DlgReportUser : public QDialog
{
    Q_OBJECT
public:
    DlgReportUser(AbstractClient *_client,
                  const QString &_reportedUser,
                  int _gameId = -1,
                  const QString &_autoChatLog = QString(),
                  QWidget *parent = nullptr);

private slots:
    void actSubmit();
    void reportResponse(const Response &response);

private:
    AbstractClient *client;
    QString reportedUser;
    int gameId;

    QLabel *reportedUserLabel;
    QLabel *gameIdLabel = nullptr;
    QLineEdit *gameIdEdit = nullptr;
    QComboBox *categoryBox;
    QTextEdit *descriptionEdit;
    QTextEdit *chatLogEdit;
    QDialogButtonBox *buttonBox;
};

#endif // COCKATRICE_DLG_REPORT_USER_H
