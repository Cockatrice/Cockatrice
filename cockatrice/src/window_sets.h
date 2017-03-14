#ifndef WINDOW_SETS_H
#define WINDOW_SETS_H

#include <QMainWindow>
#include <QSet>

class SetsModel;
class SetsProxyModel;
class QPushButton;
class CardDatabase;
class QItemSelection;
class QTreeView;

class WndSets : public QMainWindow {
    Q_OBJECT
private:
    SetsModel *model;
    QTreeView *view;
    QPushButton *toggleAllButton, *toggleSelectedButton;
    QAction *aUp, *aDown, *aBottom, *aTop;
public:
    WndSets(QWidget *parent = 0);
    ~WndSets();
protected:
    void selectRows(QSet<int> rows);
private slots:
    void actToggle();
    void actToggleAll();
    void actSave();
    void actRestore();
    void actUp();
    void actDown();
    void actTop();
    void actBottom();
    void actToggleButtons(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif
