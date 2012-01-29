#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QTextEdit>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "window_main.h"
#include "oracleimporter.h"

const QString WindowMain::defaultSetsUrl = QString("http://www.cockatrice.de/files/sets.xml");

WindowMain::WindowMain(QWidget *parent)
	: QMainWindow(parent)
{
	importer = new OracleImporter(QDesktopServices::storageLocation(QDesktopServices::DataLocation), this);
	nam = new QNetworkAccessManager(this);
	
	checkBoxLayout = new QVBoxLayout;
	
	QWidget *checkboxFrame = new QWidget;
	checkboxFrame->setLayout(checkBoxLayout);
	
	QScrollArea *checkboxArea = new QScrollArea;
	checkboxArea->setWidget(checkboxFrame);
	checkboxArea->setWidgetResizable(true);
	
	checkAllButton = new QPushButton(tr("&Check all"));
	connect(checkAllButton, SIGNAL(clicked()), this, SLOT(actCheckAll()));
	uncheckAllButton = new QPushButton(tr("&Uncheck all"));
	connect(uncheckAllButton, SIGNAL(clicked()), this, SLOT(actUncheckAll()));
	
	QHBoxLayout *checkAllButtonLayout = new QHBoxLayout;
	checkAllButtonLayout->addWidget(checkAllButton);
	checkAllButtonLayout->addWidget(uncheckAllButton);
	
	startButton = new QPushButton(tr("&Start download"));
	connect(startButton, SIGNAL(clicked()), this, SLOT(actStart()));
	
	QVBoxLayout *settingsLayout = new QVBoxLayout;
	settingsLayout->addWidget(checkboxArea);
	settingsLayout->addLayout(checkAllButtonLayout);
	settingsLayout->addWidget(startButton);
	
	totalLabel = new QLabel(tr("Total progress:"));
	totalProgressBar = new QProgressBar;
	nextSetLabel1 = new QLabel(tr("Current file:"));
	nextSetLabel2 = new QLabel;
	fileLabel = new QLabel(tr("Progress:"));
	fileProgressBar = new QProgressBar;
	
	messageLog = new QTextEdit;
	messageLog->setReadOnly(true);
	
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(totalLabel, 0, 0);
	grid->addWidget(totalProgressBar, 0, 1);
	grid->addWidget(nextSetLabel1, 1, 0);
	grid->addWidget(nextSetLabel2, 1, 1);
	grid->addWidget(fileLabel, 2, 0);
	grid->addWidget(fileProgressBar, 2, 1);
	grid->addWidget(messageLog, 3, 0, 1, 2);
	
	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addLayout(settingsLayout, 6);
	mainLayout->addSpacing(10);
	mainLayout->addLayout(grid, 10);
	
	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);
	
	connect(importer, SIGNAL(setIndexChanged(int, int, const QString &)), this, SLOT(updateTotalProgress(int, int, const QString &)));
	connect(importer, SIGNAL(dataReadProgress(int, int)), this, SLOT(updateFileProgress(int, int)));
	
	aLoadSetsFile = new QAction(tr("Load sets information from &file..."), this);
	connect(aLoadSetsFile, SIGNAL(triggered()), this, SLOT(actLoadSetsFile()));
	aDownloadSetsFile = new QAction(tr("&Download sets information..."), this);
	connect(aDownloadSetsFile, SIGNAL(triggered()), this, SLOT(actDownloadSetsFile()));
	aExit = new QAction(tr("E&xit"), this);
	connect(aExit, SIGNAL(triggered()), this, SLOT(close()));
	
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(aLoadSetsFile);
	fileMenu->addAction(aDownloadSetsFile);
	fileMenu->addSeparator();
	fileMenu->addAction(aExit);
	
	setWindowTitle(tr("Oracle importer"));
	setMinimumSize(750, 500);

	QStringList args = qApp->arguments();
	if (args.contains("-dlsets"))
		downloadSetsFile(defaultSetsUrl);
	
	statusLabel = new QLabel;
	statusBar()->addWidget(statusLabel);
	statusLabel->setText(tr("Sets data not loaded."));
}

void WindowMain::updateSetList()
{
	for (int i = 0; i < checkBoxList.size(); ++i)
		delete checkBoxList[i];
	checkBoxList.clear();
	
	QList<SetToDownload> &sets = importer->getSets();
	for (int i = 0; i < sets.size(); ++i) {
		QCheckBox *checkBox = new QCheckBox(sets[i].getLongName());
		checkBox->setChecked(sets[i].getImport());
		connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
		checkBoxLayout->addWidget(checkBox);
		checkBoxList << checkBox;
	}
	statusLabel->setText(tr("Ready."));
}

void WindowMain::actLoadSetsFile()
{
	QFileDialog dialog(this, tr("Load sets file"));
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter("Sets XML file (*.xml)");
	if (!dialog.exec())
		return;

	QString fileName = dialog.selectedFiles().at(0);
	if (importer->readSetsFromFile(fileName))
		updateSetList();
	else
		QMessageBox::critical(this, tr("Error"), tr("This file does not contain any sets data."));
}

void WindowMain::actDownloadSetsFile()
{
	QString url = QInputDialog::getText(this, tr("Load sets from URL"), tr("Please enter the URL of the sets file:"), QLineEdit::Normal, defaultSetsUrl);
	if (!url.isEmpty())
		downloadSetsFile(url);
}

void WindowMain::downloadSetsFile(const QString &url)
{
	QNetworkReply *reply = nam->get(QNetworkRequest(url));
	connect(reply, SIGNAL(finished()), this, SLOT(setsDownloadFinished()));
}

void WindowMain::setsDownloadFinished()
{
	QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
	QNetworkReply::NetworkError errorCode = reply->error();
	if (errorCode == QNetworkReply::NoError) {
		if (importer->readSetsFromByteArray(reply->readAll()))
			updateSetList();
		else
			QMessageBox::critical(this, tr("Error"), tr("The file was retrieved successfully, but it does not contain any sets data."));
	} else
		QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));
	reply->deleteLater();
}

void WindowMain::updateTotalProgress(int cardsImported, int setIndex, const QString &nextSetName)
{
	if (setIndex != 0)
		messageLog->append(QString("%1: %2 cards imported").arg(nextSetLabel2->text()).arg(cardsImported));
	totalProgressBar->setValue(setIndex);
	if (nextSetName.isEmpty()) {
		QMessageBox::information(this, tr("Oracle importer"), tr("Import finished: %1 cards.").arg(importer->getCardList().size()));
		bool ok = false;
		const QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
		QDir dir(dataDir);
		if (!dir.exists())
			dir.mkpath(dataDir);
		QString savePath = dataDir + "/cards.xml";
		do {
			QString fileName;
			if (savePath.isEmpty())
				fileName = QFileDialog::getSaveFileName(this, tr("Save card database"), dataDir + "/cards.xml", tr("XML card database (*.xml)"));
			else {
				fileName = savePath;
				savePath.clear();
			}
			if (fileName.isEmpty()) {
				qApp->quit();
				return;
			}
			if (importer->saveToFile(fileName))
				ok = true;
			else
				QMessageBox::critical(this, tr("Error"), tr("The file could not be saved to the desired location."));
		} while (!ok);
		qApp->quit();
	} else {
		nextSetLabel2->setText(nextSetName);
	}
}

void WindowMain::updateFileProgress(int bytesRead, int totalBytes)
{
	fileProgressBar->setMaximum(totalBytes);
	fileProgressBar->setValue(bytesRead);
}

void WindowMain::actCheckAll()
{
	for (int i = 0; i < checkBoxList.size(); ++i)
		checkBoxList[i]->setChecked(true);
}

void WindowMain::actUncheckAll()
{
	for (int i = 0; i < checkBoxList.size(); ++i)
		checkBoxList[i]->setChecked(false);
}

void WindowMain::actStart()
{
	int setsCount = importer->startDownload();
	if (!setsCount) {
		QMessageBox::critical(this, tr("Error"), tr("No sets to download selected."));
		return;
	}
	for (int i = 0; i < checkBoxList.size(); ++i)
		checkBoxList[i]->setEnabled(false);
	startButton->setEnabled(false);
	totalProgressBar->setMaximum(setsCount);
	statusLabel->setText(tr("Downloading card data..."));
}

void WindowMain::checkBoxChanged(int state)
{
	QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
	QList<SetToDownload> &sets = importer->getSets();
	for (int i = 0; i < sets.size(); ++i)
		if (sets[i].getLongName() == checkBox->text()) {
			sets[i].setImport(state);
			break;
		}
}
