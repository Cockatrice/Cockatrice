#ifndef DLG_SELECT_SET_FOR_CARDS_H
#define DLG_SELECT_SET_FOR_CARDS_H

#include "../client/ui/widgets/general/layout_containers/flow_widget.h"
#include "../deck/deck_list_model.h"

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>

class QCheckBox;
class SetEntryWidget; // Forward declaration

class DlgSelectSetForCards : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSelectSetForCards(QWidget *parent, DeckListModel *_model);
    void sortSetsByCount();
    QMap<QString, QStringList> getCardsForSets();
    QMap<QString, QStringList> getModifiedCards();
    QVBoxLayout *listLayout;

public slots:
    void updateCardLists();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void actOK();

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
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void toggleExpansion();
    QStringList getAllCardsForSet();
    void populateCardList();
    void updateCardState(bool checked);
    bool isChecked() const;
    DlgSelectSetForCards *parent;
    QString setName;
    bool expanded;

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
};

#endif // DLG_SELECT_SET_FOR_CARDS_H