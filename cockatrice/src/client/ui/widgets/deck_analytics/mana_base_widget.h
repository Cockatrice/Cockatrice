#include "../../../../deck/deck_list_model.h"
#include "../general/display/banner_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <decklist.h>
#include <utility>

#ifndef MANA_BASE_WIDGET_H
#define MANA_BASE_WIDGET_H

class ManaBaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaBaseWidget(QWidget *parent, DeckListModel *deckListModel);
    std::unordered_map<char, int> analyzeManaBase();
    void updateDisplay();

    std::unordered_map<char, int> determineManaProduction(const QString &manaString);
    void mergeManaCounts(std::unordered_map<char, int> &manaCounts1, const std::unordered_map<char, int> &manaCounts2);

public slots:
    void setDeckModel(DeckListModel *deckModel);

private:
    DeckListModel *deckListModel;
    BannerWidget *bannerWidget;
    std::unordered_map<char, int> manaBaseMap;
    QVBoxLayout *layout;
    QHBoxLayout *barLayout;
};

#endif // MANA_BASE_WIDGET_H
