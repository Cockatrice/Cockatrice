/**
 * @file settings_view.h
 * @brief Settings content widget hosted by the settings dialog
 * @ingroup Dialogs
 */
#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <qtmetamacros.h>

class QModelIndex;
class QObject;
class QEvent;

class QStackedWidget;
class QToolButton;
class QListView;
class QLineEdit;
class AbstractSettingsPage;
class SettingsSearchModel;
class SettingsSearchDelegate;

/**
 * @brief Settings pages with tabbed navigation and search, without any dialog chrome
 *
 * Owns the search bar, the tab bar, the stacked settings pages and the search
 * results view. The settings dialog only adds its own frame (buttons, titles).
 */
class SettingsView : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Page order in the tab bar, matching the order pages are added in setupUi()
     *
     * Use these values instead of raw indices so reordering pages never silently
     * breaks external callers like tab_room.cpp.
     */
    enum SettingsPage
    {
        GeneralPage = 0,
        AppearancePage,
        UserInterfacePage,
        DeckEditorPage,
        StoragePage,
        MessagesPage,
        SoundPage,
        ShortcutsPage,
        NumPages
    };

    explicit SettingsView(QWidget *parent = nullptr);
    void setTab(int index);
    AbstractSettingsPage *page(SettingsPage which) const;

private slots:
    void onTabClicked(int index);
    void onSearchTextChanged(const QString &text);
    void onSearchResultClicked(const QModelIndex &index);

private:
    // UI elements
    QLineEdit *searchEdit;           ///< Search bar for filtering settings
    QWidget *tabBarWidget;           ///< Container widget for the tab buttons
    QList<QToolButton *> tabButtons; ///< Navigation tab buttons
    QStackedWidget *pagesWidget;     ///< Stacked widget containing settings pages
    QListView *searchResultsView;    ///< Search results list view
    QWidget *pagesContainer;         ///< Container stacking pages and search results

    // Data
    QList<AbstractSettingsPage *> pages;    ///< All settings page instances
    SettingsSearchModel *searchModel;       ///< Model for search results
    SettingsSearchDelegate *searchDelegate; ///< Delegate for search result rendering
    int currentTabIndex;                    ///< Currently active tab index
    bool searchActive;                      ///< Whether search mode is active

    void setupUi();
    void setupTabBar();
    void buildSearchIndex();
    void switchToTabMode();
    void switchToSearchMode();
    void navigateToSearchResult(const QModelIndex &index);
    void setActiveTab(int index);
    static void flashWidget(QWidget *widget);
    static QStringList translatedPageNames();

    void retranslateUi();
    void retranslateTabNames();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SETTINGS_VIEW_H
