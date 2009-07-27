#include <QtGui>

#include "dlg_settings.h"

GeneralSettingsPage::GeneralSettingsPage()
{
	QGroupBox *personalGroupBox = new QGroupBox(tr("Personal settings"));
	QLabel *languageLabel = new QLabel(tr("Language:"));
	QComboBox *languageBox = new QComboBox;
	
	QStringList qmFiles = findQmFiles();
	for (int i = 0; i < qmFiles.size(); i++)
		languageBox->addItem(languageName(qmFiles[i]), qmFiles[i]);
	
	QGridLayout *personalGrid = new QGridLayout;
	personalGrid->addWidget(languageLabel, 0, 0);
	personalGrid->addWidget(languageBox, 0, 1);
	personalGroupBox->setLayout(personalGrid);
	
	QGroupBox *pathsGroupBox = new QGroupBox(tr("Paths"));
	QLabel *deckPathLabel = new QLabel(tr("Decks directory:"));
	QLineEdit *deckPathEdit = new QLineEdit;
	QLabel *picsPathLabel = new QLabel(tr("Pictures directory:"));
	QLineEdit *picsPathEdit = new QLineEdit;
	QLabel *cardDatabasePathLabel = new QLabel(tr("Path to card database:"));
	QLineEdit *cardDatabasePathEdit = new QLineEdit;
	
	QGridLayout *pathsGrid = new QGridLayout;
	pathsGrid->addWidget(deckPathLabel, 0, 0);
	pathsGrid->addWidget(deckPathEdit, 0, 1);
	pathsGrid->addWidget(picsPathLabel, 1, 0);
	pathsGrid->addWidget(picsPathEdit, 1, 1);
	pathsGrid->addWidget(cardDatabasePathLabel, 2, 0);
	pathsGrid->addWidget(cardDatabasePathEdit, 2, 1);
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

AppearanceSettingsPage::AppearanceSettingsPage()
{

}

MessagesSettingsPage::MessagesSettingsPage()
{
	aAdd = new QAction(tr("Add"), this);
	connect(aAdd, SIGNAL(triggered()), this, SLOT(actAdd()));
	aRemove = new QAction(tr("Remove"), this);
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

DlgSettings::DlgSettings()
	: QDialog()
{
	contentsWidget = new QListWidget;
	contentsWidget->setViewMode(QListView::IconMode);
	contentsWidget->setIconSize(QSize(96, 84));
	contentsWidget->setMovement(QListView::Static);
	contentsWidget->setMaximumWidth(128);
	contentsWidget->setSpacing(12);
	
	pagesWidget = new QStackedWidget;
	pagesWidget->addWidget(new GeneralSettingsPage);
	pagesWidget->addWidget(new AppearanceSettingsPage);
	pagesWidget->addWidget(new MessagesSettingsPage);
	
	QPushButton *closeButton = new QPushButton(tr("&Close"));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	
	createIcons();
	contentsWidget->setCurrentRow(0);
	
	QHBoxLayout *hboxLayout = new QHBoxLayout;
	hboxLayout->addWidget(contentsWidget);
	hboxLayout->addWidget(pagesWidget, 1);
	
	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(closeButton);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hboxLayout);
	mainLayout->addStretch(1);
	mainLayout->addSpacing(12);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	
	setWindowTitle(tr("Settings"));
}

void DlgSettings::createIcons()
{
	QListWidgetItem *generalButton = new QListWidgetItem(contentsWidget);
	generalButton->setText(tr("General"));
	generalButton->setTextAlignment(Qt::AlignHCenter);
	generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	
	QListWidgetItem *appearanceButton = new QListWidgetItem(contentsWidget);
	appearanceButton->setText(tr("Appearance"));
	appearanceButton->setTextAlignment(Qt::AlignHCenter);
	appearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	
	QListWidgetItem *messagesButton = new QListWidgetItem(contentsWidget);
	messagesButton->setText(tr("Messages"));
	messagesButton->setTextAlignment(Qt::AlignHCenter);
	messagesButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	
	connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem *)));
}

void DlgSettings::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (!current)
		current = previous;
	
	pagesWidget->setCurrentIndex(contentsWidget->row(current));
}
