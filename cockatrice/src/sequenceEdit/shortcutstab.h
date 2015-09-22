#ifndef SHORTCUTSTAB_H
#define SHORTCUTSTAB_H

#include <QWidget>

#include "../dlg_settings.h"

namespace Ui {
class shortcutsTab;
}

class ShortcutsTab : public AbstractSettingsPage
{
    Q_OBJECT

public:
    ShortcutsTab();
    void retranslateUi();
    ~ShortcutsTab();
private slots:
    void resetShortcuts();
    void refreshEdits();
    void clearShortcuts();
    void afterClear();
private:
    Ui::shortcutsTab *ui;
};

#endif // SHORTCUTSTAB_H
