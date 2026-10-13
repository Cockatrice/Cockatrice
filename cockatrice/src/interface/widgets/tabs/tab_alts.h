#ifndef TAB_ALTS_H
#define TAB_ALTS_H

#include "tab.h"

#include <QList>
#include <QString>
#include <QtGlobal>
#include <libcockatrice/protocol/pb/serverinfo_user_alt.pb.h>
#include <qtmetamacros.h>

class AbstractClient;
class AltTableDelegate;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTextEdit;
class Response;
class TabSupervisor;

/**
 * Staff alt-graph investigation tool. Shows accounts related to the
 * investigated user (shared email, client id, or recent IP address),
 * including transitive alts-of-alts, with per-row match evidence.
 * Selecting a row loads that account's warnings, bans, and admin notes.
 */
class TabAlts : public Tab
{
    Q_OBJECT
public:
    explicit TabAlts(TabSupervisor *_tabSupervisor, AbstractClient *_client, const QString &initialUser = {});
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("User Alts");
    }
    void investigate(const QString &userName);

private slots:
    void investigateUser();
    void goBack();
    void investigateSelected();
    void altsResponse(const Response &response);
    void onFilterChanged();
    void onAltSelected();
    void userDetailsResponse(const Response &response);
    void warnHistoryResponse(const Response &response);
    void banHistoryResponse(const Response &response);

private:
    void loadUser(const QString &userName);
    void updateTrail();
    void requestAlts(const QString &userName, int depth);
    void requestDetails(const QString &userName);
    void clearDetails();
    void renderTable();
    [[nodiscard]] bool rowVisible(const ServerInfo_UserAlt &alt) const;
    [[nodiscard]] QString formatEpoch(quint64 ts) const;

    AbstractClient *client;
    QString currentUser;
    QString selectedUser;
    QList<QString> backStack;
    QList<ServerInfo_UserAlt> lastAlts;

    QLineEdit *searchEdit;
    QSpinBox *depthSpin;
    QPushButton *investigateButton;
    QPushButton *backButton;
    QPushButton *investigateSelectedButton;
    QLabel *trailLabel;
    QLabel *statusLabel;
    QTableWidget *altsTable;
    AltTableDelegate *altsDelegate;

    QPushButton *emailChip;
    QPushButton *clientIdChip;
    QPushButton *ipChip;
    QLabel *vpnLabel;
    QSpinBox *vpnSpin;

    QLabel *detailNameValue;
    QLabel *detailCountsValue;
    QTextEdit *detailNotesEdit;
    QTableWidget *warnsTable;
    QTableWidget *bansTable;

    QGroupBox *detailsGroup;
    QLabel *detailNameLabel;
    QLabel *detailCountsLabel;
    QLabel *detailNotesLabel;
};

#endif // TAB_ALTS_H
