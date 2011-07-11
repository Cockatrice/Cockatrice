#ifndef WINDOW_MAIN_H
#define WINDOW_MAIN_H

#include <QMainWindow>
#include <QList>

class OracleImporter;
class QLabel;
class QProgressBar;
class QTextEdit;
class QPushButton;
class QCheckBox;
class QVBoxLayout;
class QMenu;
class QAction;
class QNetworkAccessManager;

class WindowMain : public QMainWindow {
	Q_OBJECT
private:
	static const QString defaultSetsUrl;
	
	OracleImporter *importer;
	QNetworkAccessManager *nam;
	
	QMenu *fileMenu;
	QAction *aLoadSetsFile, *aDownloadSetsFile, *aExit;
	QPushButton *checkAllButton, *uncheckAllButton, *startButton;
	QLabel *totalLabel, *fileLabel, *nextSetLabel1, *nextSetLabel2;
	QProgressBar *totalProgressBar, *fileProgressBar;
	QTextEdit *messageLog;
	QVBoxLayout *checkBoxLayout;
	QList<QCheckBox *> checkBoxList;
	QLabel *statusLabel;
	
	void downloadSetsFile(const QString &url);
private slots:
	void updateTotalProgress(int cardsImported, int setIndex, const QString &nextSetName);
	void updateFileProgress(int bytesRead, int totalBytes);
	void updateSetList();
	void actCheckAll();
	void actUncheckAll();
	void actStart();
	void actLoadSetsFile();
	void actDownloadSetsFile();
	void setsDownloadFinished();
	void checkBoxChanged(int state);
public:
	WindowMain(QWidget *parent = 0);
};

#endif
