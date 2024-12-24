#ifndef VISUAL_DECK_STORAGE_WIDGET_H
#define VISUAL_DECK_STORAGE_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../ui/widgets/cards/deck_preview_card_picture_widget.h"
#include "../../../ui/widgets/general/layout_containers/flow_widget.h"

#include <QFileSystemModel>

class VisualDeckStorageWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDeckStorageWidget(QWidget *parent);
    void retranslateUi();

public slots:
    void imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

signals:
    void imageClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

private:
    QHBoxLayout *layout;
    FlowWidget *flowWidget;
    DeckListModel *deckListModel;
    QMap<QString, DeckViewCardContainer *> cardContainers;

    QStringList getBannerCardsForDecks();
};

#endif // VISUAL_DECK_STORAGE_WIDGET_H
