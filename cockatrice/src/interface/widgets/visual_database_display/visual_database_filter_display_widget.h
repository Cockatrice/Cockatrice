/**
 * @file visual_database_filter_display_widget.h
 * @ingroup VisualCardDatabaseWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DATABASE_FILTER_DISPLAY_WIDGET_H
#define VISUAL_DATABASE_FILTER_DISPLAY_WIDGET_H

#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QWidget>

class FilterTreeModel;

class FilterDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterDisplayWidget(QWidget *parent, const QString &filename, FilterTreeModel *_filterModel);
    ~FilterDisplayWidget() = default;

    QSize sizeHint() const override;

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void loadFilter();
    void deleteFilter();

signals:
    void filterClicked();
    void filterLoadRequested(const QString &filename);
    void filterDeleted(const QString &filename);

private:
    QString filterFilename;
    FilterTreeModel *filterModel;
    QPushButton *filterButton; // Button for the filter text
    QPushButton *closeButton;  // Close button to delete the filter
};

#endif // VISUAL_DATABASE_FILTER_DISPLAY_WIDGET_H
