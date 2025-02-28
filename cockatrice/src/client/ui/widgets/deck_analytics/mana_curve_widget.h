#ifndef MANA_CURVE_WIDGET_H
#define MANA_CURVE_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../general/display/banner_widget.h"

#include <QHBoxLayout>
#include <QWidget>
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

private:
    DeckListModel *deckListModel;
    std::unordered_map<int, int> manaCurveMap;
    QVBoxLayout *layout;
    QHBoxLayout *barLayout;
    BannerWidget *bannerWidget;
};

#endif // MANA_CURVE_WIDGET_H
