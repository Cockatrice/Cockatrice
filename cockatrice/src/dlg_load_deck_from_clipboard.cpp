#include <QClipboard>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QApplication>
#include <QTextStream>
#include <QMessageBox>
#include "dlg_load_deck_from_clipboard.h"
#include "decklist.h"

DlgLoadDeckFromClipboard::DlgLoadDeckFromClipboard(QWidget *parent)
	: QDialog(parent), deckList(0)
{
	contentsEdit = new QPlainTextEdit;
	
	refreshButton = new QPushButton(tr("&Refresh"));
	refreshButton->setShortcut(QKeySequence("F5"));
	okButton = new QPushButton(tr("&OK"));
	okButton->setDefault(true);
	cancelButton = new QPushButton(tr("&Cancel"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(refreshButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(contentsEdit);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setWindowTitle(tr("Load deck from clipboard"));
	resize(500, 500);

	connect(refreshButton, SIGNAL(clicked()), this, SLOT(actRefresh()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(actOK()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	actRefresh();
}

void DlgLoadDeckFromClipboard::actRefresh()
{
	contentsEdit->setPlainText(QApplication::clipboard()->text());
}

void DlgLoadDeckFromClipboard::actOK()
{
	QString buffer = contentsEdit->toPlainText();
	QTextStream stream(&buffer);
	
	DeckList *l = new DeckList;
	if (l->loadFromStream_Plain(stream)) {
		deckList = l;
		accept();
	} else {
		QMessageBox::critical(this, tr("Error"), tr("Invalid deck list."));
		delete l;
	}
}
