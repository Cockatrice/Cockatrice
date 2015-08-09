#include "shortcutstab.h"
#include "ui_shortcutstab.h"

ShortcutsTab::ShortcutsTab() :
    ui(new Ui::shortcutsTab)
{
    ui->setupUi(this);
}

void ShortcutsTab::retranslateUi()
{
    ui->retranslateUi(this);
}

ShortcutsTab::~ShortcutsTab()
{
    delete ui;
}
