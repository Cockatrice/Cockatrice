#ifndef WINDOW_SETS_H
#define WINDOW_SETS_H

#include <QMainWindow>

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
    QPushButton *upButton, *downButton, *bottomButton, *topButton;
public:
    WndSets(QWidget *parent = 0);
    ~WndSets();
protected:
    void selectRow(int row);
private slots:
    void actUp();
    void actDown();
    void actTop();
    void actBottom();
    void actToggleButtons(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif
