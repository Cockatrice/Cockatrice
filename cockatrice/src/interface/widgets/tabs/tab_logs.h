/**
 * @file tab_logs.h
 * @ingroup ServerTabs
 */
//! \todo Document this file.

#ifndef TAB_LOG_H
#define TAB_LOG_H

#include "tab.h"

#include <QString>
#include <qtmetamacros.h>

class AbstractClient;
class LineEditUnfocusable;
class QGroupBox;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QLabel;
class QDockWidget;
class QWidget;
class QGridLayout;
class QVBoxLayout;
class QTableWidget;
class Response;
class TabSupervisor;

class TabLog : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    bool canUseDeveloperCommands;
    QLabel *labelFindUserName, *labelFindIPAddress, *labelFindGameName, *labelFindGameID, *labelMessage, *labelMaximum,
        *labelDescription;
    LineEditUnfocusable *findUsername, *findIPAddress, *findGameName, *findGameID, *findMessage;
    QCheckBox *mainRoom, *gameRoom, *privateChat;
    QRadioButton *pastDays, *today, *lastHour;
    QSpinBox *maximumResults, *pastXDays;
    QDockWidget *searchDock;
    QWidget *searchDockContents;
    QPushButton *getButton, *clearButton;
    QGridLayout *criteriaGrid, *locationGrid, *rangeGrid, *maxResultsGrid, *descriptionGrid, *buttonGrid;
    QGroupBox *criteriaGroupBox, *locationGroupBox, *rangeGroupBox, *maxResultsGroupBox, *descriptionGroupBox,
        *buttonGroupBox;
    QVBoxLayout *mainLayout;
    QTableWidget *roomTable, *gameTable, *chatTable;

    int currentOffset = 0;
    int pageSize = 200;
    int roomCounter = 0, gameCounter = 0, chatCounter = 0;
    bool isFetching = false;
    bool hasMoreResults = false;

    void createDock();
    void fetchPage(int offset);
    void appendChatMessage(QTableWidget *table,
                           int &counter,
                           const QString &time,
                           const QString &senderName,
                           const QString &senderIp,
                           const QString &messageText,
                           const QString &targetId,
                           const QString &targetName,
                           const QString &channel = QString());
    void checkFetchMore();
signals:

private slots:
    void getClicked();
    void clearClicked();
    void viewLogHistory_processResponse(const Response &resp);
    void restartLayout();

public:
    TabLog(TabSupervisor *_tabSupervisor, AbstractClient *_client, bool _canUseDeveloperCommands = false);
    ~TabLog() override;
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Logs");
    }
};

#endif
