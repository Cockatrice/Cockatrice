#ifndef TAB_LOG_H
#define TAB_LOG_H

#include "tab.h"
#include <QDialog>

class AbstractClient;

class QGroupBox;
class QPushButton;
class QSpinBox;
class QLineEdit;
class QCheckBox;
class QRadioButton;
class QLabel;
class QDockWidget;
class QWidget;
class QGridLayout;
class QTableWidget;
class CommandContainer;
class Response;
class AbstractClient;

class TabLog : public Tab {
    Q_OBJECT
private:
    AbstractClient *client;
    QLabel *labelFindUserName, *labelFindIPAddress, *labelFindGameName, *labelFindGameID, *labelMessage, *labelMaximum, *labelDescription;
    QLineEdit *findUsername, *findIPAddress, *findGameName, *findGameID, *findMessage;
    QCheckBox *mainRoom, *gameRoom, *privateChat;
    QRadioButton *pastDays, *today, *lastHour;
    QSpinBox *maximumResults, *pastXDays;
    QDockWidget *searchDock;
    QWidget *searchDockContents;
    QPushButton *getButton, *clearButton;
    QGridLayout *criteriaGrid, *locationGrid, *rangeGrid, *maxResultsGrid, *descriptionGrid, *buttonGrid;
    QGroupBox *criteriaGroupBox, *locationGroupBox, *rangeGroupBox, *maxResultsGroupBox, *descriptionGroupBox, *buttonGroupBox;
    QVBoxLayout *mainLayout;
    QTableWidget *roomTable, *gameTable, *chatTable;

    void createDock();
signals:

private slots:
    void getClicked();
    void clearClicked();
    void viewLogHistory_processResponse(const Response &resp);
    void restartLayout();

public:
    TabLog(TabSupervisor *_tabSupervisor, AbstractClient *_client, QWidget *parent = 0);
    ~TabLog();
    void retranslateUi();
    QString getTabText() const { return tr("Logs"); }
};

#endif
