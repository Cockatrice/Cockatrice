#ifndef WINDOW_MAIN_H
#define WINDOW_MAIN_H

#include <QMainWindow>

class OracleImporter;
class QLabel;
class QProgressBar;
class QTextEdit;

class WindowMain : public QMainWindow {
	Q_OBJECT
private:
	OracleImporter *importer;
	
	QLabel *totalLabel, *fileLabel, *nextSetLabel1, *nextSetLabel2;
	QProgressBar *totalProgressBar, *fileProgressBar;
	QTextEdit *messageLog;
private slots:
	void updateTotalProgress(int cardsImported, int setIndex, const QString &nextSetName);
	void updateFileProgress(int bytesRead, int totalBytes);
public:
	WindowMain(QWidget *parent = 0);
};

#endif
