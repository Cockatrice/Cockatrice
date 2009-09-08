#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QDialog>

class CardDatabase;
class QTranslator;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QLineEdit;
class QPushButton;
class QComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;

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
	void picDownloadCheckBoxChanged(int state);
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
	void economicGridCheckBoxChanged(int state);
signals:
	void handBgChanged(const QString &path);
	void tableBgChanged(const QString &path);
	void playerAreaBgChanged(const QString &path);
	void economicGridChanged(int state);
private:
	QLabel *handBgLabel, *tableBgLabel, *playerAreaBgLabel;
	QLineEdit *handBgEdit, *tableBgEdit, *playerAreaBgEdit;
	QCheckBox *economicGridCheckBox;
	QGroupBox *zoneBgGroupBox, *tableGroupBox;
public:
	AppearanceSettingsPage();
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
	DlgSettings(CardDatabase *_db, QTranslator *_translator, QWidget *parent = 0);
private slots:
	void changePage(QListWidgetItem *current, QListWidgetItem *previous);
	void changeLanguage(const QString &qmFile);
private:
	CardDatabase *db;
	QTranslator *translator;
	QListWidget *contentsWidget;
	QStackedWidget *pagesWidget;
	QListWidgetItem *generalButton, *appearanceButton, *messagesButton;
	QPushButton *closeButton;
	void createIcons();
	void retranslateUi();
protected:
	void changeEvent(QEvent *event);
};

#endif 
