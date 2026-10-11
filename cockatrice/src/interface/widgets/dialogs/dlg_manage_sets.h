/**
 * @file dlg_manage_sets.h
 * @ingroup Dialogs
 */
//! \todo Document this file.

#ifndef DLG_MANAGE_SETS_H
#define DLG_MANAGE_SETS_H

#include <QMainWindow>
#include <QString>
#include <qnamespace.h>
#include <qtmetamacros.h>

class LineEditUnfocusable;
class QGroupBox;
class QItemSelection;
class QPushButton;
class QTreeView;
class SetsDisplayModel;
class SetsModel;
class QAction;
class QDialogButtonBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QToolBar;
class QWidget;
template <class T> class QSet;

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
    bool setOrderIsSorted;
    enum
    {
        NO_SETS_SELECTED,
        SOME_SETS_SELECTED
    };

    void closeEvent(QCloseEvent *ev) override;
    void saveHeaderState();
    void rebuildMainLayout(int actionToTake);
    void resetSort();

public:
    explicit WndSets(QWidget *parent = nullptr);
    ~WndSets() override;

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
