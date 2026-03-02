#ifndef COCKATRICE_RESIZABLE_PANEL_H
#define COCKATRICE_RESIZABLE_PANEL_H

#include "abstract_analytics_panel_widget.h"

#include <QApplication>
#include <QDrag>
#include <QFrame>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class ResizablePanel : public QWidget
{
    Q_OBJECT
public:
    explicit ResizablePanel(const QString &typeId,
                            AbstractAnalyticsPanelWidget *analyticsPanel,
                            QWidget *parent = nullptr);

    void setSelected(bool selected);
    void setHeightFromSaved(int h);
    int getCurrentHeight() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    QString getTypeId() const
    {
        return typeId;
    }

    AbstractAnalyticsPanelWidget *panel;

signals:
    void dragStarted(ResizablePanel *panel);
    void dropRequested(ResizablePanel *dragged, ResizablePanel *target, bool insertBefore);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int getMinimumAllowedHeight() const;
    void updateSizeConstraints();
    void startDrag();
    void performAutoScroll();
    void showDropIndicator(double y);
    void hideDropIndicator();

    QString typeId;

    QFrame *frame;
    QFrame *selectionOverlay;
    QFrame *dropIndicator;
    QPushButton *dragButton;
    QWidget *resizeHandle;

    int currentHeight;
    bool isResizing = false;
    bool isDraggingPanel = false;
    double resizeStartY = 0;
    int resizeStartHeight = 0;

    QPoint dragStartPos;
    QPoint lastDragPos;
    QTimer *autoScrollTimer;
};

#endif // COCKATRICE_RESIZABLE_PANEL_H
