/**
 * @file dlg_select_set_for_cards.h
 * @ingroup Dialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_SELECT_SET_FOR_CARDS_H
#define DLG_SELECT_SET_FOR_CARDS_H

#include "../interface/widgets/general/layout_containers/flow_widget.h"

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <libcockatrice/models/deck_list/deck_list_model.h>

class SetEntryWidget; // Forward declaration

class DlgSelectSetForCards : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSelectSetForCards(QWidget *parent, DeckListModel *_model);
    void retranslateUi();
    void sortSetsByCount();
    QMap<QString, QStringList> getCardsForSets();
    QMap<QString, QStringList> getModifiedCards();
    QVBoxLayout *listLayout;
    QList<SetEntryWidget *> entry_widgets;
    QMap<QString, QStringList> cardsForSets;

signals:
    void widgetOrderChanged();
    void orderChanged();

public slots:
    void actOK();
    void actClear();
    void actSetAllToPreferred();
    void updateLayoutOrder();
    void updateCardLists();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QVBoxLayout *layout;
    QLabel *instructionLabel;
    QScrollArea *scrollArea;
    QScrollArea *uneditedCardsArea;
    FlowWidget *uneditedCardsFlowWidget;
    QLabel *uneditedCardsLabel;
    QScrollArea *modifiedCardsArea;
    FlowWidget *modifiedCardsFlowWidget;
    QLabel *modifiedCardsLabel;
    QWidget *listContainer;
    QListWidget *listWidget;
    DeckListModel *model;
    QMap<QString, SetEntryWidget *> setEntries;
    QPushButton *clearButton;
    QPushButton *setAllToPreferredButton;

    QMap<QString, int> getSetsForCards();
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
    bool isChecked() const;
    DlgSelectSetForCards *parent;
    QString setName;
    bool expanded;

public slots:
    void mousePressEvent(QMouseEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
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