#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include "tab_logs.h"
#include "abstractclient.h"
#include "window_sets.h"
#include "pending_command.h"
#include "pb/moderator_commands.pb.h"
#include "pb/response_viewlog_history.pb.h"

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

TabLog::TabLog(TabSupervisor *_tabSupervisor, AbstractClient *_client, QWidget *parent)
    : Tab(_tabSupervisor, parent), client(_client)
{
    MainWindow = new QMainWindow;
    createDock();
    restartLayout();
    clearClicked();
    retranslateUi();
}

TabLog::~TabLog()
{

}

void TabLog::retranslateUi()
{

}

void TabLog::getClicked()
{
    if (findUsername->text().isEmpty() && findIPAddress->text().isEmpty() && findGameName->text().isEmpty() && findGameID->text().isEmpty() && findMessage->text().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("You must select at least one filter."));
        return;
    }

    if (!lastHour->isChecked() && !today->isChecked() && !pastDays->isChecked()){
        pastDays->setChecked(true);
        pastXDays->setValue(20);
    }

    if (pastDays->isChecked() && pastXDays->value() == 0) {
        QMessageBox::critical(this, tr("Error"), tr("You have to select a valid number of days to locate."));
        return;
    }

    if (!mainRoom->isChecked() && !gameRoom->isChecked() && !privateChat->isChecked()) {
        mainRoom->setChecked(true);
        gameRoom->setChecked(true);
        privateChat->setChecked(true);
    }

    if (maximumResults->value() == 0)
        maximumResults->setValue(1000);

    int dateRange;
    if (lastHour->isChecked())
        dateRange = 1;

    if (today->isChecked())
        dateRange = 24;

    if (pastDays->isChecked())
        dateRange = pastXDays->value() * 24;

    Command_ViewLogHistory cmd;
    cmd.set_user_name(findUsername->text().toStdString());
    cmd.set_ip_address(findIPAddress->text().toStdString());
    cmd.set_game_name(findGameName->text().toStdString());
    cmd.set_game_id(findGameID->text().toStdString());
    cmd.set_message(findMessage->text().toStdString());
    if (mainRoom->isChecked()) { cmd.add_log_location("room"); };
    if (gameRoom->isChecked()) { cmd.add_log_location("game"); };
    if (privateChat->isChecked()) { cmd.add_log_location("chat"); };
    cmd.set_date_range(dateRange);
    cmd.set_maximum_results(maximumResults->value());
    PendingCommand *pend = client->prepareModeratorCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(viewLogHistory_processResponse(Response)));
    client->sendCommand(pend);
}

void TabLog::clearClicked()
{
    findUsername->clear();
    findIPAddress->clear();
    findGameName->clear();
    findGameID->clear();
    findMessage->clear();
    pastXDays->clear();
    maximumResults->clear();
    mainRoom->setChecked(false);
    gameRoom->setChecked(false);
    privateChat->setChecked(false);
    pastDays->setAutoExclusive(false);
    pastDays->setChecked(false);
    today->setAutoExclusive(false);
    today->setChecked(false);
    lastHour->setAutoExclusive(false);
    lastHour->setChecked(false);
    pastDays->setAutoExclusive(true);
    today->setAutoExclusive(true);
    lastHour->setAutoExclusive(true);
}

void TabLog::createDock()
{

    labelFindUserName = new QLabel(tr("Username: "));
    findUsername = new QLineEdit("");
    findUsername->setAlignment(Qt::AlignCenter);
    labelFindIPAddress = new QLabel(tr("IP Address: "));
    findIPAddress = new QLineEdit("");
    findIPAddress->setAlignment(Qt::AlignCenter);
    labelFindGameName = new QLabel(tr("Game Name: "));
    findGameName = new QLineEdit("");
    findGameName->setAlignment(Qt::AlignCenter);
    labelFindGameID = new QLabel(tr("GameID: "));
    findGameID = new QLineEdit("");
    findGameID->setAlignment(Qt::AlignCenter);
    labelMessage = new QLabel(tr("Message: "));
    findMessage = new QLineEdit("");
    findMessage->setAlignment(Qt::AlignCenter);

    mainRoom = new QCheckBox(tr("Main Room"));
    gameRoom = new QCheckBox(tr("Game Room"));
    privateChat = new QCheckBox(tr("Private Chat"));

    pastDays = new QRadioButton(tr("Past X Days: "));
    today = new QRadioButton(tr("Today"));
    lastHour = new QRadioButton(tr("Last Hour"));
    pastXDays = new QSpinBox;
    pastXDays->setMaximum(20);


    labelMaximum = new QLabel(tr("Maximum Results: "));
    maximumResults = new QSpinBox;
    maximumResults->setMaximum(1000);

    labelDescription = new QLabel(tr("At least one filter is required.\nThe more information you put in, the more specific your results will be."));

    getButton = new QPushButton(tr("Get User Logs"));
    getButton->setAutoDefault(true);
    connect(getButton, SIGNAL(clicked()), this, SLOT(getClicked()));

    clearButton = new QPushButton(tr("Clear Filters"));
    clearButton->setAutoDefault(true);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearClicked()));

    criteriaGrid = new QGridLayout;
    criteriaGrid->addWidget(labelFindUserName, 0, 0);
    criteriaGrid->addWidget(findUsername, 0, 1);
    criteriaGrid->addWidget(labelFindIPAddress, 1, 0);
    criteriaGrid->addWidget(findIPAddress, 1, 1);
    criteriaGrid->addWidget(labelFindGameName, 2, 0);
    criteriaGrid->addWidget(findGameName, 2, 1);
    criteriaGrid->addWidget(labelFindGameID, 3, 0);
    criteriaGrid->addWidget(findGameID, 3, 1);
    criteriaGrid->addWidget(labelMessage, 4, 0);
    criteriaGrid->addWidget(findMessage, 4, 1);

    criteriaGroupBox = new QGroupBox(tr("Filters"));
    criteriaGroupBox->setLayout(criteriaGrid);
    criteriaGroupBox->setFixedSize(500,300);

    locationGrid = new QGridLayout;
    locationGrid->addWidget(mainRoom, 0, 0);
    locationGrid->addWidget(gameRoom, 0, 1);
    locationGrid->addWidget(privateChat, 0, 2);

    locationGroupBox = new QGroupBox(tr("Log Locations"));
    locationGroupBox->setLayout(locationGrid);

    rangeGrid = new QGridLayout;
    rangeGrid->addWidget(pastDays, 0, 0);
    rangeGrid->addWidget(pastXDays, 0, 1);
    rangeGrid->addWidget(today, 0, 2);
    rangeGrid->addWidget(lastHour, 0, 3);

    rangeGroupBox = new QGroupBox(tr("Date Range"));
    rangeGroupBox->setLayout(rangeGrid);

    maxResultsGrid = new QGridLayout;
    maxResultsGrid->addWidget(labelMaximum, 0, 0);
    maxResultsGrid->addWidget(maximumResults, 0, 1);

    maxResultsGroupBox = new QGroupBox(tr("Maximum Results"));
    maxResultsGroupBox->setLayout(maxResultsGrid);

    descriptionGrid = new QGridLayout;
    descriptionGrid->addWidget(labelDescription, 0, 0);

    descriptionGroupBox = new QGroupBox(tr(""));
    descriptionGroupBox->setLayout(descriptionGrid);

    buttonGrid = new QGridLayout;
    buttonGrid->addWidget(getButton, 0, 0);
    buttonGrid->addWidget(clearButton, 0, 1);

    buttonGroupBox = new QGroupBox(tr(""));
    buttonGroupBox->setLayout(buttonGrid);

    mainLayout = new QVBoxLayout(MainWindow);
    mainLayout->addWidget(criteriaGroupBox);
    mainLayout->addWidget(locationGroupBox);
    mainLayout->addWidget(rangeGroupBox);
    mainLayout->addWidget(maxResultsGroupBox);
    mainLayout->addWidget(descriptionGroupBox);
    mainLayout->addWidget(buttonGroupBox);
    mainLayout->setAlignment(Qt::AlignCenter);

    searchDockContents = new QWidget(MainWindow);
    searchDockContents->setLayout(mainLayout);

    searchDock = new QDockWidget(MainWindow);
    searchDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    searchDock->setWidget(searchDockContents);

    QVBoxLayout *mainVLayoutContent = new QVBoxLayout;
    QHBoxLayout *mainHLayoutContent = new QHBoxLayout;
    mainHLayoutContent->addWidget(MainWindow);
    mainHLayoutContent->addLayout(mainVLayoutContent);
    setLayout(mainHLayoutContent);
}

void TabLog::viewLogHistory_processResponse(const Response &resp)
{
    const Response_ViewLogHistory &response = resp.GetExtension(Response_ViewLogHistory::ext);
    if (resp.response_code() == Response::RespOk) {

        if (response.log_message_size() > 0) {

            int j = 0;
            QTableWidget *roomTable = new QTableWidget();
            roomTable->setWindowTitle(tr("Room Logs"));
            roomTable->setRowCount(response.log_message_size());
            roomTable->setColumnCount(6);
            roomTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            roomTable->setHorizontalHeaderLabels(QString(tr("Time;SenderName;SenderIP;Message;TargetID;TargetName")).split(";"));

            int k = 0;
            QTableWidget *gameTable = new QTableWidget();
            gameTable->setWindowTitle(tr("Game Logs"));
            gameTable->setRowCount(response.log_message_size());
            gameTable->setColumnCount(6);
            gameTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            gameTable->setHorizontalHeaderLabels(QString(tr("Time;SenderName;SenderIP;Message;TargetID;TargetName")).split(";"));

            int l = 0;
            QTableWidget *chatTable = new QTableWidget();
            chatTable->setWindowTitle(tr("Chat Logs"));
            chatTable->setRowCount(response.log_message_size());
            chatTable->setColumnCount(6);
            chatTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            chatTable->setHorizontalHeaderLabels(QString(tr("Time;SenderName;SenderIP;Message;TargetID;TargetName")).split(";"));

            ServerInfo_ChatMessage message; for (int i = 0; i < response.log_message_size(); ++i) {
                message = response.log_message(i);
                if (QString::fromStdString(message.target_type()) == "room") {
                    roomTable->setItem(j, 0, new QTableWidgetItem(QString::fromStdString(message.time())));
                    roomTable->setItem(j, 1, new QTableWidgetItem(QString::fromStdString(message.sender_name())));
                    roomTable->setItem(j, 2, new QTableWidgetItem(QString::fromStdString(message.sender_ip())));
                    roomTable->setItem(j, 3, new QTableWidgetItem(QString::fromStdString(message.message())));
                    roomTable->setItem(j, 4, new QTableWidgetItem(QString::fromStdString(message.target_id())));
                    roomTable->setItem(j, 5, new QTableWidgetItem(QString::fromStdString(message.target_name())));
                    ++j;
                }

                if (QString::fromStdString(message.target_type()) == "game") {
                    gameTable->setItem(k, 0, new QTableWidgetItem(QString::fromStdString(message.time())));
                    gameTable->setItem(k, 1, new QTableWidgetItem(QString::fromStdString(message.sender_name())));
                    gameTable->setItem(k, 2, new QTableWidgetItem(QString::fromStdString(message.sender_ip())));
                    gameTable->setItem(k, 3, new QTableWidgetItem(QString::fromStdString(message.message())));
                    gameTable->setItem(k, 4, new QTableWidgetItem(QString::fromStdString(message.target_id())));
                    gameTable->setItem(k, 5, new QTableWidgetItem(QString::fromStdString(message.target_name())));
                    ++k;
                }

                if (QString::fromStdString(message.target_type()) == "chat") {
                    chatTable->setItem(l, 0, new QTableWidgetItem(QString::fromStdString(message.time())));
                    chatTable->setItem(l, 1, new QTableWidgetItem(QString::fromStdString(message.sender_name())));
                    chatTable->setItem(l, 2, new QTableWidgetItem(QString::fromStdString(message.sender_ip())));
                    chatTable->setItem(l, 3, new QTableWidgetItem(QString::fromStdString(message.message())));
                    chatTable->setItem(l, 4, new QTableWidgetItem(QString::fromStdString(message.target_id())));
                    chatTable->setItem(l, 5, new QTableWidgetItem(QString::fromStdString(message.target_name())));
                    ++l;
                }
            }

            roomTable->setRowCount(j);
            roomTable->resizeColumnsToContents();
            gameTable->setRowCount(k);
            gameTable->resizeColumnsToContents();
            chatTable->setRowCount(l);
            chatTable->resizeColumnsToContents();

            if (mainRoom->isChecked()) {
                roomTable->resize(600, 200);
                roomTable->show();
            }

            if (gameRoom->isChecked()) {
                gameTable->resize(600, 200);
                gameTable->show();
            }

            if (privateChat->isChecked()) {
                chatTable->resize(600, 200);
                chatTable->show();
            }

        } else
            QMessageBox::information(static_cast<QWidget *>(parent()), tr("Message History"), tr("There is no messages for the selected iilters."));

    } else
        QMessageBox::critical(static_cast<QWidget *>(parent()), tr("Message History"), tr("Failed to collecting message history information."));
}

void TabLog::restartLayout()
{
    searchDock->setFloating(false);
    MainWindow->addDockWidget(Qt::TopDockWidgetArea, searchDock);
    searchDock->setVisible(true);
}