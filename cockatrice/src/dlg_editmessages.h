#ifndef DLG_EDITMESSAGES_H
#define DLG_EDITMESSAGES_H

#include <QDialog>

class QListWidget;
class QPushButton;

class DlgEditMessages: public QDialog {
	Q_OBJECT
public:
	DlgEditMessages(QWidget *parent = 0);
private slots:
	void actAdd();
	void actRemove();
private:
	QListWidget *messageList;
	QAction *aAdd, *aRemove;
	QPushButton *cancelButton, *okButton;
	
	void storeSettings();
};

#endif
