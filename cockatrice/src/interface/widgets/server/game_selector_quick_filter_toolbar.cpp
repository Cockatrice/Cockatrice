#include "game_selector_quick_filter_toolbar.h"

#include "games_model.h"
#include "user/user_list_manager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>

GameSelectorQuickFilterToolBar::GameSelectorQuickFilterToolBar(QWidget *parent,
                                                               TabSupervisor *_tabSupervisor,
                                                               GamesProxyModel *_model,
                                                               const QMap<int, QString> &allGameTypes)
    : QWidget(parent), tabSupervisor(_tabSupervisor), model(_model)
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    searchBar = new QLineEdit(this);
    searchBar->setText(model->getGameNameFilter());
    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString &text) {
        applyFilters([&](auto &, auto &, auto &, auto &, auto &, auto &, auto &, QString &gameNameFilter, auto &,
                         auto &, auto &, auto &, auto &, auto &, auto &, auto &, auto &) { gameNameFilter = text; });
    });

    hideGamesNotCreatedByBuddiesCheckBox = new QCheckBox(this);
    hideGamesNotCreatedByBuddiesCheckBox->setChecked(model->getHideNotBuddyCreatedGames());
    connect(hideGamesNotCreatedByBuddiesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        applyFilters([&](auto &, auto &, auto &, auto &, auto &, bool &hideNotBuddyCreatedGames, auto &, auto &,
                         QStringList &creatorNameFilters, auto &, auto &, auto &, auto &, auto &, auto &, auto &,
                         auto &) {
            hideNotBuddyCreatedGames = checked;

            if (checked) {
                QStringList buddyNames;
                for (auto buddy : tabSupervisor->getUserListManager()->getBuddyList().values()) {
                    buddyNames << QString::fromStdString(buddy.name());
                }
                creatorNameFilters = buddyNames;
            } else {
                creatorNameFilters.clear();
            }
        });
    });

    hideFullGamesCheckBox = new QCheckBox(this);
    hideFullGamesCheckBox->setChecked(model->getHideFullGames());
    connect(hideFullGamesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        applyFilters([&](auto &, auto &, bool &hideFullGames, auto &, auto &, auto &, auto &, auto &, auto &, auto &,
                         auto &, auto &, auto &, auto &, auto &, auto &, auto &) { hideFullGames = checked; });
    });

    hideStartedGamesCheckBox = new QCheckBox(this);
    hideStartedGamesCheckBox->setChecked(model->getHideGamesThatStarted());
    connect(hideStartedGamesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        applyFilters([&](auto &, auto &, auto &, bool &hideGamesThatStarted, auto &, auto &, auto &, auto &, auto &,
                         auto &, auto &, auto &, auto &, auto &, auto &, auto &,
                         auto &) { hideGamesThatStarted = checked; });
    });

    filterToFormatComboBox = new QComboBox(this);

    // Add a "No filter" / "All types" option first
    filterToFormatComboBox->addItem(tr("All types"), QVariant()); // empty QVariant = no filter

    QMapIterator<int, QString> i(allGameTypes);
    while (i.hasNext()) {
        i.next();
        filterToFormatComboBox->addItem(i.value(), i.key()); // text = name, data = type ID
    }

    QSet<int> currentTypes = model->getGameTypeFilter();
    if (currentTypes.size() == 1) {
        int typeId = *currentTypes.begin();
        int index = filterToFormatComboBox->findData(typeId);
        if (index >= 0)
            filterToFormatComboBox->setCurrentIndex(index);
    } else {
        filterToFormatComboBox->setCurrentIndex(0); // "All types" by default
    }

    // Update proxy model on selection change
    connect(filterToFormatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        applyFilters([&](auto &, auto &, auto &, auto &, auto &, auto &, auto &, auto &, auto &,
                         QSet<int> &gameTypeFilter, auto &, auto &, auto &, auto &, auto &, auto &, auto &) {
            QVariant data = filterToFormatComboBox->itemData(index);
            if (!data.isValid()) {
                gameTypeFilter.clear();
            } else {
                gameTypeFilter = {data.toInt()};
            }
        });
    });

    hideGamesNotCreatedByBuddiesCheckBox->setMinimumSize(20, 20);
    hideFullGamesCheckBox->setMinimumSize(20, 20);
    hideStartedGamesCheckBox->setMinimumSize(20, 20);

#if defined(Q_OS_MAC)
    mainLayout->setSpacing(6);
#endif

    mainLayout->addWidget(searchBar);
    mainLayout->addWidget(filterToFormatComboBox);
    mainLayout->addWidget(hideGamesNotCreatedByBuddiesCheckBox);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(hideFullGamesCheckBox);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(hideStartedGamesCheckBox);

    setLayout(mainLayout);

    syncFromModel();

    connect(model, &GamesProxyModel::filtersChanged, this, &GameSelectorQuickFilterToolBar::syncFromModel);

    retranslateUi();
}

void GameSelectorQuickFilterToolBar::syncFromModel()
{
    QSignalBlocker b1(searchBar);
    QSignalBlocker b2(filterToFormatComboBox);
    QSignalBlocker b3(hideGamesNotCreatedByBuddiesCheckBox);
    QSignalBlocker b4(hideFullGamesCheckBox);
    QSignalBlocker b5(hideStartedGamesCheckBox);

    searchBar->setText(model->getGameNameFilter());

    hideGamesNotCreatedByBuddiesCheckBox->setChecked(model->getHideNotBuddyCreatedGames());
    hideFullGamesCheckBox->setChecked(model->getHideFullGames());
    hideStartedGamesCheckBox->setChecked(model->getHideGamesThatStarted());

    QSet<int> types = model->getGameTypeFilter();
    if (types.size() == 1) {
        int idx = filterToFormatComboBox->findData(*types.begin());
        filterToFormatComboBox->setCurrentIndex(idx >= 0 ? idx : 0);
    } else {
        filterToFormatComboBox->setCurrentIndex(0);
    }
}

void GameSelectorQuickFilterToolBar::applyFilters(std::function<void(bool &,
                                                                     bool &,
                                                                     bool &,
                                                                     bool &,
                                                                     bool &,
                                                                     bool &,
                                                                     bool &,
                                                                     QString &,
                                                                     QStringList &,
                                                                     QSet<int> &,
                                                                     int &,
                                                                     int &,
                                                                     QTime &,
                                                                     bool &,
                                                                     bool &,
                                                                     bool &,
                                                                     bool &)> mutator)
{
    bool hideBuddiesOnlyGames = model->getHideBuddiesOnlyGames();
    bool hideIgnoredUserGames = model->getHideIgnoredUserGames();
    bool hideFullGames = model->getHideFullGames();
    bool hideGamesThatStarted = model->getHideGamesThatStarted();
    bool hidePasswordProtectedGames = model->getHidePasswordProtectedGames();
    bool hideNotBuddyCreatedGames = model->getHideNotBuddyCreatedGames();
    bool hideOpenDecklistGames = model->getHideOpenDecklistGames();

    QString gameNameFilter = model->getGameNameFilter();
    QStringList creatorNameFilters = model->getCreatorNameFilters();
    QSet<int> gameTypeFilter = model->getGameTypeFilter();

    int minPlayers = model->getMaxPlayersFilterMin();
    int maxPlayers = model->getMaxPlayersFilterMax();
    QTime maxGameAge = model->getMaxGameAge();

    bool showOnlyIfSpectatorsCanWatch = model->getShowOnlyIfSpectatorsCanWatch();
    bool showSpectatorPasswordProtected = model->getShowSpectatorPasswordProtected();
    bool showOnlyIfSpectatorsCanChat = model->getShowOnlyIfSpectatorsCanChat();
    bool showOnlyIfSpectatorsCanSeeHands = model->getShowOnlyIfSpectatorsCanSeeHands();

    mutator(hideBuddiesOnlyGames, hideIgnoredUserGames, hideFullGames, hideGamesThatStarted, hidePasswordProtectedGames,
            hideNotBuddyCreatedGames, hideOpenDecklistGames, gameNameFilter, creatorNameFilters, gameTypeFilter,
            minPlayers, maxPlayers, maxGameAge, showOnlyIfSpectatorsCanWatch, showSpectatorPasswordProtected,
            showOnlyIfSpectatorsCanChat, showOnlyIfSpectatorsCanSeeHands);

    model->setGameFilters(hideBuddiesOnlyGames, hideIgnoredUserGames, hideFullGames, hideGamesThatStarted,
                          hidePasswordProtectedGames, hideNotBuddyCreatedGames, hideOpenDecklistGames, gameNameFilter,
                          creatorNameFilters, gameTypeFilter, minPlayers, maxPlayers, maxGameAge,
                          showOnlyIfSpectatorsCanWatch, showSpectatorPasswordProtected, showOnlyIfSpectatorsCanChat,
                          showOnlyIfSpectatorsCanSeeHands);
}

void GameSelectorQuickFilterToolBar::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Filter by game name..."));
    filterToFormatComboBox->setToolTip(tr("Filter by game type/format"));
    hideGamesNotCreatedByBuddiesCheckBox->setText(tr("Hide games not created by buddies"));
    hideFullGamesCheckBox->setText(tr("Hide full games"));
    hideStartedGamesCheckBox->setText(tr("Hide started games"));
}
