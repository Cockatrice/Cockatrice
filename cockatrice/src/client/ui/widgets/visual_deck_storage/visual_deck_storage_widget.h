#ifndef VISUAL_DECK_STORAGE_WIDGET_H
#define VISUAL_DECK_STORAGE_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../ui/widgets/cards/deck_preview_card_picture_widget.h"
#include "../../../ui/widgets/general/layout_containers/flow_widget.h"
#include "../cards/card_size_widget.h"
#include "visual_deck_storage_search_widget.h"

#include <QComboBox>
#include <QFileSystemModel>

class VisualDeckStorageSearchWidget;
class VisualDeckStorageWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDeckStorageWidget(QWidget *parent);
    void retranslateUi();

public slots:
    void imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void refreshBannerCards(); // Refresh the display of cards based on the current sorting option
    void showEvent(QShowEvent *event) override;
    void updateSortOrder();

signals:
    void imageClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClicked(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

private:
    enum SortOrder
    {
        Alphabetical,
        ByLastModified
    };

    QVBoxLayout *layout;
    FlowWidget *flowWidget;
    DeckListModel *deckListModel;
    QMap<QString, DeckViewCardContainer *> cardContainers;

    SortOrder sortOrder; // Current sorting option
    QComboBox *sortComboBox;
    VisualDeckStorageSearchWidget *searchWidget;
    CardSizeWidget *cardSizeWidget;
};

#endif // VISUAL_DECK_STORAGE_WIDGET_H
