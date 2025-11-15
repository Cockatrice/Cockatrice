#ifndef COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H
#define COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H
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
                                            GamesProxyModel *model,
                                            const QMap<int, QString> &allGameTypes);
    void retranslateUi();

private:
    GamesProxyModel *model;

    QHBoxLayout *mainLayout;

    QLineEdit *searchBar;
    QCheckBox *hideGamesWithoutBuddiesCheckBox;
    QLabel *hideGamesWithoutBuddiesLabel;
    QCheckBox *hideFullGamesCheckBox;
    QLabel *hideFullGamesLabel;
    QCheckBox *hideStartedGamesCheckBox;
    QLabel *hideStartedGamesLabel;
    QComboBox *filterToFormatComboBox;
    QLabel *filterToFormatLabel;
};

#endif // COCKATRICE_GAME_SELECTOR_QUICK_FILTER_TOOLBAR_H
