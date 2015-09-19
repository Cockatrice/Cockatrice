#include "shortcutstab.h"
#include "ui_shortcutstab.h"

#include "../settingscache.h"
#include <QMessageBox>

ShortcutsTab::ShortcutsTab() :
    ui(new Ui::shortcutsTab)
{
    ui->setupUi(this);
    connect(ui->btnResetAll,SIGNAL(clicked()),this,SLOT(resetShortcuts()));
    connect(ui->btnClearAll,SIGNAL(clicked()),this,SLOT(clearShortcuts()));
    connect(&settingsCache->shortcuts(),SIGNAL(allShortCutsReset()),this,SLOT(refreshEdits()));
    connect(&settingsCache->shortcuts(),SIGNAL(allShortCutsClear()),this,SLOT(afterClear()));
}

void ShortcutsTab::retranslateUi()
{
    ui->retranslateUi(this);
}

ShortcutsTab::~ShortcutsTab()
{
    delete ui;
}

void ShortcutsTab::resetShortcuts()
{
    if(QMessageBox::question(this,tr("Restore all default shortcuts"),
                             tr("Do you really want to restore all default shortcuts?")) == QMessageBox::Yes)
    {
        settingsCache->shortcuts().resetAllShortcuts();
    }
}

void ShortcutsTab::refreshEdits()
{    
    QList<SequenceEdit*> edits = this->findChildren<SequenceEdit*>();
    for(int i= 0; i < edits.length(); ++i)
    {
        edits.at(i)->refreshShortcut();
    }
}

void ShortcutsTab::clearShortcuts()
{
    if(QMessageBox::question(this,tr("Clear all default shortcuts"),
                             tr("Do you really want to clear all shortcuts?")) == QMessageBox::Yes)
    {
        settingsCache->shortcuts().clearAllShortcuts();
    }
}

void ShortcutsTab::afterClear()
{    
    QList<SequenceEdit*> edits = this->findChildren<SequenceEdit*>();
    for(int i= 0; i < edits.length(); ++i)
    {
        edits.at(i)->clear();
    }
}
