#include "game_selector_quick_filter_toolbar.h"

#include "games_model.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

GameSelectorQuickFilterToolBar::GameSelectorQuickFilterToolBar(QWidget *parent,
                                                               GamesProxyModel *_model,
                                                               const QMap<int, QString> &allGameTypes)
    : QWidget(parent), model(_model)
{
    mainLayout = new QHBoxLayout(this);

    searchBar = new QLineEdit(this);
    searchBar->setText(model->getCreatorNameFilter());
    connect(searchBar, &QLineEdit::textChanged, this,
            [this](const QString &text) { model->setCreatorNameFilter(text); });

    hideGamesWithoutBuddiesCheckBox = new QCheckBox(this);
    hideGamesWithoutBuddiesCheckBox->setChecked(model->getHideBuddiesOnlyGames());
    hideGamesWithoutBuddiesLabel = new QLabel(this);
    hideGamesWithoutBuddiesLabel->setBuddy(hideGamesWithoutBuddiesCheckBox);
    connect(hideGamesWithoutBuddiesCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { model->setHideBuddiesOnlyGames(checked); });

    hideFullGamesCheckBox = new QCheckBox(this);
    hideFullGamesCheckBox->setChecked(model->getHideFullGames());
    hideFullGamesLabel = new QLabel(this);
    hideFullGamesLabel->setBuddy(hideFullGamesCheckBox);
    connect(hideFullGamesCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { model->setHideFullGames(checked); });

    hideStartedGamesCheckBox = new QCheckBox(this);
    hideStartedGamesCheckBox->setChecked(model->getHideGamesThatStarted());
    hideStartedGamesLabel = new QLabel(this);
    hideStartedGamesLabel->setBuddy(hideStartedGamesCheckBox);
    connect(hideStartedGamesCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { model->setHideGamesThatStarted(checked); });

    filterToFormatComboBox = new QComboBox(this);
    filterToFormatLabel = new QLabel(this);
    filterToFormatLabel->setBuddy(filterToFormatComboBox);

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
    mainLayout->addWidget(hideGamesWithoutBuddiesLabel);
    mainLayout->addWidget(hideGamesWithoutBuddiesCheckBox);
    mainLayout->addWidget(hideFullGamesLabel);
    mainLayout->addWidget(hideFullGamesCheckBox);
    mainLayout->addWidget(hideStartedGamesLabel);
    mainLayout->addWidget(hideStartedGamesCheckBox);
    mainLayout->addWidget(filterToFormatLabel);
    mainLayout->addWidget(filterToFormatComboBox);

    setLayout(mainLayout);

    retranslateUi();
}

void GameSelectorQuickFilterToolBar::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Filter by creator name..."));
    hideGamesWithoutBuddiesLabel->setText(tr("Hide buddies only games"));
    hideFullGamesLabel->setText(tr("Hide full games"));
    hideStartedGamesLabel->setText(tr("Hide started games"));
    filterToFormatLabel->setText(tr("Filter to format"));
}
