#ifndef COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H
#define COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H

#include "../tabs/tab_supervisor.h"
#include "games_model.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

class GameSelectorQuickFilterToolBar : public QWidget
{

    Q_OBJECT

public:
    explicit GameSelectorQuickFilterToolBar(QWidget *parent,
                                            TabSupervisor *tabSupervisor,
                                            GamesProxyModel *model,
                                            const QMap<int, QString> &allGameTypes);
    void retranslateUi();

private:
    TabSupervisor *tabSupervisor;
    GamesProxyModel *model;

    QHBoxLayout *mainLayout;

    QLineEdit *searchBar;
    QCheckBox *hideGamesNotCreatedByBuddiesCheckBox;
    QCheckBox *hideFullGamesCheckBox;
    QCheckBox *hideStartedGamesCheckBox;
    QComboBox *filterToFormatComboBox;
};

#endif // COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H
