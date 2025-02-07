#ifndef DLG_SELECT_SET_FOR_CARDS_H
#define DLG_SELECT_SET_FOR_CARDS_H

#include "../client/ui/widgets/general/layout_containers/flow_widget.h"
#include "../deck/deck_list_model.h"

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>

class SetEntryWidget; // Forward declaration

class DlgSelectSetForCards : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSelectSetForCards(QWidget *parent, DeckListModel *_model);
    void actOK();
    void sortSetsByCount();
    QMap<QString, QStringList> getCardsForSets();
    QMap<QString, QStringList> getModifiedCards();
    void updateLayoutOrder();
    QVBoxLayout *listLayout;
    QList<SetEntryWidget *> entry_widgets;
    QMap<QString, QStringList> cardsForSets;

public slots:
    void updateCardLists();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QVBoxLayout *layout;
    QScrollArea *scrollArea;
    QScrollArea *uneditedCardsArea;
    FlowWidget *uneditedCardsFlowWidget;
    QLabel *uneditedCardsLabel;
    QWidget *listContainer;
    QListWidget *listWidget;
    DeckListModel *model;
    QMap<QString, SetEntryWidget *> setEntries;

    QMap<QString, int> getSetsForCards();
    void updateCardAvailability();
};

class SetEntryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SetEntryWidget(DlgSelectSetForCards *parent, const QString &setName, int count);
    void toggleExpansion();
    void checkVisibility();
    QStringList getAllCardsForSet();
    void populateCardList();
    void updateCardDisplayWidgets();
    void updateCardState(bool checked);
    bool isChecked() const;
    DlgSelectSetForCards *parent;
    QString setName;
    bool expanded;

public slots:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

private:
    QVBoxLayout *layout;
    QCheckBox *checkBox;
    QPushButton *expandButton;
    QLabel *countLabel;
    QLabel *possibleCardsLabel;
    FlowWidget *cardListContainer;
    QLabel *alreadySelectedCardsLabel;
    FlowWidget *alreadySelectedCardListContainer;
    QVBoxLayout *cardListLayout;
    QStringList possibleCards;
    QStringList unusedCards;
};

#endif // DLG_SELECT_SET_FOR_CARDS_H