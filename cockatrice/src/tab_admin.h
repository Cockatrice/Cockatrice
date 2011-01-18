#ifndef TAB_ADMIN_H
#define TAB_ADMIN_H

#include "tab.h"

class AbstractClient;

class QGroupBox;
class QPushButton;

class TabAdmin : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	QPushButton *updateServerMessageButton;
	QGroupBox *adminGroupBox;
	QPushButton *unlockButton, *lockButton;
private slots:
	void actUpdateServerMessage();
	
	void actUnlock();
	void actLock();
public:
	TabAdmin(AbstractClient *_client, QWidget *parent = 0);
	void retranslateUi();
	QString getTabText() const { return tr("Administration"); }
};

#endif
