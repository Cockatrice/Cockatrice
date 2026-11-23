/**
 * @file dlg_settings.h
 * @brief Main settings dialog for the Cockatrice client (thin frame around SettingsView)
 * @ingroup Dialogs
 */
#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include "settings_view.h"

#include <QDialog>
#include <QLoggingCategory>
#include <qtmetamacros.h>

class QPushButton;
class AbstractSettingsPage;
class FeatureIntroService;

inline Q_LOGGING_CATEGORY(DlgSettingsLog, "dlg_settings");

/**
 * @brief Main application settings dialog hosting the shared SettingsView
 *
 * Thin QDialog frame around SettingsView: only the window title, the OK button
 * and the close validation live here. All pages, navigation and search are
 * owned by SettingsView. The dialog fills ~90% of the main window so dense
 * pages stay visible without losing the modal adjust-and-return flow.
 */
class DlgSettings : public QDialog
{
    Q_OBJECT
public:
    using SettingsPage = SettingsView::SettingsPage;
    static constexpr SettingsPage GeneralPage = SettingsView::GeneralPage;
    static constexpr SettingsPage AppearancePage = SettingsView::AppearancePage;
    static constexpr SettingsPage UserInterfacePage = SettingsView::UserInterfacePage;
    static constexpr SettingsPage DeckEditorPage = SettingsView::DeckEditorPage;
    static constexpr SettingsPage StoragePage = SettingsView::StoragePage;
    static constexpr SettingsPage MessagesPage = SettingsView::MessagesPage;
    static constexpr SettingsPage SoundPage = SettingsView::SoundPage;
    static constexpr SettingsPage ShortcutsPage = SettingsView::ShortcutsPage;
    static constexpr SettingsPage NumPages = SettingsView::NumPages;

    explicit DlgSettings(QWidget *parent = nullptr);
    void setTab(int index);
    /**
     * @brief Opens the page named by @a pageKey with @a query in the search field.
     *
     * @a pageKey is the stable catalog key (`user-interface`, `sound`, …), not a
     * translated title, so an introduction keeps working in every language.
     */
    void openSetting(const QString &pageKey, const QString &query);
    /**
     * @brief Connects the "What's new…" entry points inside the pages.
     *
     * The dialog can be opened from the main window and from a game tab; passing the
     * service here keeps both paths able to show the index without a back-reference
     * from the pages to the main window.
     */
    void setFeatureIntroService(FeatureIntroService *service);
    AbstractSettingsPage *page(SettingsPage which) const;

private slots:
    void updateLanguage();

private:
    SettingsView *settingsView;                   ///< Shared settings content
    QPushButton *okButton;                        ///< Button to close the dialog
    FeatureIntroService *featureIntros = nullptr; ///< Set by the owner that can show What's New

    void openWhatsNew();
    void retranslateUi();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // DLG_SETTINGS_H
