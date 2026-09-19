/**
 * @file flow_widget.h
 * @ingroup UI
 * @brief A QWidget that wraps a FlowLayout inside an optional QScrollArea.
 */
//! \todo Document this file.

#ifndef FLOW_WIDGET_H
#define FLOW_WIDGET_H

#include "../../../layouts/flow_layout.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QScrollArea>
#include <QWidget>

inline Q_LOGGING_CATEGORY(FlowWidgetLog, "flow_widget", QtInfoMsg);
inline Q_LOGGING_CATEGORY(FlowWidgetSizeLog, "flow_widget.size", QtInfoMsg);

class FlowWidget : public QWidget
{
    Q_OBJECT

public:
    FlowWidget(QWidget *parent,
               Qt::Orientation orientation,
               Qt::ScrollBarPolicy horizontalPolicy,
               Qt::ScrollBarPolicy verticalPolicy);

    void addWidget(QWidget *widget_to_add);
    void insertWidgetAtIndex(QWidget *toInsert, int index);
    void removeWidget(QWidget *widgetToRemove) const;
    void clearLayout();

    [[nodiscard]] int count() const;
    [[nodiscard]] QLayoutItem *itemAt(int index) const;

    QScrollArea *scrollArea; ///< Null when both scroll policies are AlwaysOff.

public slots:
    void setMinimumSizeToMaxSizeHint();
    void setSpacing(int hSpacing, int vSpacing);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    /// @brief Moves keyboard focus to an adjacent flow item for an arrow-key event.
    /// @return True when the event was an arrow key and was handled.
    bool moveFocus(QKeyEvent *event);

    Qt::Orientation flowDirection;
    QHBoxLayout *mainLayout;
    FlowLayout *flowLayout;
    QWidget *container;
};

#endif // FLOW_WIDGET_H