/**
 * @file visual_deck_editor_sample_hand_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
#define VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H

#include <QWidget>

class DeckListModel;
class DeckListStatisticsAnalyzer;
class ExactCard;
class DrawProbabilityWidget;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QSpinBox;
class FlowWidget;
class CardSizeWidget;
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
