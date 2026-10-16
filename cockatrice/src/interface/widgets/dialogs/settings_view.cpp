/**
 * @file settings_view.cpp
 * @brief Implementation of the settings content widget
 * @ingroup Dialogs
 */
#include "settings_view.h"

#include "../../../client/settings/cache_settings.h"
#include "../../pixel_map_generator.h"
#include "../settings_page/abstract_settings_page.h"
#include "../settings_page/appearance_settings_page.h"
#include "../settings_page/deck_editor_settings_page.h"
#include "../settings_page/general_settings_page.h"
#include "../settings_page/messages_settings_page.h"
#include "../settings_page/settings_search_delegate.h"
#include "../settings_page/settings_search_model.h"
#include "../settings_page/shortcut_settings_page.h"
#include "../settings_page/sound_settings_page.h"
#include "../settings_page/storage_settings_page.h"
#include "../settings_page/user_interface_settings_page.h"

#include <QAbstractAnimation>
#include <QAbstractItemView>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QEasingCurve>
#include <QEvent>
#include <QFlags>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QListView>
#include <QMessageLogger>
#include <QModelIndex>
#include <QObject>
#include <QPalette>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSequentialAnimationGroup>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSize>
#include <QSizePolicy>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QStringLiteral>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <QtGlobal>
#include <libcockatrice/settings/personal_settings.h>
#include <qnamespace.h>

/**
 * @brief Wraps a widget in a scroll area for long settings pages
 * @param widget The widget to wrap
 * @return The scroll area containing the widget
 */
static QScrollArea *makeScrollable(QWidget *widget)
{
    widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->horizontalScrollBar()->setEnabled(false);
    scrollArea->setWidget(widget);
    return scrollArea;
}

/**
 * @brief Returns the theme icon resources for each settings page, indexed by SettingsPage order
 */
static QStringList pageIconResources()
{
    return {QStringLiteral("theme:config/general"),   QStringLiteral("theme:config/appearance"),
            QStringLiteral("theme:config/interface"), QStringLiteral("theme:config/deckeditor"),
            QStringLiteral("theme:config/storage"),   QStringLiteral("theme:config/messages"),
            QStringLiteral("theme:config/sound"),     QStringLiteral("theme:config/shorcuts")};
}

namespace
{
/// Item data role holding a sub-page row's section widget, stored as a WidgetId
constexpr int SectionWidgetRole = Qt::UserRole + 1;
using WidgetId = quintptr;

/**
 * @brief Packs a section widget for storage in a tree item's data role
 * @param widget The section's group box (owned by its settings page)
 * @return The widget as an opaque id
 */
WidgetId toWidgetId(QWidget *widget)
{
    return reinterpret_cast<WidgetId>(widget);
}

/**
 * @brief Unpacks a section widget from a tree item's data role
 * @param id The stored id, or 0 for page rows without a section
 * @return The section widget, or nullptr when unset
 */
QWidget *fromWidgetId(WidgetId id)
{
    return reinterpret_cast<QWidget *>(id);
}
} // namespace

SettingsView::SettingsView(QWidget *parent) : QWidget(parent), currentTabIndex(0), searchActive(false)
{
    setupUi();

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this, &SettingsView::retranslateUi);
    retranslateUi();

    searchEdit->setFocus();
}

void SettingsView::setupUi()
{
    // Search bar
    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(themePixmap(QStringLiteral("icons/search")), QLineEdit::LeadingPosition);
    searchEdit->installEventFilter(this);
    connect(searchEdit, &QLineEdit::textChanged, this, &SettingsView::onSearchTextChanged);

    auto *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(searchEdit);

    // Pages stacked widget
    pagesWidget = new QStackedWidget;

    auto *generalPage = new GeneralSettingsPage;
    auto *appearancePage = new AppearanceSettingsPage;
    auto *userInterfacePage = new UserInterfaceSettingsPage;
    auto *deckEditorPage = new DeckEditorSettingsPage;
    auto *storagePage = new StorageSettingsPage;
    auto *messagesPage = new MessagesSettingsPage;
    auto *soundPage = new SoundSettingsPage;
    auto *shortcutsPage = new ShortcutSettingsPage;

    pages.append(generalPage);
    pages.append(appearancePage);
    pages.append(userInterfacePage);
    pages.append(deckEditorPage);
    pages.append(storagePage);
    pages.append(messagesPage);
    pages.append(soundPage);
    pages.append(shortcutsPage);

    pagesWidget->addWidget(makeScrollable(generalPage));
    pagesWidget->addWidget(makeScrollable(appearancePage));
    pagesWidget->addWidget(makeScrollable(userInterfacePage));
    pagesWidget->addWidget(makeScrollable(deckEditorPage));
    pagesWidget->addWidget(makeScrollable(storagePage));
    pagesWidget->addWidget(makeScrollable(messagesPage));
    pagesWidget->addWidget(soundPage);
    pagesWidget->addWidget(shortcutsPage);

    Q_ASSERT(pages.size() == NumPages);

    // Navigation tree (needs the pages for their sections)
    setupNavTree();

    // Search results view (hidden by default)
    searchResultsView = new QListView;
    searchResultsView->setUniformItemSizes(false);
    searchResultsView->setSelectionMode(QAbstractItemView::SingleSelection);
    searchResultsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    searchResultsView->setVisible(false);
    searchResultsView->setStyleSheet(
        "QListView::item:selected { background: palette(highlight); color: palette(highlighted-text); }");

    searchModel = new SettingsSearchModel(this);
    searchDelegate = new SettingsSearchDelegate(this);
    searchResultsView->setModel(searchModel);
    searchResultsView->setItemDelegate(searchDelegate);
    connect(searchResultsView, &QListView::clicked, this, &SettingsView::onSearchResultClicked);

    connect(&SettingsCache::instance(), &SettingsCache::themeChanged, this, [this] {
        const QStringList icons = pageIconResources();
        for (int i = 0; i < pageItems.size() && i < icons.size(); ++i) {
            if (pageItems[i]) {
                pageItems[i]->setIcon(0, QPixmap(icons[i]));
            }
        }
        searchDelegate->setPageIcons(icons);
        searchResultsView->viewport()->update();
    });

    // Build search index after pages are created
    buildSearchIndex();

    // Pages container (stacked widget + search results overlay)
    pagesContainer = new QWidget;
    auto *containerLayout = new QStackedLayout;
    containerLayout->setStackingMode(QStackedLayout::StackAll);
    containerLayout->addWidget(pagesWidget);
    containerLayout->addWidget(searchResultsView);
    pagesContainer->setLayout(containerLayout);

    // Main layout: search on top, navigation + pages side by side below
    auto *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(navTree);
    contentLayout->addWidget(pagesContainer, 1);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(contentLayout);
    setLayout(mainLayout);

    // Keyboard shortcuts
    auto *searchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(searchShortcut, &QShortcut::activated, searchEdit, qOverload<>(&QLineEdit::setFocus));

    auto *nextTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), this);
    connect(nextTabShortcut, &QShortcut::activated, this, [this] {
        int next = (currentTabIndex + 1) % pages.size();
        setActiveTab(next);
    });

    auto *prevTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab), this);
    connect(prevTabShortcut, &QShortcut::activated, this, [this] {
        int prev = (currentTabIndex - 1 + pages.size()) % pages.size();
        setActiveTab(prev);
    });

    // Initialize to first tab
    setActiveTab(0);
}

void SettingsView::setupNavTree()
{
    navTree = new QTreeWidget;
    navTree->setHeaderHidden(true);
    navTree->setRootIsDecorated(true);
    navTree->setIconSize(QSize(24, 24));
    navTree->setMinimumWidth(230);
    navTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navTree->setStyleSheet(
        "QTreeWidget::item:selected { background: palette(highlight); color: palette(highlighted-text); }");

    // Groups of SettingsPage values, in navigation order
    static const QList<QList<SettingsPage>> groupedPages = {
        {GeneralPage, AppearancePage, UserInterfacePage},
        {DeckEditorPage, StoragePage},
        {MessagesPage, SoundPage},
        {ShortcutsPage},
    };

    const QStringList groupNames = translatedGroupNames();
    const QStringList pageNames = translatedPageNames();
    const QStringList iconResources = pageIconResources();

    pageItems.resize(NumPages);
    for (int i = 0; i < groupedPages.size(); ++i) {
        auto *group = new QTreeWidgetItem(navTree, {groupNames[i]});
        group->setFlags(Qt::ItemIsEnabled); // header row: expandable, not selectable
        group->setExpanded(true);
        groupItems.append(group);
        for (SettingsPage page : groupedPages[i]) {
            auto *item = new QTreeWidgetItem(group, {pageNames[page]});
            item->setIcon(0, QPixmap(iconResources[page]));
            item->setData(0, Qt::UserRole, static_cast<int>(page));
            pageItems[page] = item;
            // Sub-page rows deep-link into the page's sections (scroll + highlight)
            for (const SettingsPageSection &section : pages[page]->getSections()) {
                auto *sectionItem = new QTreeWidgetItem(item, {section.title});
                sectionItem->setData(0, Qt::UserRole, static_cast<int>(page));
                sectionItem->setData(0, SectionWidgetRole, QVariant::fromValue(toWidgetId(section.sectionWidget)));
            }
        }
    }

    Q_ASSERT(groupItems.size() == groupedPages.size());
    Q_ASSERT(!pageItems.contains(nullptr));

    connect(navTree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem *current) {
        // Any selection change consumes a preceding press: only a press without a
        // following change counts as a re-click (handled in itemClicked)
        pressWasOnCurrentItem = false;
        if (!current) {
            return;
        }
        bool ok = false;
        int index = current->data(0, Qt::UserRole).toInt(&ok);
        if (!ok) {
            return;
        }
        if (QWidget *section = fromWidgetId(current->data(0, SectionWidgetRole).value<WidgetId>())) {
            navigateToSection(index, section);
        } else {
            onTabClicked(index);
        }
    });

    connect(navTree, &QTreeWidget::itemPressed, this, [this](QTreeWidgetItem *item) {
        // Left button only: context presses must never toggle the filter
        pressWasOnCurrentItem =
            QApplication::mouseButtons() == Qt::LeftButton && item != nullptr && item == navTree->currentItem();
    });

    connect(navTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        const bool wasReClick = pressWasOnCurrentItem;
        pressWasOnCurrentItem = false;
        // A fresh click already navigated via currentItemChanged; only a press that
        // started on the current item is a re-click. Re-clicking the isolated
        // section restores the full page.
        if (!wasReClick || item == nullptr || item != navTree->currentItem() || isolatedSection == nullptr ||
            fromWidgetId(item->data(0, SectionWidgetRole).value<WidgetId>()) != isolatedSection) {
            return;
        }
        bool ok = false;
        const int index = item->data(0, Qt::UserRole).toInt(&ok);
        if (ok) {
            // setActiveTab, not onTabClicked: the page is already current and
            // only setActiveTab clears the section filter
            setActiveTab(index);
        }
    });
}

void SettingsView::buildSearchIndex()
{
    QList<SettingsSearchEntry> allEntries;

    const QStringList pageNames = translatedPageNames();
    searchDelegate->setPageNames(pageNames);
    searchDelegate->setPageIcons(pageIconResources());

    for (int i = 0; i < pages.size(); ++i) {
        QList<SettingsSearchEntry> pageEntries = pages[i]->getSearchEntries();
        for (auto &entry : pageEntries) {
            if (entry.pageIndex == -1) {
                entry.pageIndex = i;
            }
        }
        allEntries.append(pageEntries);
    }

    searchModel->setSourceEntries(allEntries);
}

void SettingsView::onTabClicked(int index)
{
    if (searchActive) {
        switchToTabMode();
    } else if (index == currentTabIndex) {
        return;
    }
    setActiveTab(index);
}

void SettingsView::setActiveTab(int index)
{
    if (index < 0 || index >= pageItems.size() || !pageItems[index]) {
        return;
    }

    switchPageContent(index);

    // Selecting the item also expands a collapsed group; selecting the already
    // current item emits nothing, so the currentItemChanged handler cannot loop.
    navTree->setCurrentItem(pageItems[index]);
}

void SettingsView::switchPageContent(int index)
{
    if (index < 0 || index >= pageItems.size() || !pageItems[index]) {
        return;
    }

    currentTabIndex = index;
    pagesWidget->setCurrentIndex(index);

    // A page row always shows the full page; sections filter down from here
    applySectionFilter(index, nullptr);

    if (QTreeWidgetItem *parent = pageItems[index]->parent()) {
        parent->setExpanded(true);
    }
}

void SettingsView::flashWidget(QWidget *widget)
{
    auto *overlay = new QWidget(widget);
    overlay->setGeometry(widget->rect());
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QPalette pal = overlay->palette();
    QColor flashColor = pal.color(QPalette::Highlight);
    flashColor.setAlpha(100);
    pal.setBrush(QPalette::Window, flashColor);
    overlay->setPalette(pal);
    overlay->setAutoFillBackground(true);

    auto *effect = new QGraphicsOpacityEffect(overlay);
    effect->setOpacity(0.0);
    overlay->setGraphicsEffect(effect);
    overlay->show();
    overlay->raise();

    auto *flashIn = new QPropertyAnimation(effect, "opacity");
    flashIn->setDuration(120);
    flashIn->setStartValue(0.0);
    flashIn->setEndValue(0.6);
    flashIn->setEasingCurve(QEasingCurve::OutCubic);

    auto *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(900);
    fadeOut->setStartValue(0.6);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    auto *group = new QSequentialAnimationGroup(overlay);
    group->addAnimation(flashIn);
    group->addAnimation(fadeOut);

    connect(group, &QSequentialAnimationGroup::finished, overlay, &QWidget::deleteLater);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void SettingsView::onSearchTextChanged(const QString &text)
{
    searchModel->setFilterString(text);

    if (searchModel->isFilterActive() && !text.trimmed().isEmpty()) {
        if (!searchActive) {
            switchToSearchMode();
        }
        if (searchModel->rowCount(QModelIndex()) > 0) {
            searchResultsView->setCurrentIndex(searchModel->index(0));
        }
    } else if (searchActive) {
        switchToTabMode();
    }
}

void SettingsView::switchToSearchMode()
{
    searchActive = true;
    navTree->setVisible(false);
    pagesWidget->setVisible(false);
    searchResultsView->setVisible(true);
    if (searchModel->rowCount(QModelIndex()) > 0) {
        searchResultsView->setCurrentIndex(searchModel->index(0));
    }
}

void SettingsView::switchToTabMode()
{
    searchActive = false;
    navTree->setVisible(true);
    pagesWidget->setVisible(true);
    searchResultsView->setVisible(false);
    searchEdit->blockSignals(true);
    searchEdit->clear();
    searchEdit->blockSignals(false);
    setActiveTab(currentTabIndex);
}

void SettingsView::onSearchResultClicked(const QModelIndex &index)
{
    navigateToSearchResult(index);
}

void SettingsView::navigateToSearchResult(const QModelIndex &index)
{
    SettingsSearchEntry entry = searchModel->entryForIndex(index);
    if (entry.pageIndex < 0 || entry.pageIndex >= pages.size()) {
        return;
    }

    // Switch to the page
    switchToTabMode();
    setActiveTab(entry.pageIndex);

    // Isolate the control's section, then scroll to the widget, focus it, and flash it
    if (entry.widget) {
        QWidget *section = nullptr;
        for (const SettingsPageSection &candidate : pages[entry.pageIndex]->getSections()) {
            if (candidate.sectionWidget == entry.widget ||
                (candidate.sectionWidget && candidate.sectionWidget->isAncestorOf(entry.widget))) {
                section = candidate.sectionWidget;
                break;
            }
        }
        applySectionFilter(entry.pageIndex, section);
        scrollToAndFlash(entry.widget);
        entry.widget->setFocus();
    }
}

void SettingsView::navigateToSection(int pageIndex, QWidget *sectionWidget)
{
    if (pageIndex < 0 || pageIndex >= pages.size() || !sectionWidget) {
        return;
    }

    // Never steal the view's own selection mid-gesture: when this runs from
    // currentItemChanged the section row is already current (mouse press or arrow key),
    // and re-selecting the page row here is what made the selection visibly snap back
    // to the parent on mouse release. Switch page content without touching selection,
    // then only sync selection for programmatic callers where the section is not current yet.
    QTreeWidgetItem *sectionItem = findSectionItem(pageIndex, sectionWidget);

    if (currentTabIndex != pageIndex) {
        if (searchActive) {
            switchToTabMode();
        }
        const QSignalBlocker blocker(navTree);
        switchPageContent(pageIndex);
    }
    applySectionFilter(pageIndex, sectionWidget);

    // Unknown widgets show the full page; known sections keep their row selected
    if (sectionItem && pageIndex >= 0 && pageIndex < pageItems.size() && pageItems[pageIndex]) {
        pageItems[pageIndex]->setExpanded(true);
        if (navTree->currentItem() != sectionItem) {
            const QSignalBlocker blocker(navTree);
            navTree->setCurrentItem(sectionItem);
        }
        flashWidget(sectionWidget);
    }
}

/**
 * @brief Finds the nav row for a section widget
 * @param pageIndex SettingsPage owning the section
 * @param sectionWidget Section's group box
 * @return The section's tree item, or nullptr when it has no row
 */
QTreeWidgetItem *SettingsView::findSectionItem(int pageIndex, QWidget *sectionWidget) const
{
    if (pageIndex < 0 || pageIndex >= pageItems.size() || !pageItems[pageIndex]) {
        return nullptr;
    }
    for (int c = 0; c < pageItems[pageIndex]->childCount(); ++c) {
        QTreeWidgetItem *child = pageItems[pageIndex]->child(c);
        if (fromWidgetId(child->data(0, SectionWidgetRole).value<WidgetId>()) == sectionWidget) {
            return child;
        }
    }
    return nullptr;
}

/**
 * @brief Shows only the given section of a page, hiding its siblings
 *
 * A null sectionWidget restores the full page. Unknown widgets fall back to
 * the full page so a stale reference can never leave a blank page.
 *
 * @param pageIndex SettingsPage to filter
 * @param sectionWidget Section's group box, or nullptr to show all sections
 */
void SettingsView::applySectionFilter(int pageIndex, QWidget *sectionWidget)
{
    if (pageIndex < 0 || pageIndex >= pages.size()) {
        return;
    }

    const QList<SettingsPageSection> sections = pages[pageIndex]->getSections();

    bool known = sectionWidget == nullptr;
    for (const SettingsPageSection &section : sections) {
        if (section.sectionWidget == sectionWidget) {
            known = true;
        }
    }
    if (!known) {
        sectionWidget = nullptr;
    }
    isolatedSection = sectionWidget;

    for (const SettingsPageSection &section : sections) {
        if (section.sectionWidget) {
            section.sectionWidget->setVisible(sectionWidget == nullptr || section.sectionWidget == sectionWidget);
        }
    }

    if (auto *scrollArea = qobject_cast<QScrollArea *>(pagesWidget->widget(pageIndex))) {
        if (QScrollBar *scrollBar = scrollArea->verticalScrollBar()) {
            scrollBar->setValue(0);
        }
    }
}

void SettingsView::scrollToAndFlash(QWidget *target)
{
    if (!target) {
        return;
    }
    QWidget *widget = target;
    while (widget) {
        if (auto *scrollArea = qobject_cast<QScrollArea *>(widget)) {
            scrollArea->ensureWidgetVisible(target);
            break;
        }
        widget = widget->parentWidget();
    }
    flashWidget(target);
}

void SettingsView::setTab(int index)
{
    if (index >= 0 && index < pageItems.size()) {
        setActiveTab(index);
    }
}

AbstractSettingsPage *SettingsView::page(SettingsPage which) const
{
    return pages.value(static_cast<int>(which));
}

bool SettingsView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == searchEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            if (searchActive) {
                switchToTabMode();
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (searchActive) {
                if (searchResultsView->currentIndex().isValid()) {
                    navigateToSearchResult(searchResultsView->currentIndex());
                }
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Down) {
            if (searchActive) {
                int nextRow = searchResultsView->currentIndex().row() + 1;
                if (nextRow >= searchModel->rowCount()) {
                    nextRow = 0;
                }
                searchResultsView->setCurrentIndex(searchModel->index(nextRow));
                searchResultsView->scrollTo(searchModel->index(nextRow));
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Up) {
            if (searchActive) {
                int prevRow = searchResultsView->currentIndex().row() - 1;
                if (prevRow < 0) {
                    prevRow = searchModel->rowCount() - 1;
                }
                if (prevRow >= 0) {
                    searchResultsView->setCurrentIndex(searchModel->index(prevRow));
                    searchResultsView->scrollTo(searchModel->index(prevRow));
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SettingsView::retranslateUi()
{
    retranslateNavNames();

    // NOTE: the "DlgSettings" context is deliberate — these strings lived in DlgSettings
    // before the view was extracted, and keeping the context preserves existing translations.
    searchEdit->setPlaceholderText(QCoreApplication::translate("DlgSettings", "Search settings..."));

    // Rebuild search index for translated text
    buildSearchIndex();
}

QStringList SettingsView::translatedPageNames()
{
    // NOTE: same "DlgSettings" context as above (translation continuity, see retranslateUi).
    return {QCoreApplication::translate("DlgSettings", "General"),
            QCoreApplication::translate("DlgSettings", "Appearance"),
            QCoreApplication::translate("DlgSettings", "User Interface"),
            QCoreApplication::translate("DlgSettings", "Card Sources"),
            QCoreApplication::translate("DlgSettings", "Storage"),
            QCoreApplication::translate("DlgSettings", "Chat"),
            QCoreApplication::translate("DlgSettings", "Sound"),
            QCoreApplication::translate("DlgSettings", "Shortcuts")};
}

void SettingsView::retranslateNavNames()
{
    const QStringList groupNames = translatedGroupNames();
    for (int i = 0; i < groupItems.size() && i < groupNames.size(); ++i) {
        groupItems[i]->setText(0, groupNames[i]);
    }

    const QStringList pageNames = translatedPageNames();
    for (int i = 0; i < pageItems.size() && i < pageNames.size(); ++i) {
        if (pageItems[i]) {
            pageItems[i]->setText(0, pageNames[i]);
        }
    }

    // Sub-page rows mirror the pages' group box titles (already retranslated by the pages)
    for (int i = 0; i < pageItems.size() && i < pages.size(); ++i) {
        if (!pageItems[i]) {
            continue;
        }
        const QList<SettingsPageSection> sections = pages[i]->getSections();
        Q_ASSERT(pageItems[i]->childCount() == sections.size());
        for (int c = 0; c < pageItems[i]->childCount() && c < sections.size(); ++c) {
            pageItems[i]->child(c)->setText(0, sections[c].title);
        }
    }
}

QStringList SettingsView::translatedGroupNames()
{
    return {tr("Application"), tr("Cards & Data"), tr("Chat & Sound"), tr("Advanced")};
}
