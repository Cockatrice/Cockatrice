#include "game_selector_quick_filter_toolbar.h"

#include "games_model.h"
#include "user/user_list_manager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

GameSelectorQuickFilterToolBar::GameSelectorQuickFilterToolBar(QWidget *parent,
                                                               TabSupervisor *_tabSupervisor,
                                                               GamesProxyModel *_model,
                                                               const QMap<int, QString> &allGameTypes)
    : QWidget(parent), tabSupervisor(_tabSupervisor), model(_model)
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    searchBar = new QLineEdit(this);
    searchBar->setText(model->getCreatorNameFilters().join(", "));
    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString &text) { model->setGameNameFilter(text); });

    hideGamesNotCreatedByBuddiesCheckBox = new QCheckBox(this);
    hideGamesNotCreatedByBuddiesCheckBox->setChecked(model->getHideBuddiesOnlyGames());
    hideGamesNotCreatedByBuddiesCheckBox->setContentsMargins(5, 0, 0, 5);
    hideGamesNotCreatedByBuddiesLabel = new QLabel(this);
    hideGamesNotCreatedByBuddiesLabel->setBuddy(hideGamesNotCreatedByBuddiesCheckBox);
    hideGamesNotCreatedByBuddiesLabel->setContentsMargins(0, 0, 5, 0);
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
    hideFullGamesCheckBox->setContentsMargins(5, 0, 0, 0);
    hideFullGamesLabel = new QLabel(this);
    hideFullGamesLabel->setBuddy(hideFullGamesCheckBox);
    hideFullGamesLabel->setContentsMargins(0, 0, 5, 0);
    connect(hideFullGamesCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { model->setHideFullGames(checked); });

    hideStartedGamesCheckBox = new QCheckBox(this);
    hideStartedGamesCheckBox->setChecked(model->getHideGamesThatStarted());
    hideStartedGamesCheckBox->setContentsMargins(5, 0, 0, 0);
    hideStartedGamesLabel = new QLabel(this);
    hideStartedGamesLabel->setBuddy(hideStartedGamesCheckBox);
    hideStartedGamesLabel->setContentsMargins(0, 0, 5, 0);
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

    mainLayout->addWidget(searchBar);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(filterToFormatComboBox);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(hideGamesNotCreatedByBuddiesCheckBox);
    mainLayout->addWidget(hideGamesNotCreatedByBuddiesLabel);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(hideFullGamesCheckBox);
    mainLayout->addWidget(hideFullGamesLabel);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(hideStartedGamesCheckBox);
    mainLayout->addWidget(hideStartedGamesLabel);

    setLayout(mainLayout);

    retranslateUi();
}

void GameSelectorQuickFilterToolBar::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Filter by game name..."));
    filterToFormatComboBox->setToolTip(tr("Filter by game type/format"));
    hideGamesNotCreatedByBuddiesLabel->setText(tr("Hide games not created by buddies"));
    hideFullGamesLabel->setText(tr("Hide full games"));
    hideStartedGamesLabel->setText(tr("Hide started games"));
}
