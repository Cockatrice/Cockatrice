#ifndef WINDOW_SETS_H
#define WINDOW_SETS_H

#include <QMainWindow>

class SetsModel;
class SetsProxyModel;
class QTreeView;
class QPushButton;
class CardDatabase;

class WndSets : public QMainWindow {
    Q_OBJECT
private:
    SetsModel *model;
    SetsProxyModel *proxyModel;
    QTreeView *view;
    QPushButton *saveButton, *restoreButton;
public:
    WndSets(QWidget *parent = 0);
    ~WndSets();
private slots:
    void actSave();
    void actRestore();
};

#endif
