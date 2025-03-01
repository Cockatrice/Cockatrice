#ifndef VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
#define VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QPushButton>
#include <QWidget>

class VisualDeckEditorSampleHandWidget : public QWidget
{
    Q_OBJECT
public:
    VisualDeckEditorSampleHandWidget(QWidget *parent, DeckListModel *deckListModel);
    QList<CardInfoPtr> getRandomCards(int amountToGet);

public slots:
    void updateDisplay();
    void setDeckModel(DeckListModel *deckModel);
    void retranslateUi();

private:
    DeckListModel *deckListModel;
    QVBoxLayout *layout;
    QPushButton *resetButton;
    FlowWidget *flowWidget;
};

#endif // VISUAL_DECK_EDITOR_SAMPLE_HAND_WIDGET_H
