#include <QtGui>
#include "window_main.h"
#include "oracleimporter.h"

WindowMain::WindowMain(QWidget *parent)
	: QMainWindow(parent)
{
	importer = new OracleImporter("../oracle", this);
	
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
	
	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(grid);
	setCentralWidget(centralWidget);
	
	connect(importer, SIGNAL(setIndexChanged(int, int, const QString &)), this, SLOT(updateTotalProgress(int, int, const QString &)));
	connect(importer, SIGNAL(dataReadProgress(int, int)), this, SLOT(updateFileProgress(int, int)));
	totalProgressBar->setMaximum(importer->getSetsCount());
	
	setWindowTitle(tr("Oracle importer"));
	setFixedSize(300, 300);
	
	importer->downloadNextFile();
}

void WindowMain::updateTotalProgress(int cardsImported, int setIndex, const QString &nextSetName)
{
	if (setIndex != 0)
		messageLog->append(QString("%1: %2 cards imported").arg(nextSetLabel2->text()).arg(cardsImported));
	totalProgressBar->setValue(setIndex);
	if (nextSetName.isEmpty()) {
		QMessageBox::information(this, tr("Oracle importer"), tr("Import finished: %1 cards.").arg(importer->getCardList().size()));
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
