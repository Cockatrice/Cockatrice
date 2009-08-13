#include <QtGui>

#include "carddatabase.h"
#include "dlg_settings.h"

GeneralSettingsPage::GeneralSettingsPage()
{
	QSettings settings;
	
	personalGroupBox = new QGroupBox;
	languageLabel = new QLabel;
	languageBox = new QComboBox;
	
	settings.beginGroup("personal");
	QString setLanguage = settings.value("lang").toString();
	QStringList qmFiles = findQmFiles();
	for (int i = 0; i < qmFiles.size(); i++) {
		QString langName = languageName(qmFiles[i]);
		languageBox->addItem(langName, qmFiles[i]);
		if ((qmFiles[i] == settings.value("lang").toString()) || (setLanguage.isEmpty() && langName == tr("English")))
			languageBox->setCurrentIndex(i);
	}
	settings.endGroup();
	connect(languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));
	
	QGridLayout *personalGrid = new QGridLayout;
	personalGrid->addWidget(languageLabel, 0, 0);
	personalGrid->addWidget(languageBox, 0, 1);
	personalGroupBox->setLayout(personalGrid);
	
	pathsGroupBox = new QGroupBox;
	settings.beginGroup("paths");
	
	deckPathLabel = new QLabel;
	deckPathEdit = new QLineEdit(settings.value("decks").toString());
	deckPathEdit->setReadOnly(true);
	QPushButton *deckPathButton = new QPushButton("...");
	connect(deckPathButton, SIGNAL(clicked()), this, SLOT(deckPathButtonClicked()));
	
	picsPathLabel = new QLabel;
	picsPathEdit = new QLineEdit(settings.value("pics").toString());
	picsPathEdit->setReadOnly(true);
	QPushButton *picsPathButton = new QPushButton("...");
	connect(picsPathButton, SIGNAL(clicked()), this, SLOT(picsPathButtonClicked()));
	
	cardDatabasePathLabel = new QLabel;
	cardDatabasePathEdit = new QLineEdit(settings.value("carddatabase").toString());
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
	QSettings settings;
	settings.beginGroup("paths");
	settings.setValue("decks", path);
	deckPathEdit->setText(path);
}

void GeneralSettingsPage::picsPathButtonClicked()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	QSettings settings;
	settings.beginGroup("paths");
	settings.setValue("pics", path);
	picsPathEdit->setText(path);
	
	emit picsPathChanged(path);
}

void GeneralSettingsPage::cardDatabasePathButtonClicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
	if (path.isEmpty())
		return;
	QSettings settings;
	settings.beginGroup("paths");
	settings.setValue("carddatabase", path);
	cardDatabasePathEdit->setText(path);
	
	emit cardDatabasePathChanged(path);
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
	QSettings settings;
	settings.beginGroup("personal");
	settings.setValue("lang", qmFile);
	emit changeLanguage(qmFile);
}

void GeneralSettingsPage::retranslateUi()
{
	personalGroupBox->setTitle(tr("Personal settings"));
	languageLabel->setText(tr("Language:"));
	pathsGroupBox->setTitle(tr("Paths"));
	deckPathLabel->setText(tr("Decks directory:"));
	picsPathLabel->setText(tr("Pictures directory:"));
	cardDatabasePathLabel->setText(tr("Path to card database:"));
	//cardBackgroundPathLabel->setText(tr("Path to card background:"));
}

AppearanceSettingsPage::AppearanceSettingsPage()
{

}

void AppearanceSettingsPage::retranslateUi()
{

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

DlgSettings::DlgSettings(CardDatabase *_db, QTranslator *_translator, QWidget *parent)
	: QDialog(parent), db(_db), translator(_translator)
{
	contentsWidget = new QListWidget;
	contentsWidget->setViewMode(QListView::IconMode);
	contentsWidget->setIconSize(QSize(96, 84));
	contentsWidget->setMovement(QListView::Static);
	contentsWidget->setMinimumWidth(128);
	contentsWidget->setMaximumWidth(150);
	contentsWidget->setSpacing(12);
	
	pagesWidget = new QStackedWidget;
	GeneralSettingsPage *general = new GeneralSettingsPage;
	connect(general, SIGNAL(picsPathChanged(const QString &)), db, SLOT(updatePicsPath(const QString &)));
	connect(general, SIGNAL(cardDatabasePathChanged(const QString &)), db, SLOT(updateDatabasePath(const QString &)));
	connect(general, SIGNAL(changeLanguage(const QString &)), this, SLOT(changeLanguage(const QString &)));
	pagesWidget->addWidget(general);
	pagesWidget->addWidget(new AppearanceSettingsPage);
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
	mainLayout->addStretch(1);
	mainLayout->addSpacing(12);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	
	retranslateUi();
	
	resize(700, sizeHint().height());
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

void DlgSettings::changeLanguage(const QString &qmFile)
{
	qApp->removeTranslator(translator);
	translator->load(qmFile);
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
	messagesButton->setText(tr("Messages"));
	
	closeButton->setText(tr("&Close"));
	
	for (int i = 0; i < pagesWidget->count(); i++)
		dynamic_cast<AbstractSettingsPage *>(pagesWidget->widget(i))->retranslateUi();
}
