#include <QtGui>
#include "dlg_editmessages.h"

DlgEditMessages::DlgEditMessages(QWidget *parent)
	: QDialog(parent)
{
	aAdd = new QAction(tr("Add"), this);
	connect(aAdd, SIGNAL(triggered()), this, SLOT(actAdd()));
	aRemove = new QAction(tr("Remove"), this);
	connect(aRemove, SIGNAL(triggered()), this, SLOT(actRemove()));
	
	messageList = new QListWidget;
	QToolBar *messageToolBar = new QToolBar;
	messageToolBar->setOrientation(Qt::Vertical);
	messageToolBar->addAction(aAdd);
	messageToolBar->addAction(aRemove);

	QSettings settings;
	settings.beginGroup("messages");
	int count = settings.value("count", 0).toInt();
	for (int i = 0; i < count; i++)
		messageList->addItem(settings.value(QString("msg%1").arg(i)).toString());
	
	QHBoxLayout *listLayout = new QHBoxLayout;
	listLayout->addWidget(messageList);
	listLayout->addWidget(messageToolBar);

	cancelButton = new QPushButton(tr("&Cancel"));
	okButton = new QPushButton(tr("O&K"));
	okButton->setAutoDefault(true);
	
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(cancelButton);
	buttonLayout->addWidget(okButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(listLayout);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setWindowTitle(tr("Edit messages"));
	setMinimumWidth(sizeHint().width());
	resize(300, 300);

	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void DlgEditMessages::storeSettings()
{
	QSettings settings;
	settings.beginGroup("messages");
	settings.setValue("count", messageList->count());
	for (int i = 0; i < messageList->count(); i++)
		settings.setValue(QString("msg%1").arg(i), messageList->item(i)->text());
}

void DlgEditMessages::actAdd()
{
	bool ok;
	QString msg = QInputDialog::getText(this, tr("Add message"), QString("Message:"), QLineEdit::Normal, QString(), &ok);
	if (ok) {
		messageList->addItem(msg);
		storeSettings();
	}
}

void DlgEditMessages::actRemove()
{
	if (messageList->currentItem()) {
		delete messageList->takeItem(messageList->currentRow());
		storeSettings();
	}
}
