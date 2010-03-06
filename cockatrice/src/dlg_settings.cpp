#include <QtGui>

#include "carddatabase.h"
#include "dlg_settings.h"
#include "main.h"
#include "settingscache.h"

GeneralSettingsPage::GeneralSettingsPage()
{
	languageLabel = new QLabel;
	languageBox = new QComboBox;
	
	QString setLanguage = settingsCache->getLang();
	QStringList qmFiles = findQmFiles();
	for (int i = 0; i < qmFiles.size(); i++) {
		QString langName = languageName(qmFiles[i]);
		languageBox->addItem(langName, qmFiles[i]);
		if ((qmFiles[i] == setLanguage) || (setLanguage.isEmpty() && langName == tr("English")))
			languageBox->setCurrentIndex(i);
	}
	
	picDownloadCheckBox = new QCheckBox;
	picDownloadCheckBox->setChecked(settingsCache->getPicDownload());
	
	connect(languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));
	connect(picDownloadCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPicDownload(int)));
	
	QGridLayout *personalGrid = new QGridLayout;
	personalGrid->addWidget(languageLabel, 0, 0);
	personalGrid->addWidget(languageBox, 0, 1);
	personalGrid->addWidget(picDownloadCheckBox, 1, 0, 1, 2);
	
	personalGroupBox = new QGroupBox;
	personalGroupBox->setLayout(personalGrid);
	
	deckPathLabel = new QLabel;
	deckPathEdit = new QLineEdit(settingsCache->getDeckPath());
	deckPathEdit->setReadOnly(true);
	QPushButton *deckPathButton = new QPushButton("...");
	connect(deckPathButton, SIGNAL(clicked()), this, SLOT(deckPathButtonClicked()));
	
	picsPathLabel = new QLabel;
	picsPathEdit = new QLineEdit(settingsCache->getPicsPath());
	picsPathEdit->setReadOnly(true);
	QPushButton *picsPathButton = new QPushButton("...");
	connect(picsPathButton, SIGNAL(clicked()), this, SLOT(picsPathButtonClicked()));
	
	cardDatabasePathLabel = new QLabel;
	cardDatabasePathEdit = new QLineEdit(settingsCache->getCardDatabasePath());
	cardDatabasePathEdit->setReadOnly(true);
	QPushButton *cardDatabasePathButton = new QPushButton("...");
	connect(cardDatabasePathButton, SIGNAL(clicked()), this, SLOT(cardDatabasePathButtonClicked()));
	
    /*
  	cardBackgroundPathLabel = new QLabel;
	cardBackgroundPathEdit = new QLineEdit(settings.value("cardbackground").toString());
	cardBackgroundPathEdit->setReadOnly(true);
	QPushButton *cardBackgroundPathButton = new QPushButton("...");
	connect(cardBackgroundPathButton, SIGNAL(clicked()), this, SLOT(cardBackgroundPathButtonClicked()));
	*/
	
	QGridLayout *pathsGrid = new QGridLayout;
	pathsGrid->addWidget(deckPathLabel, 0, 0);
	pathsGrid->addWidget(deckPathEdit, 0, 1);
	pathsGrid->addWidget(deckPathButton, 0, 2);
	pathsGrid->addWidget(picsPathLabel, 1, 0);
	pathsGrid->addWidget(picsPathEdit, 1, 1);
	pathsGrid->addWidget(picsPathButton, 1, 2);
	pathsGrid->addWidget(cardDatabasePathLabel, 2, 0);
	pathsGrid->addWidget(cardDatabasePathEdit, 2, 1);
	pathsGrid->addWidget(cardDatabasePathButton, 2, 2);
	/*	
	pathsGrid->addWidget(cardBackgroundPathLabel, 3, 0);
	pathsGrid->addWidget(cardBackgroundPathEdit, 3, 1);
	pathsGrid->addWidget(cardBackgroundPathButton, 3, 2);
	*/
	pathsGroupBox = new QGroupBox;
	pathsGroupBox->setLayout(pathsGrid);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(personalGroupBox);
	mainLayout->addWidget(pathsGroupBox);
	
	setLayout(mainLayout);
}

QStringList GeneralSettingsPage::findQmFiles()
{
	QDir dir(":/translations");
	QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
	QMutableStringListIterator i(fileNames);
	while (i.hasNext()) {
		i.next();
		i.setValue(dir.filePath(i.value()));
	}
	return fileNames;
}

QString GeneralSettingsPage::languageName(const QString &qmFile)
{
	QTranslator translator;
	translator.load(qmFile);
	
	return translator.translate("GeneralSettingsPage", "English");
}

void GeneralSettingsPage::deckPathButtonClicked()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	
	deckPathEdit->setText(path);
	settingsCache->setDeckPath(path);
}

void GeneralSettingsPage::picsPathButtonClicked()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	
	picsPathEdit->setText(path);
	settingsCache->setPicsPath(path);
}

void GeneralSettingsPage::cardDatabasePathButtonClicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	
	cardDatabasePathEdit->setText(path);
	settingsCache->setCardDatabasePath(path);
}

void GeneralSettingsPage::cardBackgroundPathButtonClicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	QSettings settings;
	settings.beginGroup("paths");
	settings.setValue("cardbackground", path);
	cardBackgroundPathEdit->setText(path);
	
	emit cardBackgroundPathChanged(path);
}

void GeneralSettingsPage::languageBoxChanged(int index)
{
	QString qmFile = languageBox->itemData(index).toString();
	settingsCache->setLang(qmFile);
}

void GeneralSettingsPage::retranslateUi()
{
	personalGroupBox->setTitle(tr("Personal settings"));
	languageLabel->setText(tr("Language:"));
	picDownloadCheckBox->setText(tr("Download card pictures on the fly"));
	pathsGroupBox->setTitle(tr("Paths"));
	deckPathLabel->setText(tr("Decks directory:"));
	picsPathLabel->setText(tr("Pictures directory:"));
	cardDatabasePathLabel->setText(tr("Path to card database:"));
	//cardBackgroundPathLabel->setText(tr("Path to card background:"));
}

AppearanceSettingsPage::AppearanceSettingsPage()
{
	zoneBgGroupBox = new QGroupBox;
	QSettings settings;
	settings.beginGroup("zonebg");
	
	handBgLabel = new QLabel;
	handBgEdit = new QLineEdit(settings.value("hand").toString());
	handBgEdit->setReadOnly(true);
	QPushButton *handBgButton = new QPushButton("...");
	connect(handBgButton, SIGNAL(clicked()), this, SLOT(handBgButtonClicked()));
	
	tableBgLabel = new QLabel;
	tableBgEdit = new QLineEdit(settings.value("table").toString());
	tableBgEdit->setReadOnly(true);
	QPushButton *tableBgButton = new QPushButton("...");
	connect(tableBgButton, SIGNAL(clicked()), this, SLOT(tableBgButtonClicked()));
	
	playerAreaBgLabel = new QLabel;
	playerAreaBgEdit = new QLineEdit(settings.value("playerarea").toString());
	playerAreaBgEdit->setReadOnly(true);
	QPushButton *playerAreaBgButton = new QPushButton("...");
	connect(playerAreaBgButton, SIGNAL(clicked()), this, SLOT(playerAreaBgButtonClicked()));
	
	settings.endGroup();

	QGridLayout *zoneBgGrid = new QGridLayout;
	zoneBgGrid->addWidget(handBgLabel, 0, 0);
	zoneBgGrid->addWidget(handBgEdit, 0, 1);
	zoneBgGrid->addWidget(handBgButton, 0, 2);
	zoneBgGrid->addWidget(tableBgLabel, 1, 0);
	zoneBgGrid->addWidget(tableBgEdit, 1, 1);
	zoneBgGrid->addWidget(tableBgButton, 1, 2);
	zoneBgGrid->addWidget(playerAreaBgLabel, 2, 0);
	zoneBgGrid->addWidget(playerAreaBgEdit, 2, 1);
	zoneBgGrid->addWidget(playerAreaBgButton, 2, 2);

	zoneBgGroupBox->setLayout(zoneBgGrid);
	
	tableGroupBox = new QGroupBox;
	economicGridCheckBox = new QCheckBox;
	economicGridCheckBox->setChecked(settingsCache->getEconomicGrid());
	connect(economicGridCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setEconomicGrid(int)));
	
	QGridLayout *tableGrid = new QGridLayout;
	tableGrid->addWidget(economicGridCheckBox, 0, 0, 1, 2);
	
	tableGroupBox->setLayout(tableGrid);
	
	zoneViewGroupBox = new QGroupBox;
	settings.beginGroup("zoneview");
	
	zoneViewSortingCheckBox = new QCheckBox;
	zoneViewSortingCheckBox->setChecked(settings.value("sorting").toInt());
	connect(zoneViewSortingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(zoneViewSortingCheckBoxChanged(int)));
	
	settings.endGroup();

	QGridLayout *zoneViewGrid = new QGridLayout;
	zoneViewGrid->addWidget(zoneViewSortingCheckBox, 0, 0, 1, 2);
	
	zoneViewGroupBox->setLayout(zoneViewGrid);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(zoneBgGroupBox);
	mainLayout->addWidget(tableGroupBox);
	mainLayout->addWidget(zoneViewGroupBox);
	
	setLayout(mainLayout);

}

void AppearanceSettingsPage::retranslateUi()
{
	zoneBgGroupBox->setTitle(tr("Zone background pictures"));
	handBgLabel->setText(tr("Path to hand background:"));
	tableBgLabel->setText(tr("Path to table background:"));
	playerAreaBgLabel->setText(tr("Path to player info background:"));
	
	tableGroupBox->setTitle(tr("Table grid layout"));
	economicGridCheckBox->setText(tr("Economic layout"));
	
	zoneViewGroupBox->setTitle(tr("Zone view layout"));
	zoneViewSortingCheckBox->setText(tr("Sort alphabetically by default"));
}

void AppearanceSettingsPage::handBgButtonClicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	QSettings settings;
	settings.beginGroup("zonebg");
	settings.setValue("hand", path);
	handBgEdit->setText(path);
	
	emit handBgChanged(path);
}

void AppearanceSettingsPage::tableBgButtonClicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	QSettings settings;
	settings.beginGroup("zonebg");
	settings.setValue("table", path);
	tableBgEdit->setText(path);
	
	emit tableBgChanged(path);
}

void AppearanceSettingsPage::playerAreaBgButtonClicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	QSettings settings;
	settings.beginGroup("zonebg");
	settings.setValue("playerarea", path);
	playerAreaBgEdit->setText(path);
	
	emit playerAreaBgChanged(path);
}

void AppearanceSettingsPage::zoneViewSortingCheckBoxChanged(int state)
{
	QSettings settings;
	settings.beginGroup("zoneview");
	settings.setValue("sorting", state);
	
	emit zoneViewSortingChanged(state);
}

UserInterfaceSettingsPage::UserInterfaceSettingsPage()
{
	doubleClickToPlayCheckBox = new QCheckBox;
	doubleClickToPlayCheckBox->setChecked(settingsCache->getDoubleClickToPlay());
	connect(doubleClickToPlayCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setDoubleClickToPlay(int)));
	
	QGridLayout *generalGrid = new QGridLayout;
	generalGrid->addWidget(doubleClickToPlayCheckBox, 0, 0);
	
	generalGroupBox = new QGroupBox;
	generalGroupBox->setLayout(generalGrid);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(generalGroupBox);
	
	setLayout(mainLayout);
}

void UserInterfaceSettingsPage::retranslateUi()
{
	generalGroupBox->setTitle(tr("General interface settings"));
	doubleClickToPlayCheckBox->setText(tr("&Double-click cards to play them (instead of single-click)"));
}

MessagesSettingsPage::MessagesSettingsPage()
{
	aAdd = new QAction(this);
	connect(aAdd, SIGNAL(triggered()), this, SLOT(actAdd()));
	aRemove = new QAction(this);
	connect(aRemove, SIGNAL(triggered()), this, SLOT(actRemove()));
	
	messageList = new QListWidget;
	QToolBar *messageToolBar = new QToolBar;
	messageToolBar->setOrientation(Qt::Vertical);
	messageToolBar->addAction(aAdd);
	messageToolBar->addAction(aRemove);

	QSettings settings;
	settings.beginGroup("messages");
	int count = settings.value("count", 0).toInt();
	for (int i = 0; i < count; i++)
		messageList->addItem(settings.value(QString("msg%1").arg(i)).toString());
	
	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(messageList);
	mainLayout->addWidget(messageToolBar);

	setLayout(mainLayout);
	
	retranslateUi();
}

void MessagesSettingsPage::storeSettings()
{
	QSettings settings;
	settings.beginGroup("messages");
	settings.setValue("count", messageList->count());
	for (int i = 0; i < messageList->count(); i++)
		settings.setValue(QString("msg%1").arg(i), messageList->item(i)->text());
}

void MessagesSettingsPage::actAdd()
{
	bool ok;
	QString msg = QInputDialog::getText(this, tr("Add message"), tr("Message:"), QLineEdit::Normal, QString(), &ok);
	if (ok) {
		messageList->addItem(msg);
		storeSettings();
	}
}

void MessagesSettingsPage::actRemove()
{
	if (messageList->currentItem()) {
		delete messageList->takeItem(messageList->currentRow());
		storeSettings();
	}
}

void MessagesSettingsPage::retranslateUi()
{
	aAdd->setText(tr("&Add"));
	aRemove->setText(tr("&Remove"));
}

DlgSettings::DlgSettings(QWidget *parent)
	: QDialog(parent)
{
	connect(settingsCache, SIGNAL(langChanged()), this, SLOT(updateLanguage()));
	
	contentsWidget = new QListWidget;
	contentsWidget->setViewMode(QListView::IconMode);
	contentsWidget->setIconSize(QSize(96, 84));
	contentsWidget->setMovement(QListView::Static);
	contentsWidget->setMinimumWidth(115);
	contentsWidget->setMaximumWidth(130);
	contentsWidget->setSpacing(12);
	
	pagesWidget = new QStackedWidget;
	pagesWidget->addWidget(new GeneralSettingsPage);
	pagesWidget->addWidget(new AppearanceSettingsPage);
	pagesWidget->addWidget(new UserInterfaceSettingsPage);
	pagesWidget->addWidget(new MessagesSettingsPage);
	
	closeButton = new QPushButton;
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	
	createIcons();
	contentsWidget->setCurrentRow(0);
	
	QHBoxLayout *hboxLayout = new QHBoxLayout;
	hboxLayout->addWidget(contentsWidget);
	hboxLayout->addWidget(pagesWidget);
	
	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(closeButton);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hboxLayout);
	//mainLayout->addStretch(1);
	mainLayout->addSpacing(12);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	
	retranslateUi();
	
	resize(700, 450);
}

void DlgSettings::createIcons()
{
	generalButton = new QListWidgetItem(contentsWidget);
	generalButton->setTextAlignment(Qt::AlignHCenter);
	generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	generalButton->setIcon(QIcon(":/resources/icon_config_general.svg"));
	
	appearanceButton = new QListWidgetItem(contentsWidget);
	appearanceButton->setTextAlignment(Qt::AlignHCenter);
	appearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	appearanceButton->setIcon(QIcon(":/resources/icon_config_appearance.svg"));
	
	userInterfaceButton = new QListWidgetItem(contentsWidget);
	userInterfaceButton->setTextAlignment(Qt::AlignHCenter);
	userInterfaceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	userInterfaceButton->setIcon(QIcon(":/resources/icon_config_appearance.svg"));
	
	messagesButton = new QListWidgetItem(contentsWidget);
	messagesButton->setTextAlignment(Qt::AlignHCenter);
	messagesButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	messagesButton->setIcon(QIcon(":/resources/icon_config_messages.svg"));
	
	connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem *)));
}

void DlgSettings::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (!current)
		current = previous;
	
	pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void DlgSettings::updateLanguage()
{
	qApp->removeTranslator(translator);
	translator->load(settingsCache->getLang());
	qApp->installTranslator(translator);
}

void DlgSettings::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
		retranslateUi();
	QDialog::changeEvent(event);
}

void DlgSettings::retranslateUi()
{
	setWindowTitle(tr("Settings"));
	
	generalButton->setText(tr("General"));
	appearanceButton->setText(tr("Appearance"));
	userInterfaceButton->setText(tr("User interface"));
	messagesButton->setText(tr("Messages"));
	
	closeButton->setText(tr("&Close"));
	
	for (int i = 0; i < pagesWidget->count(); i++)
		dynamic_cast<AbstractSettingsPage *>(pagesWidget->widget(i))->retranslateUi();
}
