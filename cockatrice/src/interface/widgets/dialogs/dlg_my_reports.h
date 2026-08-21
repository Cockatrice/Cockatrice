#ifndef COCKATRICE_DLG_MY_REPORTS_H
#define COCKATRICE_DLG_MY_REPORTS_H

#include <QDialog>
#include <QList>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_report.pb.h>

class AbstractClient;
class QTableWidget;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;

class DlgMyReports : public QDialog
{
    Q_OBJECT
public:
    explicit DlgMyReports(AbstractClient *_client, QWidget *parent = nullptr);

private slots:
    void refreshList();
    void reportListResponse(const Response &response);
    void onSelectionChanged();
    void reportDetailsResponse(const Response &response);
    void addComment();
    void addCommentResponse(const Response &response);

private:
    void loadReportDetails(const ServerInfo_Report &report);
    void setActionsEnabled(bool enabled);

    AbstractClient *client;

    QTableWidget *table;
    QTextEdit *descriptionEdit;
    QTextEdit *chatLogEdit;
    QTextEdit *commentsEdit;
    QLineEdit *commentInput;
    QPushButton *commentButton;
    QPushButton *refreshButton;
    QPushButton *closeButton;
    QLabel *statusLabel;

    QList<ServerInfo_Report> currentReports;
    int selectedReportId;
    int selectedReportIdBeforeRefresh = -1;
    QString commentDraftBeforeRefresh;
};

#endif // COCKATRICE_DLG_MY_REPORTS_H
