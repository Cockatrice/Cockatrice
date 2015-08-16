#include <QSettings>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <iostream>
#include "dlg_connect.h"

DlgConnect::DlgConnect(QWidget *parent)
    : QDialog(parent)
{
    QSettings settings;
    settings.beginGroup("server");

    previousHostButton = new QRadioButton(tr("Previous Host"), this);

    previousHosts = new QComboBox(this);
    previousHosts->installEventFilter(new DeleteHighlightedItemWhenShiftDelPressedEventFilter);
    QStringList previousHostList = settings.value("previoushosts").toStringList();
    if (previousHostList.isEmpty()) {
        previousHostList << "cockatrice.woogerworks.com";
        previousHostList << "vps.poixen.com";
        previousHostList << "chickatrice.net";
    }
    previousHosts->addItems(previousHostList);
    previousHosts->setCurrentIndex(settings.value("previoushostindex").toInt());

    newHostButton = new QRadioButton(tr("New Host"), this);
    
    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit();
    hostEdit->setPlaceholderText(tr("Enter host name"));
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(settings.value("port", "4747").toString());
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(settings.value("playername", "Player").toString());
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit(settings.value("password").toString());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    savePasswordCheckBox = new QCheckBox(tr("&Save password"));
    savePasswordCheckBox->setChecked(settings.value("save_password", 1).toInt());

    autoConnectCheckBox = new QCheckBox(tr("A&uto connect at start"));
    if(savePasswordCheckBox->isChecked())
    {
        autoConnectCheckBox->setChecked(settings.value("auto_connect", 0).toInt());
        autoConnectCheckBox->setEnabled(true);
    } else {
        settings.setValue("auto_connect", 0);
        autoConnectCheckBox->setChecked(0);
        autoConnectCheckBox->setEnabled(false);
    }

    connect(savePasswordCheckBox, SIGNAL(stateChanged(int)), this, SLOT(passwordSaved(int)));

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(previousHostButton, 0, 1);
    grid->addWidget(previousHosts, 1, 1);
    grid->addWidget(newHostButton, 2, 1);
    grid->addWidget(hostLabel, 3, 0);
    grid->addWidget(hostEdit, 3, 1);
    grid->addWidget(portLabel, 4, 0);
    grid->addWidget(portEdit, 4, 1);
    grid->addWidget(playernameLabel, 5, 0);
    grid->addWidget(playernameEdit, 5, 1);
    grid->addWidget(passwordLabel, 6, 0);
    grid->addWidget(passwordEdit, 6, 1);
    grid->addWidget(savePasswordCheckBox, 7, 0, 1, 2);
    grid->addWidget(autoConnectCheckBox, 8, 0, 1, 2);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);

    connect(previousHostButton, SIGNAL(toggled(bool)), this, SLOT(previousHostSelected(bool)));
    connect(newHostButton, SIGNAL(toggled(bool)), this, SLOT(newHostSelected(bool)));

    if (settings.value("previoushostlogin", 1).toInt())
        previousHostButton->setChecked(true);
    else
        newHostButton->setChecked(true);
}


void DlgConnect::previousHostSelected(bool state) {
    if (state) {
        hostEdit->setDisabled(true);
        previousHosts->setDisabled(false);
    }
}

void DlgConnect::newHostSelected(bool state) {
    if (state) {
        hostEdit->setDisabled(false);
        previousHosts->setDisabled(true);
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
    QSettings settings;
    settings.beginGroup("server");
    settings.setValue("port", portEdit->text());
    settings.setValue("playername", playernameEdit->text());
    settings.setValue("password", savePasswordCheckBox->isChecked() ? passwordEdit->text() : QString());
    settings.setValue("save_password", savePasswordCheckBox->isChecked() ? 1 : 0);
    settings.setValue("auto_connect", autoConnectCheckBox->isChecked() ? 1 : 0);
    settings.setValue("previoushostlogin", previousHostButton->isChecked() ? 1 : 0);
    
    QStringList hostList;
    if (newHostButton->isChecked())
        if (!hostEdit->text().trimmed().isEmpty())
            hostList << hostEdit->text();
    
    for (int i = 0; i < previousHosts->count(); i++)
        if(!previousHosts->itemText(i).trimmed().isEmpty())
            hostList << previousHosts->itemText(i);
    
    settings.setValue("previoushosts", hostList);
    settings.setValue("previoushostindex", previousHosts->currentIndex());
    settings.endGroup();

    accept();
}


QString DlgConnect::getHost() const {
    return previousHostButton->isChecked() ? previousHosts->currentText() : hostEdit->text();
}

void DlgConnect::actCancel()
{
    QSettings settings;
    settings.beginGroup("server");
    settings.setValue("save_password", savePasswordCheckBox->isChecked() ? 1 : 0);
    settings.setValue("auto_connect", autoConnectCheckBox->isChecked() ? 1 : 0);
    settings.endGroup();
    
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
