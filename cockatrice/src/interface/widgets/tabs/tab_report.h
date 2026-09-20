#ifndef TAB_REPORT_H
#define TAB_REPORT_H

#include "tab.h"

#include <QList>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_report.pb.h>

class AbstractClient;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTableWidget;
class QTextEdit;
class QTimer;
class GameReplay;

class TabReport : public Tab
{
    Q_OBJECT
public:
    TabReport(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Report Queue");
    }

signals:
    void openReplay(GameReplay *replay);
    void requestJoinGame(int gameId, int roomId);

private slots:
    void refreshList();
    void reportListResponse(const Response &response);
    void assignReport();
    void assignResponse(const Response &response);
    void resolveReport(bool dismissed, bool promptNote);
    void resolveResponse(const Response &response);
    void onSelectionChanged();
    void viewReplay();
    void viewReplayResponse(const Response &response);
    void joinGame();
    void addComment();
    void addCommentResponse(const Response &response);
    void requestUserInfo(const QString &userName);
    void userInfoResponse(const Response &response);
    void reportDetailsResponse(const Response &response);
    void requestStats();
    void statsResponse(const Response &response);

private:
    int selectedReportId() const;
    bool selectedReportInfo(ServerInfo_Report &info) const;
    void setActionsEnabled(bool enabled);
    void updateActionStates();
    void applyFilters();
    void updateStats();
    void loadReportDetails(int reportId);

    AbstractClient *client;

    QLineEdit *searchEdit;
    QComboBox *statusFilter;
    QCheckBox *unresolvedOnlyBox;
    QPushButton *refreshButton;
    QLabel *statsLabel;
    QTableWidget *table;
    QSplitter *detailSplitter;
    QGroupBox *descGroup;
    QGroupBox *chatGroup;
    QGroupBox *commentsGroup;
    QTextEdit *descriptionEdit;
    QTextEdit *chatLogEdit;
    QTextEdit *commentsEdit;
    QLineEdit *commentInput;
    QPushButton *commentButton;
    QPushButton *assignButton;
    QPushButton *resolveButton;
    QPushButton *resolveWithNoteButton;
    QPushButton *dismissButton;
    QPushButton *viewReplayButton;
    QPushButton *joinGameButton;
    QLabel *statusLabel;
    QTimer *refreshTimer;

    QGroupBox *userContextGroup;
    QLabel *userContextName;
    QLabel *userContextAccountAge;
    QLabel *userContextReports;
    QLabel *userContextBans;
    QLabel *userContextWarns;
    QTextEdit *userContextNotes;
    QTextEdit *userContextRecentReports;
    QLabel *userContextUserLabel;
    QLabel *userContextAgeLabel;
    QLabel *userContextReportsLabel;
    QLabel *userContextBansLabel;
    QLabel *userContextWarnsLabel;
    QLabel *userContextNotesLabel;
    QLabel *userContextRecentReportsLabel;
    QString lastRequestedUser;
    QGroupBox *statsGroup;
    QLabel *statsTotalLabel;
    QLabel *statsTrendLabel;
    QLabel *statsCategoriesLabel;
    QTextEdit *statsDetailText;

    QList<ServerInfo_Report> allReports;
    QList<ServerInfo_Report> filteredReports;
    int detailsRequestedReportId = -1;
    int detailsRequestSeq = 0;
    int selectedReportIdBeforeRefresh = -1;
    QString commentDraftBeforeRefresh;
    ServerInfo_Report previousSelectedReport;
    bool previousSelectedReportValid = false;
};

#endif // TAB_REPORT_H
