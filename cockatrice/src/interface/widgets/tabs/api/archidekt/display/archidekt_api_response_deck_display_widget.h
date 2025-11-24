#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_DISPLAY_WIDGET_H

#include "../../../../../deck_loader/deck_loader.h"
#include "../../../../cards/card_size_widget.h"
#include "../../../../general/layout_containers/flow_widget.h"
#include "../api_response/deck/archidekt_api_response_deck.h"
#include "deck_list_model.h"

#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class ArchidektApiResponseDeckDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    void requestNavigation(QString url);
    void openInDeckEditor(DeckLoader *loader);

public:
    explicit ArchidektApiResponseDeckDisplayWidget(QWidget *parent,
                                                   ArchidektApiResponseDeck response,
                                                   CardSizeWidget *cardSizeSlider);
    void retranslateUi();
    void actOpenInDeckEditor();
    void clearAllDisplayWidgets();
    void decklistModelReset();
    void constructZoneWidgetsFromDeckListModel();
    void resizeEvent(QResizeEvent *event) override;

private:
    ArchidektApiResponseDeck response;
    CardSizeWidget *cardSizeSlider;
    QVBoxLayout *layout;
    QPushButton *openInEditorButton;
    QScrollArea *scrollArea;
    QWidget *zoneContainer;
    QVBoxLayout *zoneContainerLayout;
    QWidget *container;
    QHash<QPersistentModelIndex, QWidget *> indexToWidgetMap;
    QVBoxLayout *containerLayout;
    DeckListModel *model;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_DISPLAY_WIDGET_H
