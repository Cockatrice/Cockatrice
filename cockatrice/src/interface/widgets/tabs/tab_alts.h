#ifndef TAB_ALTS_H
#define TAB_ALTS_H

#include "tab.h"

#include <QString>
#include <QtGlobal>
#include <qtmetamacros.h>

class AbstractClient;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class Response;
class TabSupervisor;

/**
 * Staff alt-graph investigation tool. Shows accounts related to the
 * investigated user (shared email, client id, or recent IP address),
 * including transitive alts-of-alts, with per-row match evidence.
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
    void altsResponse(const Response &response);

private:
    void loadUser(const QString &userName);
    void requestAlts(const QString &userName, int depth);
    [[nodiscard]] QString formatEpoch(quint64 ts) const;

    AbstractClient *client;
    QString currentUser;
    QList<QString> backStack;

    QLineEdit *searchEdit;
    QSpinBox *depthSpin;
    QPushButton *investigateButton;
    QPushButton *backButton;
    QLabel *statusLabel;
    QTableWidget *altsTable;
};

#endif // TAB_ALTS_H
