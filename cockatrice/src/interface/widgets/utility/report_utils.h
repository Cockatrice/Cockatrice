#ifndef REPORT_UTILS_H
#define REPORT_UTILS_H

#include <QString>
#include <libcockatrice/protocol/pb/serverinfo_report.pb.h>

class QTableWidget;
class QTextEdit;

namespace report_utils
{

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
