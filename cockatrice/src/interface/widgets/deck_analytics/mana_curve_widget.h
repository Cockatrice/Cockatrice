/**
 * @file mana_curve_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_CURVE_WIDGET_H
#define MANA_CURVE_WIDGET_H

#include "../general/display/banner_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <unordered_map>

class ManaCurveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaCurveWidget(QWidget *parent, DeckListModel *deckListModel);
    void updateDisplay();

public slots:
    void setDeckModel(DeckListModel *deckModel);
    std::unordered_map<int, int> analyzeManaCurve();
    void retranslateUi();

private:
    DeckListModel *deckListModel;
    std::unordered_map<int, int> manaCurveMap;
    QVBoxLayout *layout;
    BannerWidget *bannerWidget;
    QWidget *barContainer;
    QHBoxLayout *barLayout;
};

#endif // MANA_CURVE_WIDGET_H
