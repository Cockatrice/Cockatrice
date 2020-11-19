#ifndef DLG_MANAGE_SETS_H
#define DLG_MANAGE_SETS_H

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QSet>

class CardDatabase;
class LineEditUnfocusable;
class QGroupBox;
class QItemSelection;
class QPushButton;
class QTreeView;
class SetsDisplayModel;
class SetsModel;
class SetsProxyModel;

class WndSets : public QMainWindow
{
    Q_OBJECT
private:
    SetsModel *model;
    SetsDisplayModel *displayModel;
    QGroupBox *hintsGroupBox;
    QTreeView *view;
    QPushButton *toggleAllButton, *toggleSelectedButton;
    QPushButton *enableAllButton, *disableAllButton, *enableSomeButton, *disableSomeButton;
    QPushButton *defaultSortButton;
    QAction *aUp, *aDown, *aBottom, *aTop;
    QToolBar *setsEditToolBar;
    QDialogButtonBox *buttonBox;
    QLabel *labNotes, *searchLabel;
    QGroupBox *sortWarning;
    QLabel *sortWarningText;
    QPushButton *sortWarningButton;
    LineEditUnfocusable *searchField;
    QGridLayout *mainLayout;
    QHBoxLayout *filterBox;
    int sortIndex;
    Qt::SortOrder sortOrder;
    void rebuildMainLayout(int actionToTake);
    bool setOrderIsSorted;
    enum
    {
        NO_SETS_SELECTED,
        SOME_SETS_SELECTED
    };

public:
    WndSets(QWidget *parent = nullptr);
    ~WndSets();

protected:
    void selectRows(QSet<int> rows);
private slots:
    void actEnableAll();
    void actDisableAll();
    void actEnableSome();
    void actDisableSome();
    void actSave();
    void actRestore();
    void actUp();
    void actDown();
    void actTop();
    void actBottom();
    void actToggleButtons(const QItemSelection &selected, const QItemSelection &deselected);
    void actDisableSortButtons(int index);
    void actRestoreOriginalOrder();
    void actDisableResetButton(const QString &filterText);
    void actSort(int index);
    void actIgnoreWarning();
};

#endif
