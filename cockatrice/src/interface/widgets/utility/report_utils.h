#ifndef REPORT_UTILS_H
#define REPORT_UTILS_H

#include <QFont>
#include <QString>
#include <QtGlobal>

class QTableWidget;
class QTextEdit;
class ServerInfo_Report;

namespace report_utils
{

/** @brief Font for the report chat-log views: the theme's ChatLog role when it
 *         sets one, otherwise the monospace font these views used before the
 *         role existed. Shared by the three report surfaces so they cannot
 *         drift apart. */
QFont reportLogFont();

QString formatReportCategory(const QString &category);
QString formatReportTime(qint64 secondsSinceEpoch);

void fillReportTableRow(QTableWidget *table,
                        int row,
                        const ServerInfo_Report &report,
                        int idColumn,
                        int timeColumn,
                        int reportedColumn,
                        int categoryColumn,
                        int gameIdColumn,
                        int statusColumn,
                        int assignedColumn);

void renderReportDetails(QTextEdit *chatLogEdit,
                         QTextEdit *commentsEdit,
                         const ServerInfo_Report &report,
                         const QString &commentsEmptyText,
                         const QString &moderatorPrefix,
                         const QString &nonModeratorPrefix);

} // namespace report_utils

#endif // REPORT_UTILS_H
