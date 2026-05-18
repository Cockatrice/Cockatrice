/**
 * @file dlg_settings.h
 * @ingroup Dialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QComboBox>
#include <QDialog>
#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(DlgSettingsLog, "dlg_settings");

class QListWidget;
class QStackedWidget;
class QListWidgetItem;

class DlgSettings : public QDialog
{
    Q_OBJECT
public:
    explicit DlgSettings(QWidget *parent = nullptr);
    void setTab(int index);

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void updateLanguage();

private:
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    QListWidgetItem *generalButton, *appearanceButton, *userInterfaceButton, *deckEditorButton, *storageButton,
        *messagesButton, *soundButton, *shortcutsButton;
    void createIcons();
    void retranslateUi();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif
