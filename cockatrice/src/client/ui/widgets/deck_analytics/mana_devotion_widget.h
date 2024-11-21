#ifndef MANA_DEVOTION_WIDGET_H
#define MANA_DEVOTION_WIDGET_H
#include "../../../../deck/deck_list_model.h"

#include <QHBoxLayout>
#include <QWidget>
#include <decklist.h>
#include <utility>

class ManaDevotionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaDevotionWidget(QWidget *parent = nullptr, DeckListModel *deck_list_model = nullptr);
    std::unordered_map<char, int> analyzeManaDevotion();
    void updateDisplay();

    std::unordered_map<char, int> countManaSymbols(const QString &manaString);
    void mergeManaCounts(std::unordered_map<char, int> &manaCounts1, const std::unordered_map<char, int> &manaCounts2);

private:
    DeckListModel *deck_list_model;
    std::unordered_map<char, int> manaDevotionMap;
    QHBoxLayout *layout;
};

#endif // MANA_DEVOTION_WIDGET_H
