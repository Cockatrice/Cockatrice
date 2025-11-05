/**
 * @file mana_base_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_BASE_WIDGET_H
#define MANA_BASE_WIDGET_H

#include "../general/display/banner_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <utility>

class ManaBaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaBaseWidget(QWidget *parent, DeckListModel *deckListModel);
    QHash<QString, int> analyzeManaBase();
    void updateDisplay();

    QHash<QString, int> determineManaProduction(const QString &manaString);
    void mergeManaCounts(QHash<QString, int> &manaCounts1, const QHash<QString, int> &manaCounts2);

public slots:
    void setDeckModel(DeckListModel *deckModel);
    void retranslateUi();

private:
    DeckListModel *deckListModel;
    BannerWidget *bannerWidget;
    QHash<QString, int> manaBaseMap;
    QVBoxLayout *layout;
    QWidget *barContainer;
    QHBoxLayout *barLayout;
};

#endif // MANA_BASE_WIDGET_H
