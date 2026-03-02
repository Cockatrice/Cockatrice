/**
 * @file visual_deck_editor_sample_hand_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
#define VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H

#include "../cards/card_size_widget.h"
#include "../deck_analytics/deck_list_statistics_analyzer.h"
#include "../general/layout_containers/flow_widget.h"

#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>

class DrawProbabilityWidget;
class VisualDeckEditorSampleHandWidget : public QWidget
{
    Q_OBJECT
public:
    VisualDeckEditorSampleHandWidget(QWidget *parent,
                                     DeckListModel *deckListModel,
                                     DeckListStatisticsAnalyzer *statsAnalyzer);
    QList<ExactCard> getRandomCards(int amountToGet);

public slots:
    void updateDisplay();
    void setDeckModel(DeckListModel *deckModel);
    void retranslateUi();

private:
    DeckListModel *deckListModel;
    DeckListStatisticsAnalyzer *statsAnalyzer;
    QVBoxLayout *layout;
    QWidget *resetAndHandSizeContainerWidget;
    QHBoxLayout *resetAndHandSizeLayout;
    QPushButton *resetButton;
    QSpinBox *handSizeSpinBox;
    FlowWidget *flowWidget;
    DrawProbabilityWidget *drawProbabilityWidget;
    CardSizeWidget *cardSizeWidget;
};

#endif // VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
