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
    searchBar->setText(model->getCreatorNameFilters().join(", "));
    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString &text) { model->setGameNameFilter(text); });

    hideGamesNotCreatedByBuddiesCheckBox = new QCheckBox(this);
    hideGamesNotCreatedByBuddiesCheckBox->setChecked(model->getHideBuddiesOnlyGames());
    connect(hideGamesNotCreatedByBuddiesCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            QStringList buddyNames;
            for (auto buddy : tabSupervisor->getUserListManager()->getBuddyList().values()) {
                buddyNames << QString::fromStdString(buddy.name());
            }
            model->setCreatorNameFilters(buddyNames);
        } else {
            model->setCreatorNameFilters({});
        }
    });

    hideFullGamesCheckBox = new QCheckBox(this);
    hideFullGamesCheckBox->setChecked(model->getHideFullGames());
    connect(hideFullGamesCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { model->setHideFullGames(checked); });

    hideStartedGamesCheckBox = new QCheckBox(this);
    hideStartedGamesCheckBox->setChecked(model->getHideGamesThatStarted());
    connect(hideStartedGamesCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { model->setHideGamesThatStarted(checked); });

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
        QVariant data = filterToFormatComboBox->itemData(index);
        if (!data.isValid()) {
            model->setGameTypeFilter({}); // empty = no filter
        } else {
            int typeId = data.toInt();
            model->setGameTypeFilter({typeId});
        }
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

    retranslateUi();
}

void GameSelectorQuickFilterToolBar::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Filter by game name..."));
    filterToFormatComboBox->setToolTip(tr("Filter by game type/format"));
    hideGamesNotCreatedByBuddiesCheckBox->setText(tr("Hide games not created by buddies"));
    hideFullGamesCheckBox->setText(tr("Hide full games"));
    hideStartedGamesCheckBox->setText(tr("Hide started games"));
}
