#ifndef VISUAL_DECK_EDITOR_H
#define VISUAL_DECK_EDITOR_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"
#include "../quick_settings/settings_button_widget.h"

#include <QListWidget>
#include <QWidget>
#include <qscrollarea.h>

class VisualDeckEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckEditorWidget(QWidget *parent, DeckListModel *deckListModel);
    void resizeEvent(QResizeEvent *event) override;

    void setDeckList(const DeckList &_deckListModel);

public slots:
    void decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight);
    void updateZoneWidgets();
    void addZoneIfDoesNotExist();
    void deleteZoneIfDoesNotExist();

signals:
    void activeCardChanged(CardInfoPtr activeCard);
    void activeGroupCriteriaChanged(QString activeGroupCriteria);
    void activeSortCriteriaChanged(QStringList activeSortCriteria);
    void cardClicked(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);

protected slots:
    void onHover(CardInfoPtr hoveredCard);
    void onCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);
    void actChangeActiveGroupCriteria();
    void actChangeActiveSortCriteria();

private:
    DeckListModel *deckListModel;
    QVBoxLayout *mainLayout;
    QWidget *groupAndSortContainer;
    QHBoxLayout *groupAndSortLayout;
    QComboBox *groupByComboBox;
    QString activeGroupCriteria = "maintype";
    SettingsButtonWidget *sortCriteriaButton;
    QListWidget *sortByListWidget;
    QStringList activeSortCriteria = {"name", "cmc", "colors", "maintype"};
    QScrollArea *scrollArea;
    QWidget *zoneContainer;
    QVBoxLayout *zoneContainerLayout;
    OverlapControlWidget *overlapControlWidget;
    QWidget *container;
};

#endif // VISUAL_DECK_EDITOR_H
