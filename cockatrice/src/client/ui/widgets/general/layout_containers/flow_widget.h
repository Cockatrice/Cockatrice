#ifndef FLOW_WIDGET_H
#define FLOW_WIDGET_H
#include "../../../layouts/flow_layout.h"

#include <QHBoxLayout>
#include <QWidget>
#include <qscrollarea.h>

class FlowWidget final : public QWidget
{
    Q_OBJECT

public:
    FlowWidget(QWidget *parent, Qt::ScrollBarPolicy horizontalPolicy, Qt::ScrollBarPolicy verticalPolicy);
    void addWidget(QWidget *widget_to_add) const;
    void clearLayout();
    [[nodiscard]] int count() const;
    [[nodiscard]] QLayoutItem *itemAt(int index) const;

    QScrollArea *scrollArea;

public slots:
    void setMinimumSizeToMaxSizeHint();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *mainLayout;
    FlowLayout *flowLayout;
    QWidget *container;
};

#endif // FLOW_WIDGET_H
