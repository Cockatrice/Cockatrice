/**
 * @file dlg_settings.cpp
 * @brief Implementation of the main settings dialog
 * @ingroup Dialogs
 */
#include "dlg_settings.h"

#include "../../../client/settings/cache_settings.h"
#include "../main.h"
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
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_manager.h"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QDir>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSequentialAnimationGroup>
#include <QShortcut>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/personal_settings.h>

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

DlgSettings::DlgSettings(QWidget *parent) : QDialog(parent), currentTabIndex(0), searchActive(false)
{
    auto rec = QGuiApplication::primaryScreen()->availableGeometry();
    setMinimumSize(qMin(750, rec.width()), qMin(700, rec.height()));

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this, &DlgSettings::updateLanguage);

    setupUi();

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this, &DlgSettings::retranslateUi);
    retranslateUi();

    searchEdit->setFocus();

    adjustSize();
}

void DlgSettings::setupUi()
{
    // Search bar
    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(QPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    searchEdit->installEventFilter(this);
    connect(searchEdit, &QLineEdit::textChanged, this, &DlgSettings::onSearchTextChanged);

    auto *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(searchEdit);

    // Tab bar (built in setupTabBar)
    setupTabBar();

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
    pagesWidget->addWidget(messagesPage);
    pagesWidget->addWidget(soundPage);
    pagesWidget->addWidget(shortcutsPage);

    Q_ASSERT(pages.size() == NumPages);

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
    connect(searchResultsView, &QListView::clicked, this, &DlgSettings::onSearchResultClicked);

    connect(&SettingsCache::instance(), &SettingsCache::themeChanged, this, [this] {
        const QStringList icons = pageIconResources();
        for (int i = 0; i < tabButtons.size() && i < icons.size(); ++i) {
            tabButtons[i]->setIcon(QPixmap(icons[i]));
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

    // Bottom buttons
    auto *buttonBox = new QHBoxLayout;
    buttonBox->addStretch();
    okButton = new QPushButton;
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &DlgSettings::close);
    buttonBox->addWidget(okButton);

    // Main layout
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(tabBarWidget);
    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);
    mainLayout->addWidget(pagesContainer);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(buttonBox);
    setLayout(mainLayout);

    // Keyboard shortcuts
    auto *searchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(searchShortcut, &QShortcut::activated, searchEdit, qOverload<>(&QLineEdit::setFocus));

    auto *nextTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), this);
    connect(nextTabShortcut, &QShortcut::activated, this, [this] {
        int next = (currentTabIndex + 1) % tabButtons.size();
        setActiveTab(next);
    });

    auto *prevTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab), this);
    connect(prevTabShortcut, &QShortcut::activated, this, [this] {
        int prev = (currentTabIndex - 1 + tabButtons.size()) % tabButtons.size();
        setActiveTab(prev);
    });

    // Initialize to first tab
    setActiveTab(0);
}

void DlgSettings::setupTabBar()
{
    tabBarWidget = new QWidget;
    auto *tabLayout = new QHBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(2);

    const QStringList iconResources = pageIconResources();

    for (int i = 0; i < iconResources.size(); ++i) {
        auto *tabButton = new QToolButton;
        tabButton->setCheckable(true);
        tabButton->setIcon(QPixmap(iconResources[i]));
        tabButton->setIconSize(QSize(48, 48));
        tabButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        tabButton->setAutoExclusive(true);
        tabButton->setMinimumHeight(85);
        tabButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        connect(tabButton, &QToolButton::clicked, this, [this, idx = i] { onTabClicked(idx); });

        tabButtons.append(tabButton);
        tabLayout->addWidget(tabButton);
    }

    tabBarWidget->setLayout(tabLayout);
}

void DlgSettings::buildSearchIndex()
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

void DlgSettings::onTabClicked(int index)
{
    if (searchActive) {
        switchToTabMode();
    }
    setActiveTab(index);
}

void DlgSettings::setActiveTab(int index)
{
    if (index < 0 || index >= tabButtons.size()) {
        return;
    }

    currentTabIndex = index;
    pagesWidget->setCurrentIndex(index);

    for (int i = 0; i < tabButtons.size(); ++i) {
        tabButtons[i]->setChecked(i == index);
    }

    // Style active tab with a thick accent border + subtle background tint
    for (int i = 0; i < tabButtons.size(); ++i) {
        if (i == index) {
            tabButtons[i]->setStyleSheet("QToolButton { border: none; border-bottom: 3px solid palette(highlight); "
                                         "border-top-left-radius: 4px; border-top-right-radius: 4px; "
                                         "background: palette(window); padding-bottom: 1px; }");
        } else {
            tabButtons[i]->setStyleSheet("QToolButton { border: none; border-bottom: 1px solid transparent; "
                                         "border-top-left-radius: 4px; border-top-right-radius: 4px; "
                                         "background: transparent; }");
        }
    }
}

void DlgSettings::flashWidget(QWidget *widget)
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

void DlgSettings::onSearchTextChanged(const QString &text)
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

void DlgSettings::switchToSearchMode()
{
    searchActive = true;
    tabBarWidget->setVisible(false);
    pagesWidget->setVisible(false);
    searchResultsView->setVisible(true);
    if (searchModel->rowCount(QModelIndex()) > 0) {
        searchResultsView->setCurrentIndex(searchModel->index(0));
    }
}

void DlgSettings::switchToTabMode()
{
    searchActive = false;
    tabBarWidget->setVisible(true);
    pagesWidget->setVisible(true);
    searchResultsView->setVisible(false);
    searchEdit->blockSignals(true);
    searchEdit->clear();
    searchEdit->blockSignals(false);
    setActiveTab(currentTabIndex);
}

void DlgSettings::onSearchResultClicked(const QModelIndex &index)
{
    navigateToSearchResult(index);
}

void DlgSettings::navigateToSearchResult(const QModelIndex &index)
{
    SettingsSearchEntry entry = searchModel->entryForIndex(index);
    if (entry.pageIndex < 0 || entry.pageIndex >= pages.size()) {
        return;
    }

    // Switch to the page
    switchToTabMode();
    setActiveTab(entry.pageIndex);

    // Scroll to the widget, focus it, and flash to highlight it
    if (entry.widget) {
        QWidget *widget = entry.widget;
        while (widget) {
            if (auto *scrollArea = qobject_cast<QScrollArea *>(widget)) {
                scrollArea->ensureWidgetVisible(entry.widget);
                break;
            }
            widget = widget->parentWidget();
        }
        entry.widget->setFocus();
        flashWidget(entry.widget);
    }
}

void DlgSettings::setTab(int index)
{
    if (index >= 0 && index < tabButtons.size()) {
        setActiveTab(index);
    }
}

void DlgSettings::updateLanguage()
{
    qApp->removeTranslator(translator); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    installNewTranslator();
}

bool DlgSettings::eventFilter(QObject *watched, QEvent *event)
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
    return QDialog::eventFilter(watched, event);
}

void DlgSettings::closeEvent(QCloseEvent *event)
{
    bool showLoadError = true;
    QString loadErrorMessage = tr("Unknown Error loading card database");
    LoadStatus loadStatus = CardDatabaseManager::getInstance()->getLoadStatus();
    qCInfo(DlgSettingsLog) << "Card Database load status: " << loadStatus;
    switch (loadStatus) {
        case Ok:
            showLoadError = false;
            break;
        case Invalid:
            loadErrorMessage = tr("Your card database is invalid.\n\n"
                                  "Cockatrice may not function correctly with an invalid database\n\n"
                                  "You may need to rerun oracle to update your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case VersionTooOld:
            loadErrorMessage = tr("Your card database version is too old.\n\n"
                                  "This can cause problems loading card information or images\n\n"
                                  "Usually this can be fixed by rerunning oracle to to update your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case NotLoaded:
            loadErrorMessage = tr("Your card database did not finish loading\n\n"
                                  "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues with your "
                                  "cards.xml attached\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case FileError:
            loadErrorMessage = tr("File Error loading your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case NoCards:
            loadErrorMessage = tr("Your card database was loaded but contains no cards.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        default:
            loadErrorMessage = tr("Unknown card database load status\n\n"
                                  "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues\n\n"
                                  "Would you like to change your database location setting?");

            break;
    }

    if (showLoadError) {
        if (QMessageBox::critical(this, tr("Error"), loadErrorMessage, QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (!QDir(SettingsCache::instance().paths().getDeckPath()).exists() ||
        SettingsCache::instance().paths().getDeckPath().isEmpty()) {
        if (QMessageBox::critical(
                this, tr("Error"),
                tr("The path to your deck directory is invalid. Would you like to go back and set the correct path?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (!QDir(SettingsCache::instance().paths().getPicsPath()).exists() ||
        SettingsCache::instance().paths().getPicsPath().isEmpty()) {
        if (QMessageBox::critical(this, tr("Error"),
                                  tr("The path to your card pictures directory is invalid. Would you like to go back "
                                     "and set the correct path?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void DlgSettings::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    retranslateTabNames();

    searchEdit->setPlaceholderText(tr("Search settings..."));
    okButton->setText(tr("OK"));

    // Rebuild search index for translated text
    buildSearchIndex();
}

QStringList DlgSettings::translatedPageNames()
{
    return {tr("General"), tr("Appearance"), tr("User Interface"), tr("Card Sources"),
            tr("Storage"), tr("Chat"),       tr("Sound"),          tr("Shortcuts")};
}

void DlgSettings::retranslateTabNames()
{
    const QStringList tabLabels = translatedPageNames();

    for (int i = 0; i < tabButtons.size() && i < tabLabels.size(); ++i) {
        tabButtons[i]->setText(tabLabels[i]);
    }
}
