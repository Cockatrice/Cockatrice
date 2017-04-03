#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QGroupBox>
#include <QPushButton>
#include "dlg_connect.h"
#include "settingscache.h"
#include "userconnection_information.h"

#define PUBLIC_SERVERS_URL "https://github.com/Cockatrice/Cockatrice/wiki/Public-Servers"

DlgConnect::DlgConnect(QWidget *parent)
    : QDialog(parent)
{
    previousHostButton = new QRadioButton(tr("Known Hosts"), this);
    previousHosts = new QComboBox(this);
    previousHosts->installEventFilter(new DeleteHighlightedItemWhenShiftDelPressedEventFilter);

    rebuildComboBoxList();

    newHostButton = new QRadioButton(tr("New Host"), this);
    
    saveLabel = new QLabel(tr("Name:"));
    saveEdit = new QLineEdit(settingsCache->servers().getSaveName());
    saveLabel->setBuddy(saveEdit);

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settingsCache->servers().getHostname());
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(settingsCache->servers().getPort("4747"));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(settingsCache->servers().getPlayerName("Player"));
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit(settingsCache->servers().getPassword());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    savePasswordCheckBox = new QCheckBox(tr("&Save password"));
    savePasswordCheckBox->setChecked(settingsCache->servers().getSavePassword());

    autoConnectCheckBox = new QCheckBox(tr("A&uto connect"));
    autoConnectCheckBox->setToolTip(tr("Automatically connect to the most recent login when Cockatrice opens"));

    publicServersLabel = new QLabel(QString("(<a href=\"%1\">").arg(PUBLIC_SERVERS_URL)+tr("Public Servers")+QString("</a>)"));
    publicServersLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    publicServersLabel->setWordWrap(true);
    publicServersLabel->setTextFormat(Qt::RichText);
    publicServersLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    publicServersLabel->setOpenExternalLinks(true);
    publicServersLabel->setAlignment(Qt::AlignCenter);

    if (savePasswordCheckBox->isChecked())
    {
        autoConnectCheckBox->setChecked(settingsCache->servers().getAutoConnect());
        autoConnectCheckBox->setEnabled(true);
    }
    else
    {
        settingsCache->servers().setAutoConnect(0);
        autoConnectCheckBox->setChecked(0);
        autoConnectCheckBox->setEnabled(false);
    }

    connect(savePasswordCheckBox, SIGNAL(stateChanged(int)), this, SLOT(passwordSaved(int)));

    btnForgotPassword = new QPushButton(tr("Forgot password"));
    connect(btnForgotPassword, SIGNAL(released()), this, SLOT(actForgotPassword()));

    btnOk = new QPushButton(tr("Connect"));
    btnOk->setFixedWidth(100);
    connect(btnOk, SIGNAL(released()), this, SLOT(actOk()));

    btnCancel = new QPushButton(tr("Cancel"));
    btnCancel->setFixedWidth(100);
    connect(btnCancel, SIGNAL(released()), this, SLOT(actCancel()));

    QGridLayout *newHostLayout = new QGridLayout;
    newHostLayout->addWidget(newHostButton, 0, 1);
    newHostLayout->addWidget(publicServersLabel, 0, 2);

    QGridLayout *connectionLayout = new QGridLayout;
    connectionLayout->addWidget(previousHostButton, 0, 1);
    connectionLayout->addWidget(previousHosts, 1, 1);
    connectionLayout->addLayout(newHostLayout, 2, 1, 1, 2);
    connectionLayout->addWidget(saveLabel, 3, 0);
    connectionLayout->addWidget(saveEdit, 3, 1);
    connectionLayout->addWidget(hostLabel, 4, 0);
    connectionLayout->addWidget(hostEdit, 4, 1);
    connectionLayout->addWidget(portLabel, 5, 0);
    connectionLayout->addWidget(portEdit, 5, 1);
    connectionLayout->addWidget(autoConnectCheckBox, 6, 1);

    QGridLayout *buttons = new QGridLayout;
    buttons->addWidget(btnOk, 0, 0);
    buttons->addWidget(btnForgotPassword, 0, 1);
    buttons->addWidget(btnCancel, 0, 2);

    QGroupBox *restrictionsGroupBox = new QGroupBox(tr("Server"));
    restrictionsGroupBox->setLayout(connectionLayout);

    QGridLayout *loginLayout = new QGridLayout;
    loginLayout->addWidget(playernameLabel, 0, 0);
    loginLayout->addWidget(playernameEdit, 0, 1);
    loginLayout->addWidget(passwordLabel, 1, 0);
    loginLayout->addWidget(passwordEdit, 1, 1);
    loginLayout->addWidget(savePasswordCheckBox, 2, 1);

    QGroupBox *loginGroupBox = new QGroupBox(tr("Login"));
    loginGroupBox->setLayout(loginLayout);

    QGroupBox *btnGroupBox = new QGroupBox(tr(""));
    btnGroupBox->setLayout(buttons);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(restrictionsGroupBox, 0, 0);
    grid->addWidget(loginGroupBox, 1, 0);
    grid->addWidget(btnGroupBox, 2, 0);
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);

    connect(previousHostButton, SIGNAL(toggled(bool)), this, SLOT(previousHostSelected(bool)));
    connect(newHostButton, SIGNAL(toggled(bool)), this, SLOT(newHostSelected(bool)));

    previousHostButton->setChecked(true);

    connect(previousHosts, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updateDisplayInfo(const QString &)));

    playernameEdit->setFocus();
}

void DlgConnect::actSaveConfig()
{
    if (!settingsCache->servers().updateExistingServer(saveEdit->text(), hostEdit->text(), portEdit->text(), playernameEdit->text(), passwordEdit->text(), savePasswordCheckBox->isChecked()))
        settingsCache->servers().addNewServer(saveEdit->text(), hostEdit->text(), portEdit->text(), playernameEdit->text(), passwordEdit->text(), savePasswordCheckBox->isChecked());

    rebuildComboBoxList();
}

void DlgConnect::rebuildComboBoxList()
{
    previousHosts->clear();

    UserConnection_Information uci;
    savedHostList = uci.getServerInfo();

    if (savedHostList.size() == 1)
    {
        settingsCache->servers().addNewServer("Woogerworks", "cockatrice.woogerworks.com", "4747", "", "", false);
        settingsCache->servers().addNewServer("Chickatrice", "chickatrice.net", "4747", "", "", false);
        settingsCache->servers().addNewServer("cockatric.es", "cockatric.es", "4747", "", "", false);
        settingsCache->servers().addNewServer("Tetrarch", "mtg.tetrarch.co", "4747", "", "", false);
    }
    savedHostList = uci.getServerInfo();

    int i = 0;
    foreach (UserConnection_Information tmp, savedHostList)
    {
        QString saveName = tmp.getSaveName();
        if (saveName.size())
        {
            previousHosts->addItem(saveName);
            
            if (settingsCache->servers().getPrevioushostName() == saveName)
                previousHosts->setCurrentIndex(i);
            
            i++;
        }
    }
}


void DlgConnect::previousHostSelected(bool state) {
    if (state) {
        saveEdit->setDisabled(true);
        previousHosts->setDisabled(false);
    }
}

void DlgConnect::updateDisplayInfo(const QString &saveName)
{
    UserConnection_Information uci;
    QStringList data = uci.getServerInfo(saveName);

    if (saveEdit == nullptr)
        return;

    bool savePasswordStatus = data.at(5) == "1";
    
    saveEdit->setText(data.at(0));
    hostEdit->setText(data.at(1));
    portEdit->setText(data.at(2));
    playernameEdit->setText(data.at(3));
    savePasswordCheckBox->setChecked(savePasswordStatus);

    if (savePasswordStatus)
        passwordEdit->setText(data.at(4));
}

void DlgConnect::newHostSelected(bool state) {
    if (state)
    {
        previousHosts->setDisabled(true);
        hostEdit->clear();
        portEdit->clear();
        playernameEdit->clear();
        passwordEdit->clear();
        savePasswordCheckBox->setChecked(false);
        saveEdit->clear();
        saveEdit->setPlaceholderText("New Menu Name");
        saveEdit->setDisabled(false);
    }
    else
    {
        rebuildComboBoxList();
    }
}


void DlgConnect::passwordSaved(int state)
{
    Q_UNUSED(state);
    if(savePasswordCheckBox->isChecked()) {
       autoConnectCheckBox->setEnabled(true);
    } else {
        autoConnectCheckBox->setChecked(0);
        autoConnectCheckBox->setEnabled(false);
    }
}

void DlgConnect::actOk()
{
    if (newHostButton->isChecked())
    {
        if (saveEdit->text().isEmpty())
        {
            QMessageBox::critical(this, tr("Connection Warning"), tr("You need to name your new connection profile."));
            return;
        }

        settingsCache->servers().addNewServer(saveEdit->text(), hostEdit->text(), portEdit->text(), playernameEdit->text(), passwordEdit->text(), savePasswordCheckBox->isChecked());
    }
    else
    {
        settingsCache->servers().updateExistingServer(saveEdit->text(), hostEdit->text(), portEdit->text(), playernameEdit->text(), passwordEdit->text(), savePasswordCheckBox->isChecked());
    }
    settingsCache->servers().setPrevioushostName(saveEdit->text());
    settingsCache->servers().setAutoConnect(autoConnectCheckBox->isChecked() ? 1 : 0);

    if (playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Connect Warning"), tr("The player name can't be empty."));
        return;
    }

    accept();
}


QString DlgConnect::getHost() const {
    return hostEdit->text();
}

void DlgConnect::actCancel()
{
    settingsCache->servers().setSavePassword(savePasswordCheckBox->isChecked() ? 1 : 0);
    settingsCache->servers().setAutoConnect( autoConnectCheckBox->isChecked() ? 1 : 0);
    reject();
}

bool DeleteHighlightedItemWhenShiftDelPressedEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            QComboBox* combobox = reinterpret_cast<QComboBox *>(obj);
            combobox->removeItem(combobox->currentIndex());
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void DlgConnect::actForgotPassword()
{
    emit sigStartForgotPasswordRequest();
    reject();
}
