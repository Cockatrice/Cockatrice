/**
 * @file visual_database_display_name_filter_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_DISPLAY_NAME_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_NAME_FILTER_WIDGET_H

#include <QWidget>

class AbstractTabDeckEditor;
class FilterTreeModel;
class QVBoxLayout;
class QLineEdit;
class FlowWidget;
class QPushButton;
class VisualDatabaseDisplayNameFilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDatabaseDisplayNameFilterWidget(QWidget *parent,
                                                   AbstractTabDeckEditor *deckEditor,
                                                   FilterTreeModel *filterModel);

    void createNameFilter(const QString &name);
    void removeNameFilter(const QString &name);
    void updateFilterList();
    void updateFilterModel();
    void syncWithFilterModel();

public slots:
    void retranslateUi();

private:
    AbstractTabDeckEditor *deckEditor;
    FilterTreeModel *filterModel;
    QVBoxLayout *layout;
    QLineEdit *searchBox;
    FlowWidget *flowWidget;
    QPushButton *loadFromDeckButton;
    QPushButton *loadFromClipboardButton;

    QMap<QString, QPushButton *> activeFilters; // Store active name filter buttons

private slots:
    void actLoadFromDeck();
    void actLoadFromClipboard();
};

#endif // VISUAL_DATABASE_DISPLAY_NAME_FILTER_WIDGET_H
