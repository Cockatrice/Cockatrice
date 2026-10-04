#ifndef COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H
#define COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H

#include <QWidget>
#include <functional>
#include <qtmetamacros.h>

class GamesProxyModel;
class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QLineEdit;
class QString;
class TabSupervisor;
struct GameFilterConfigs;
template <class Key, class T> class QMap;

class GameSelectorQuickFilterToolBar : public QWidget
{

    Q_OBJECT

public:
    explicit GameSelectorQuickFilterToolBar(QWidget *parent,
                                            TabSupervisor *tabSupervisor,
                                            GamesProxyModel *model,
                                            const QMap<int, QString> &allGameTypes);
    void syncFromModel();
    void applyFilters(std::function<void(GameFilterConfigs &)> mutator);
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
