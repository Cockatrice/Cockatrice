/**
 * @file dlg_settings.h
 * @brief Main settings dialog for the Cockatrice client
 * @ingroup Dialogs
 */
#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QDialog>
#include <QLoggingCategory>

class QPushButton;

inline Q_LOGGING_CATEGORY(DlgSettingsLog, "dlg_settings");

class QStackedWidget;
class QToolButton;
class QListView;
class QLineEdit;

class AbstractSettingsPage;
class SettingsSearchModel;
class SettingsSearchDelegate;

/**
 * @brief Main application settings dialog with tabbed navigation and search
 *
 * Provides a modern settings interface organized into tabbed pages. Users can
 * either navigate by clicking tabs or search for specific settings using the
 * built-in search bar. Search results are filtered and ranked by relevance.
 */
class DlgSettings : public QDialog
{
    Q_OBJECT
public:
    explicit DlgSettings(QWidget *parent = nullptr);
    void setTab(int index);

private slots:
    void onTabClicked(int index);
    void onSearchTextChanged(const QString &text);
    void onSearchResultActivated(const QModelIndex &index);
    void onResetDefaultsClicked();
    void updateLanguage();

private:
    // UI elements
    QLineEdit *searchEdit;           ///< Search bar for filtering settings
    QWidget *tabBarWidget;           ///< Container widget for the tab buttons
    QList<QToolButton *> tabButtons; ///< Navigation tab buttons
    QStackedWidget *pagesWidget;     ///< Stacked widget containing settings pages
    QListView *searchResultsView;    ///< Search results list view
    QWidget *pagesContainer;         ///< Container stacking pages and search results
    QPushButton *resetButton;        ///< Button to reset current page to defaults
    QPushButton *okButton;           ///< Button to close the dialog

    // Data
    QList<AbstractSettingsPage *> pages;    ///< All settings page instances
    SettingsSearchModel *searchModel;       ///< Model for search results
    SettingsSearchDelegate *searchDelegate; ///< Delegate for search result rendering
    int currentTabIndex;                    ///< Currently active tab index
    bool searchActive;                      ///< Whether search mode is active
    QStringList pageNames;                  ///< Translated page names for breadcrumb display

    void setupUi();
    void setupTabBar();
    void buildSearchIndex();
    void switchToTabMode();
    void switchToSearchMode();
    void navigateToSearchResult(const QModelIndex &index);
    void setActiveTab(int index);
    void flashWidget(QWidget *widget);

    void retranslateUi();
    void retranslateTabNames();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // DLG_SETTINGS_H
