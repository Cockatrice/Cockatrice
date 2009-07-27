#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class GeneralSettingsPage : public QWidget {
	Q_OBJECT
public:
	GeneralSettingsPage();
private:
	QStringList findQmFiles();
	QString languageName(const QString &qmFile);
};

class AppearanceSettingsPage : public QWidget {
	Q_OBJECT
public:
	AppearanceSettingsPage();
};

class MessagesSettingsPage : public QWidget {
	Q_OBJECT
public:
	MessagesSettingsPage();
private slots:
	void actAdd();
	void actRemove();
private:
	QListWidget *messageList;
	QAction *aAdd, *aRemove;
	
	void storeSettings();
};

class DlgSettings : public QDialog {
	Q_OBJECT
public:
	DlgSettings();
public slots:
	void changePage(QListWidgetItem *current, QListWidgetItem *previous);
private:
	QListWidget *contentsWidget;
	QStackedWidget *pagesWidget;
	void createIcons();
};

#endif 
