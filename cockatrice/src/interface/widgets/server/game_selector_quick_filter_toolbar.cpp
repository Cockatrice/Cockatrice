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

    const GameFilterConfigs &filters = model->getFilters();

    searchBar = new QLineEdit(this);
    searchBar->setText(filters.gameNameFilter);
    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString &text) {
        applyFilters([&](GameFilterConfigs &configs) { configs.gameNameFilter = text; });
    });

    hideGamesNotCreatedByBuddiesCheckBox = new QCheckBox(this);
    hideGamesNotCreatedByBuddiesCheckBox->setChecked(filters.hideNotBuddyCreatedGames);
    connect(hideGamesNotCreatedByBuddiesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        applyFilters([&](GameFilterConfigs &configs) { configs.hideNotBuddyCreatedGames = checked; });
    });

    hideFullGamesCheckBox = new QCheckBox(this);
    hideFullGamesCheckBox->setChecked(filters.hideFullGames);
    connect(hideFullGamesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        applyFilters([&](GameFilterConfigs &configs) { configs.hideFullGames = checked; });
    });

    hideStartedGamesCheckBox = new QCheckBox(this);
    hideStartedGamesCheckBox->setChecked(filters.hideGamesThatStarted);
    connect(hideStartedGamesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        applyFilters([&](GameFilterConfigs &configs) { configs.hideGamesThatStarted = checked; });
    });

    filterToFormatComboBox = new QComboBox(this);

    // Add a "No filter" / "All types" option first
    filterToFormatComboBox->addItem(tr("All types"), QVariant()); // empty QVariant = no filter

    QMapIterator<int, QString> i(allGameTypes);
    while (i.hasNext()) {
        i.next();
        filterToFormatComboBox->addItem(i.value(), i.key()); // text = name, data = type ID
    }

    QSet<int> currentTypes = filters.gameTypeFilter;
    if (currentTypes.size() == 1) {
        int typeId = *currentTypes.begin();
        int index = filterToFormatComboBox->findData(typeId);
        if (index >= 0) {
            filterToFormatComboBox->setCurrentIndex(index);
        }
    } else {
        filterToFormatComboBox->setCurrentIndex(0); // "All types" by default
    }

    // Update proxy model on selection change
    connect(filterToFormatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        applyFilters([&](GameFilterConfigs &configs) {
            QVariant data = filterToFormatComboBox->itemData(index);
            if (!data.isValid()) {
                configs.gameTypeFilter.clear();
            } else {
                configs.gameTypeFilter = {data.toInt()};
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

    const GameFilterConfigs filters = model->getFilters();

    searchBar->setText(filters.gameNameFilter);

    hideGamesNotCreatedByBuddiesCheckBox->setChecked(filters.hideNotBuddyCreatedGames);
    hideFullGamesCheckBox->setChecked(filters.hideFullGames);
    hideStartedGamesCheckBox->setChecked(filters.hideGamesThatStarted);

    QSet<int> types = filters.gameTypeFilter;
    if (types.size() == 1) {
        int idx = filterToFormatComboBox->findData(*types.begin());
        filterToFormatComboBox->setCurrentIndex(idx >= 0 ? idx : 0);
    } else {
        filterToFormatComboBox->setCurrentIndex(0);
    }
}

void GameSelectorQuickFilterToolBar::applyFilters(std::function<void(GameFilterConfigs &)> mutator)
{
    GameFilterConfigs configs = model->getFilters();

    mutator(configs);

    model->setGameFilters(configs);
}

void GameSelectorQuickFilterToolBar::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Filter by game name..."));
    filterToFormatComboBox->setToolTip(tr("Filter by game type/format"));
    hideGamesNotCreatedByBuddiesCheckBox->setText(tr("Hide games not created by buddies"));
    hideFullGamesCheckBox->setText(tr("Hide full games"));
    hideStartedGamesCheckBox->setText(tr("Hide started games"));
}
