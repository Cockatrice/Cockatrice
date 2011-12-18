#include "userinfobox.h"
#include "protocol_datastructures.h"
#include "pixmapgenerator.h"
#include "protocol_items.h"
#include "abstractclient.h"
#include <QLabel>
#include <QGridLayout>

#include "pending_command.h"
#include "pb/session_commands.pb.h"

UserInfoBox::UserInfoBox(AbstractClient *_client, bool _fullInfo, QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags), client(_client), fullInfo(_fullInfo)
{
	avatarLabel = new QLabel;
	nameLabel = new QLabel;
	QFont nameFont = nameLabel->font();
	nameFont.setBold(true);
	nameFont.setPointSize(nameFont.pointSize() * 1.5);
	nameLabel->setFont(nameFont);
	realNameLabel1 = new QLabel;
	realNameLabel2 = new QLabel;
	genderLabel1 = new QLabel;
	genderLabel2 = new QLabel;
	countryLabel1 = new QLabel;
	countryLabel2 = new QLabel;
	userLevelLabel1 = new QLabel;
	userLevelLabel2 = new QLabel;
	userLevelLabel3 = new QLabel;
	
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(avatarLabel, 0, 0, 1, 3, Qt::AlignCenter);
	mainLayout->addWidget(nameLabel, 1, 0, 1, 3);
	mainLayout->addWidget(realNameLabel1, 2, 0, 1, 1);
	mainLayout->addWidget(realNameLabel2, 2, 1, 1, 2);
	mainLayout->addWidget(genderLabel1, 3, 0, 1, 1);
	mainLayout->addWidget(genderLabel2, 3, 1, 1, 2);
	mainLayout->addWidget(countryLabel1, 4, 0, 1, 1);
	mainLayout->addWidget(countryLabel2, 4, 1, 1, 2);
	mainLayout->addWidget(userLevelLabel1, 5, 0, 1, 1);
	mainLayout->addWidget(userLevelLabel2, 5, 1, 1, 1);
	mainLayout->addWidget(userLevelLabel3, 5, 2, 1, 1);
	mainLayout->setColumnStretch(2, 10);
	
	setWindowTitle(tr("User information"));
	setLayout(mainLayout);
	retranslateUi();
}

void UserInfoBox::retranslateUi()
{
	realNameLabel1->setText(tr("Real name:"));
	genderLabel1->setText(tr("Gender:"));
	countryLabel1->setText(tr("Location:"));
	userLevelLabel1->setText(tr("User level:"));
}

void UserInfoBox::updateInfo(ServerInfo_User *user)
{
	int userLevel = user->getUserLevel();
	
	QPixmap avatarPixmap;
	if (!avatarPixmap.loadFromData(user->getAvatarBmp()))
		avatarPixmap = UserLevelPixmapGenerator::generatePixmap(64, userLevel);
	avatarLabel->setPixmap(avatarPixmap);
	
	nameLabel->setText(user->getName());
	realNameLabel2->setText(user->getRealName());
	genderLabel2->setPixmap(GenderPixmapGenerator::generatePixmap(15, user->getGender()));
	countryLabel2->setPixmap(CountryPixmapGenerator::generatePixmap(15, user->getCountry()));
	userLevelLabel2->setPixmap(UserLevelPixmapGenerator::generatePixmap(15, userLevel));
	QString userLevelText;
	if (userLevel & ServerInfo_User::IsAdmin)
		userLevelText = tr("Administrator");
	else if (userLevel & ServerInfo_User::IsModerator)
		userLevelText = tr("Moderator");
	else if (userLevel & ServerInfo_User::IsRegistered)
		userLevelText = tr("Registered user");
	else
		userLevelText = tr("Unregistered user");
	userLevelLabel3->setText(userLevelText);
}

void UserInfoBox::updateInfo(const QString &userName)
{
	Command_GetUserInfo cmd;
	cmd.set_user_name(userName.toStdString());
	
	PendingCommand *pend = client->prepareSessionCommand(cmd);
	connect(pend, SIGNAL(finished(ProtocolResponse *)), this, SLOT(processResponse(ProtocolResponse *)));
	
	client->sendCommand(pend);
}

void UserInfoBox::processResponse(ProtocolResponse *r)
{
	Response_GetUserInfo *response = qobject_cast<Response_GetUserInfo *>(r);
	if (!response)
		return;
	
	updateInfo(response->getUserInfo());
	setFixedSize(sizeHint());
	show();
}
