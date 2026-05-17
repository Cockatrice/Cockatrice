#ifndef COCKATRICE_SHORTCUT_SETTINGS_PAGE_H
#define COCKATRICE_SHORTCUT_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

class SequenceEdit;
class ShortcutTreeView;
class SearchLineEdit;

class ShortcutSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    ShortcutSettingsPage();
    void retranslateUi() override;

private:
    SearchLineEdit *searchEdit;
    ShortcutTreeView *shortcutsTable;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonsLayout;
    QGroupBox *editShortcutGroupBox;
    QGridLayout *editLayout;
    QLabel *currentActionGroupLabel;
    QLabel *currentActionGroupName;
    QLabel *currentActionLabel;
    QLabel *currentActionName;
    QLabel *currentShortcutLabel;
    SequenceEdit *editTextBox;
    QLabel *faqLabel;
    QPushButton *btnResetAll;
    QPushButton *btnClearAll;

private slots:
    void resetShortcuts();
    void clearShortcuts();
    void currentItemChanged(const QString &key);
};

#endif // COCKATRICE_SHORTCUT_SETTINGS_PAGE_H
