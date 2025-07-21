#ifndef VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
#define VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H

#include "../../../../client/ui/widgets/cards/card_size_widget.h"
#include "../../../../deck/deck_list_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

class VisualDeckEditorSampleHandWidget : public QWidget
{
    Q_OBJECT
public:
    VisualDeckEditorSampleHandWidget(QWidget *parent, DeckListModel *deckListModel);
    QList<ExactCard> getRandomCards(int amountToGet);

public slots:
    void updateDisplay();
    void setDeckModel(DeckListModel *deckModel);
    void retranslateUi();

private:
    DeckListModel *deckListModel;
    QVBoxLayout *layout;
    QWidget *resetAndHandSizeContainerWidget;
    QHBoxLayout *resetAndHandSizeLayout;
    QPushButton *resetButton;
    QSpinBox *handSizeSpinBox;
    FlowWidget *flowWidget;
    CardSizeWidget *cardSizeWidget;
};

#endif // VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
