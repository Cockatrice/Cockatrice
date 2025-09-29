/**
 * @file visual_database_display_filter_save_load_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_DISPLAY_FILTER_SAVE_LOAD_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_FILTER_SAVE_LOAD_WIDGET_H

#include "../../../filters/filter_tree_model.h"
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
    void deleteFilter(const QString &filename, QPushButton *deleteButton);

public slots:
    void retranslateUi();

private:
    FilterTreeModel *filterModel;

    QVBoxLayout *layout;
    QLineEdit *filenameInput;
    QPushButton *saveButton;
    FlowWidget *fileListWidget;

    QMap<QString, QPair<QPushButton *, QPushButton *>> fileButtons;
};

#endif // VISUAL_DATABASE_DISPLAY_FILTER_SAVE_LOAD_WIDGET_H
