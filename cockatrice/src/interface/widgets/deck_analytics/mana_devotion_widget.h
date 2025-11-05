/**
 * @file mana_devotion_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_DEVOTION_WIDGET_H
#define MANA_DEVOTION_WIDGET_H

#include "../general/display/banner_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <utility>

class ManaDevotionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaDevotionWidget(QWidget *parent, DeckListModel *deckListModel);
    void updateDisplay();

    std::unordered_map<char, int> countManaSymbols(const QString &manaString);
    void mergeManaCounts(std::unordered_map<char, int> &manaCounts1, const std::unordered_map<char, int> &manaCounts2);

public slots:
    void setDeckModel(DeckListModel *deckModel);
    std::unordered_map<char, int> analyzeManaDevotion();
    void retranslateUi();

private:
    DeckListModel *deckListModel;
    BannerWidget *bannerWidget;
    std::unordered_map<char, int> manaDevotionMap;
    QVBoxLayout *layout;
    QHBoxLayout *barLayout;
};

#endif // MANA_DEVOTION_WIDGET_H
