#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QDialog>

class CardDatabase;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QLineEdit;
class QPushButton;
class QComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;
class QCloseEvent;

class AbstractSettingsPage : public QWidget {
public:
	virtual void retranslateUi() = 0;
};

class GeneralSettingsPage : public AbstractSettingsPage {
	Q_OBJECT
public:
	GeneralSettingsPage();
	void retranslateUi();
private slots:
	void deckPathButtonClicked();
	void picsPathButtonClicked();
	void cardDatabasePathButtonClicked();
	void cardBackgroundPathButtonClicked();
	void languageBoxChanged(int index);
signals:
	void picsPathChanged(const QString &path);
	void cardDatabasePathChanged(const QString &path);
	void cardBackgroundPathChanged(const QString &path);
	void changeLanguage(const QString &qmFile);
	void picDownloadChanged(int state);
private:
	QStringList findQmFiles();
	QString languageName(const QString &qmFile);
	QLineEdit *deckPathEdit, *picsPathEdit, *cardDatabasePathEdit, *cardBackgroundPathEdit;
	QGroupBox *personalGroupBox, *pathsGroupBox;
	QComboBox *languageBox;
	QCheckBox *picDownloadCheckBox;
	QLabel *languageLabel, *deckPathLabel, *picsPathLabel, *cardDatabasePathLabel, *cardBackgroundPathLabel;
};

class AppearanceSettingsPage : public AbstractSettingsPage {
	Q_OBJECT
private slots:
	void handBgButtonClicked();
	void tableBgButtonClicked();
	void playerAreaBgButtonClicked();
signals:
	void handBgChanged(const QString &path);
	void tableBgChanged(const QString &path);
	void playerAreaBgChanged(const QString &path);
private:
	QLabel *handBgLabel, *tableBgLabel, *playerAreaBgLabel;
	QLineEdit *handBgEdit, *tableBgEdit, *playerAreaBgEdit;
	QCheckBox *economicGridCheckBox, *zoneViewSortByNameCheckBox, *zoneViewSortByTypeCheckBox;
	QGroupBox *zoneBgGroupBox, *tableGroupBox, *zoneViewGroupBox;
public:
	AppearanceSettingsPage();
	void retranslateUi();
};

class UserInterfaceSettingsPage : public AbstractSettingsPage {
	Q_OBJECT
private:
	QCheckBox *doubleClickToPlayCheckBox;
	QGroupBox *generalGroupBox;
public:
	UserInterfaceSettingsPage();
	void retranslateUi();
};

class MessagesSettingsPage : public AbstractSettingsPage {
	Q_OBJECT
public:
	MessagesSettingsPage();
	void retranslateUi();
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
	DlgSettings(QWidget *parent = 0);
private slots:
	void changePage(QListWidgetItem *current, QListWidgetItem *previous);
	void updateLanguage();
private:
	QListWidget *contentsWidget;
	QStackedWidget *pagesWidget;
	QListWidgetItem *generalButton, *appearanceButton, *userInterfaceButton, *messagesButton;
	QPushButton *closeButton;
	void createIcons();
	void retranslateUi();
protected:
	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent *event);
};

#endif 
