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

class WindowMain : public QMainWindow {
	Q_OBJECT
private:
	OracleImporter *importer;
	
	QPushButton *startButton;
	QLabel *totalLabel, *fileLabel, *nextSetLabel1, *nextSetLabel2;
	QProgressBar *totalProgressBar, *fileProgressBar;
	QTextEdit *messageLog;
	QList<QCheckBox *> checkBoxList;
private slots:
	void updateTotalProgress(int cardsImported, int setIndex, const QString &nextSetName);
	void updateFileProgress(int bytesRead, int totalBytes);
	void actStart();
	void checkBoxChanged(int state);
public:
	WindowMain(QWidget *parent = 0);
};

#endif
