#include "report_utils.h"

#include <QBrush>
#include <QDateTime>
#include <QTableWidget>
#include <QTextEdit>

namespace report_utils
{

namespace
{
QColor reportStatusColor(const QString &status)
{
    if (status == "open") {
        return QColor("#e74c3c");
    }
    if (status == "assigned") {
        return QColor("#f39c12");
    }
    if (status == "resolved") {
        return QColor("#27ae60");
    }
    if (status == "dismissed") {
        return QColor("#95a5a6");
    }
    return QColor();
}

void setReportStatusItemColor(QTableWidgetItem *item, const QString &status)
{
    const QColor color = reportStatusColor(status);
    if (color.isValid()) {
        item->setForeground(QBrush(color));
    }
}
} // namespace

QString formatReportCategory(const QString &category)
{
    QString result = category;
    result.replace('_', ' ');
    return result.left(1).toUpper() + result.mid(1);
}

QString formatReportTime(qint64 secondsSinceEpoch)
{
    const QDateTime dt = QDateTime::fromSecsSinceEpoch(secondsSinceEpoch);
    return dt.toString("yyyy-MM-dd hh:mm");
}

void fillReportTableRow(QTableWidget *table,
                        int row,
                        const ServerInfo_Report &report,
                        int idColumn,
                        int timeColumn,
                        int reportedColumn,
                        int categoryColumn,
                        int gameIdColumn,
                        int statusColumn,
                        int assignedColumn)
{
    auto *idItem = new QTableWidgetItem(QString::number(report.report_id()));
    idItem->setData(Qt::UserRole, report.report_id());
    table->setItem(row, idColumn, idItem);

    table->setItem(row, timeColumn, new QTableWidgetItem(formatReportTime(report.report_time())));
    table->setItem(row, reportedColumn, new QTableWidgetItem(QString::fromStdString(report.reported_user_name())));
    table->setItem(row, categoryColumn,
                   new QTableWidgetItem(formatReportCategory(QString::fromStdString(report.category()))));
    table->setItem(row, gameIdColumn,
                   new QTableWidgetItem(report.game_id() > 0 ? QString::number(report.game_id()) : QString()));

    auto *statusItem = new QTableWidgetItem(QString::fromStdString(report.status()));
    setReportStatusItemColor(statusItem, QString::fromStdString(report.status()));
    table->setItem(row, statusColumn, statusItem);

    table->setItem(row, assignedColumn, new QTableWidgetItem(QString::fromStdString(report.assigned_mod_name())));
}

void renderReportDetails(QTextEdit *chatLogEdit,
                         QTextEdit *commentsEdit,
                         const ServerInfo_Report &report,
                         const QString &commentsEmptyText,
                         const QString &moderatorPrefix,
                         const QString &nonModeratorPrefix)
{
    if (report.has_chat_log() && !report.chat_log().empty()) {
        chatLogEdit->setPlainText(QString::fromStdString(report.chat_log()));
    } else {
        chatLogEdit->clear();
    }

    commentsEdit->clear();

    if (report.comments_size() == 0) {
        commentsEdit->setPlainText(commentsEmptyText);
        return;
    }

    for (int i = 0; i < report.comments_size(); ++i) {
        const ServerInfo_ReportComment &c = report.comments(i);
        const QString author = QString::fromStdString(c.author_name());
        const QString text = QString::fromStdString(c.comment_text());
        const QString prefix = c.is_moderator() ? moderatorPrefix : nonModeratorPrefix;
        commentsEdit->append(
            QString("[%1] %2 %3:\n%4\n").arg(formatReportTime(c.comment_time()), prefix, author, text));
    }
}

} // namespace report_utils
