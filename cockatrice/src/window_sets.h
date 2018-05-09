#ifndef WINDOW_SETS_H
#define WINDOW_SETS_H

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSet>

class SetsModel;
class SetsProxyModel;
class SetsDisplayModel;
class QPushButton;
class CardDatabase;
class QItemSelection;
class QTreeView;

class WndSets : public QMainWindow
{
    Q_OBJECT
private:
    SetsModel *model;
    SetsDisplayModel *displayModel;
    QTreeView *view;
    QPushButton *toggleAllButton, *toggleSelectedButton;
    QPushButton *enableAllButton, *disableAllButton, *enableSomeButton, *disableSomeButton;
    QAction *aUp, *aDown, *aBottom, *aTop;
    QToolBar *setsEditToolBar;
    QDialogButtonBox *buttonBox;
    QLabel *labNotes, *searchLabel;
    QLineEdit *searchField;
    QGridLayout *mainLayout;
    void rebuildMainLayout(int actionToTake);
    enum
    {
        NO_SETS_SELECTED,
        SOME_SETS_SELECTED
    };

public:
    WndSets(QWidget *parent = 0);
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
};

#endif
