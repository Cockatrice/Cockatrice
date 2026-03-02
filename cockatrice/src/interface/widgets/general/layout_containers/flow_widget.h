/**
 * @file flow_widget.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef FLOW_WIDGET_H
#define FLOW_WIDGET_H
#include "../../../layouts/flow_layout.h"

#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QWidget>
#include <qscrollarea.h>

inline Q_LOGGING_CATEGORY(FlowWidgetLog, "flow_widget", QtInfoMsg);
inline Q_LOGGING_CATEGORY(FlowWidgetSizeLog, "flow_widget.size", QtInfoMsg);

class FlowWidget final : public QWidget
{
    Q_OBJECT

public:
    FlowWidget(QWidget *parent,
               Qt::Orientation orientation,
               Qt::ScrollBarPolicy horizontalPolicy,
               Qt::ScrollBarPolicy verticalPolicy);
    void addWidget(QWidget *widget_to_add) const;
    void insertWidgetAtIndex(QWidget *toInsert, int index);
    void removeWidget(QWidget *widgetToRemove) const;
    void clearLayout();
    [[nodiscard]] int count() const;
    [[nodiscard]] QLayoutItem *itemAt(int index) const;

    QScrollArea *scrollArea;

public slots:
    void setMinimumSizeToMaxSizeHint();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Qt::Orientation flowDirection;
    QHBoxLayout *mainLayout;
    FlowLayout *flowLayout;
    QWidget *container;
};

#endif // FLOW_WIDGET_H
