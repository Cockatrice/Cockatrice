#ifndef ALL_ZONES_CARD_AMOUNT_WIDGET_H
#define ALL_ZONES_CARD_AMOUNT_WIDGET_H
#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../deck/deck_view.h"
#include "card_amount_widget.h"

#include <QVBoxLayout>
#include <QWidget>

class AllZonesCardAmountWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AllZonesCardAmountWidget(QWidget *parent,
                                      TabDeckEditor *deckEditor,
                                      DeckListModel *deckModel,
                                      QTreeView *deckView,
                                      CardInfoPtr rootCard,
                                      CardInfoPerSet setInfoForCard);
    int getMainboardAmount();
    int getSideboardAmount();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif

private:
    QVBoxLayout *layout;
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr rootCard;
    CardInfoPerSet setInfoForCard;
    QLabel *zoneLabelMainboard;
    CardAmountWidget *buttonBoxMainboard;
    QLabel *zoneLabelSideboard;
    CardAmountWidget *buttonBoxSideboard;
};

#endif // ALL_ZONES_CARD_AMOUNT_WIDGET_H
