#include "../../../../deck/deck_list_model.h"

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
    explicit ManaBaseWidget(QWidget *parent = nullptr, DeckListModel *deck_list_model = nullptr);
    std::unordered_map<char, int> analyzeManaBase();
    void updateDisplay();

    std::unordered_map<char, int> determineManaProduction(const QString &manaString);
    void mergeManaCounts(std::unordered_map<char, int> &manaCounts1, const std::unordered_map<char, int> &manaCounts2);

private:
    DeckListModel *deck_list_model;
    std::unordered_map<char, int> manaBaseMap;
    QHBoxLayout *layout;
};

#endif // MANA_BASE_WIDGET_H
