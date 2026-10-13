#ifndef REPORT_STATUS_DELEGATE_H
#define REPORT_STATUS_DELEGATE_H

#include <QStyledItemDelegate>
#include <qtmetamacros.h>

/**
 * Status pill for the report queue table. Paints the status text on a
 * translucent badge tinted with a palette role per status, so the colors
 * always follow the active theme. Falls back to default painting for
 * selected rows and unknown statuses.
 */
class ReportStatusDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ReportStatusDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // REPORT_STATUS_DELEGATE_H
