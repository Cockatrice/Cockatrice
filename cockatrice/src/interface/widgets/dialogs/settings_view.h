/**
 * @file settings_view.h
 * @brief Settings content widget hosted by the settings dialog (and, later, the settings tab)
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
class QTreeWidget;
class QTreeWidgetItem;
class QListView;
class QLineEdit;
class AbstractSettingsPage;
class SettingsSearchModel;
class SettingsSearchDelegate;

/**
 * @brief Settings pages with grouped left navigation and search, without any dialog chrome
 *
 * Owns the search bar, the navigation tree, the stacked settings pages and the search
 * results view. Dialog/tab hosts only add their own frame (buttons, titles).
 */
class SettingsView : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Page order in the navigation, matching the order pages are added in setupUi()
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
    void updateLanguage();

private:
    // UI elements
    QLineEdit *searchEdit;               ///< Search bar for filtering settings
    QTreeWidget *navTree;                ///< Grouped navigation tree for the pages
    QList<QTreeWidgetItem *> groupItems; ///< Group header rows, indexed by group
    QList<QTreeWidgetItem *> pageItems;  ///< Page rows, indexed by SettingsPage
    QStackedWidget *pagesWidget;         ///< Stacked widget containing settings pages
    QListView *searchResultsView;        ///< Search results list view
    QWidget *pagesContainer;             ///< Container stacking pages and search results

    // Data
    QList<AbstractSettingsPage *> pages;    ///< All settings page instances
    SettingsSearchModel *searchModel;       ///< Model for search results
    SettingsSearchDelegate *searchDelegate; ///< Delegate for search result rendering
    int currentTabIndex;                    ///< Currently active tab index
    bool searchActive;                      ///< Whether search mode is active

    void setupUi();
    void setupNavTree();
    void buildSearchIndex();
    void switchToTabMode();
    void switchToSearchMode();
    void navigateToSearchResult(const QModelIndex &index);
    void navigateToSection(int pageIndex, QWidget *sectionWidget);
    void setActiveTab(int index);
    static void flashWidget(QWidget *widget);
    static void scrollToAndFlash(QWidget *target);
    static QStringList translatedPageNames();
    static QStringList translatedGroupNames();

    void retranslateUi();
    void retranslateNavNames();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SETTINGS_VIEW_H
