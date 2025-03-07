#ifndef VISUAL_DATABASE_DISPLAY_FILTER_SAVE_LOAD_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_FILTER_SAVE_LOAD_WIDGET_H

#include "../../../../game/filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class VisualDatabaseDisplayFilterSaveLoadWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplayFilterSaveLoadWidget(QWidget *parent, FilterTreeModel *filterModel);

    void saveFilter();
    void loadFilter(const QString &filename);
    void refreshFilterList();

private:
    FilterTreeModel *filterModel;

    QVBoxLayout *layout;
    QLineEdit *filenameInput;
    QPushButton *saveButton;
    FlowWidget *fileListWidget;

    QMap<QString, QPushButton *> fileButtons;
};

#endif // VISUAL_DATABASE_DISPLAY_FILTER_SAVE_LOAD_WIDGET_H
