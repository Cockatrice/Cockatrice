#include "dlg_my_reports.h"

#include "../utility/report_utils.h"
#include "abstract_client.h"

#include <QFontDatabase>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <libcockatrice/protocol/pb/command_report_add_comment.pb.h>
#include <libcockatrice/protocol/pb/command_report_details.pb.h>
#include <libcockatrice/protocol/pb/command_report_my_list.pb.h>
#include <libcockatrice/protocol/pb/response_report_details.pb.h>
#include <libcockatrice/protocol/pb/response_report_my_list.pb.h>
#include <libcockatrice/protocol/pending_command.h>

namespace
{
constexpr int COL_ID = 0;
constexpr int COL_TIME = 1;
constexpr int COL_REPORTED = 2;
constexpr int COL_CATEGORY = 3;
constexpr int COL_GAMEID = 4;
constexpr int COL_STATUS = 5;
constexpr int COL_ASSIGNED = 6;
constexpr int COL_COUNT = 7;
} // namespace

DlgMyReports::DlgMyReports(AbstractClient *_client, QWidget *parent)
    : QDialog(parent), client(_client), selectedReportId(-1)
{
    setWindowTitle(tr("My Reports"));
    setMinimumSize(800, 500);

    table = new QTableWidget(0, COL_COUNT);
    table->setHorizontalHeaderLabels(
        {tr("#"), tr("Time"), tr("Reported User"), tr("Category"), tr("Game ID"), tr("Status"), tr("Assigned To")});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(COL_TIME, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(COL_REPORTED, QHeaderView::Stretch);
    connect(table, &QTableWidget::itemSelectionChanged, this, &DlgMyReports::onSelectionChanged);

    auto *detailsGroup = new QGroupBox(tr("Report Details"));
    descriptionEdit = new QTextEdit;
    descriptionEdit->setReadOnly(true);
    descriptionEdit->setFixedHeight(80);

    auto *chatGroup = new QGroupBox(tr("Chat Log Context"));
    chatLogEdit = new QTextEdit;
    chatLogEdit->setReadOnly(true);
    QFont monoFont("monospace");
    monoFont.setStyleHint(QFont::Monospace);
    const int systemPointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    if (systemPointSize > 0) {
        monoFont.setPointSize(systemPointSize);
    }
    chatLogEdit->setFont(monoFont);
    auto *chatLayout = new QVBoxLayout(chatGroup);
    chatLayout->setContentsMargins(4, 4, 4, 4);
    chatLayout->addWidget(chatLogEdit);

    auto *commentsLabel = new QLabel(tr("Comments:"));
    commentsEdit = new QTextEdit;
    commentsEdit->setReadOnly(true);
    commentsEdit->setFixedHeight(120);

    auto *addCommentLabel = new QLabel(tr("Add a comment:"));
    commentInput = new QLineEdit;
    commentInput->setPlaceholderText(tr("Type your comment here..."));
    commentButton = new QPushButton(tr("Send"));
    commentButton->setEnabled(false);
    connect(commentButton, &QPushButton::clicked, this, &DlgMyReports::addComment);
    connect(commentInput, &QLineEdit::returnPressed, this, &DlgMyReports::addComment);

    auto *detailsLayout = new QVBoxLayout(detailsGroup);
    detailsLayout->setContentsMargins(4, 4, 4, 4);
    detailsLayout->addWidget(descriptionEdit);
    detailsLayout->addWidget(chatGroup);
    detailsLayout->addWidget(commentsLabel);
    detailsLayout->addWidget(commentsEdit);
    detailsLayout->addWidget(addCommentLabel);
    auto *commentRow = new QHBoxLayout;
    commentRow->addWidget(commentInput);
    commentRow->addWidget(commentButton);
    detailsLayout->addLayout(commentRow);

    closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    refreshButton = new QPushButton(tr("Refresh"));
    connect(refreshButton, &QPushButton::clicked, this, &DlgMyReports::refreshList);

    statusLabel = new QLabel;

    auto *bottomBar = new QHBoxLayout;
    bottomBar->addWidget(statusLabel);
    bottomBar->addStretch();
    bottomBar->addWidget(refreshButton);
    bottomBar->addWidget(closeButton);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(table, 1);
    layout->addWidget(detailsGroup);
    layout->addLayout(bottomBar);

    setActionsEnabled(false);
    refreshList();
}

void DlgMyReports::refreshList()
{
    selectedReportIdBeforeRefresh = selectedReportId;
    commentDraftBeforeRefresh = commentInput->text();
    statusLabel->setText(tr("Loading..."));
    refreshButton->setEnabled(false);
    table->setRowCount(0);
    currentReports.clear();
    descriptionEdit->clear();
    chatLogEdit->clear();
    commentsEdit->clear();
    commentInput->clear();
    setActionsEnabled(false);
    selectedReportId = -1;

    Command_ReportMyList cmd;

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgMyReports::reportListResponse);
    client->sendCommand(pend);
}

void DlgMyReports::reportListResponse(const Response &response)
{
    refreshButton->setEnabled(true);

    if (response.response_code() != Response::RespOk) {
        statusLabel->setText(tr("Failed to load reports."));
        return;
    }

    const Response_ReportMyList &resp = response.GetExtension(Response_ReportMyList::ext);
    currentReports.clear();
    for (int i = 0; i < resp.reports_size(); ++i) {
        currentReports.append(resp.reports(i));
    }

    table->setSortingEnabled(false);
    table->setRowCount(currentReports.size());

    for (int row = 0; row < currentReports.size(); ++row) {
        const ServerInfo_Report &r = currentReports[row];

        report_utils::fillReportTableRow(table, row, r, COL_ID, COL_TIME, COL_REPORTED, COL_CATEGORY, COL_GAMEID,
                                         COL_STATUS, COL_ASSIGNED);
    }

    table->setSortingEnabled(true);
    table->resizeColumnsToContents();
    table->horizontalHeader()->setSectionResizeMode(COL_REPORTED, QHeaderView::Stretch);

    if (selectedReportIdBeforeRefresh >= 0) {
        for (int row = 0; row < table->rowCount(); ++row) {
            if (table->item(row, COL_ID) &&
                table->item(row, COL_ID)->data(Qt::UserRole).toInt() == selectedReportIdBeforeRefresh) {
                table->setCurrentCell(row, 0);
                break;
            }
        }
    }

    if (commentInput->text().isEmpty()) {
        commentInput->setText(commentDraftBeforeRefresh);
    }

    statusLabel->setText(tr("%1 report(s)").arg(currentReports.size()));
}

void DlgMyReports::onSelectionChanged()
{
    const int row = table->currentRow();
    if (row < 0 || !table->item(row, COL_ID)) {
        descriptionEdit->clear();
        chatLogEdit->clear();
        commentsEdit->clear();
        commentInput->clear();
        commentButton->setEnabled(false);
        selectedReportId = -1;
        return;
    }

    const int reportId = table->item(row, COL_ID)->data(Qt::UserRole).toInt();
    selectedReportId = reportId;

    for (const ServerInfo_Report &r : currentReports) {
        if (r.report_id() == reportId) {
            descriptionEdit->setPlainText(QString::fromStdString(r.description()));
            break;
        }
    }

    chatLogEdit->setPlainText(tr("Loading..."));
    commentsEdit->setPlainText(tr("Loading..."));

    Command_ReportDetails cmd;
    cmd.set_report_id(reportId);

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgMyReports::reportDetailsResponse);
    client->sendCommand(pend);

    QString status = table->item(row, COL_STATUS)->text();
    bool canComment = (status == "open" || status == "assigned");
    commentButton->setEnabled(canComment);
    commentInput->setEnabled(canComment);
    if (!canComment) {
        commentInput->setPlaceholderText(tr("This report is closed."));
    } else {
        commentInput->setPlaceholderText(tr("Type your comment here..."));
    }
}

void DlgMyReports::reportDetailsResponse(const Response &response)
{
    if (response.response_code() != Response::RespOk) {
        if (selectedReportId == -1) {
            return;
        }
        chatLogEdit->clear();
        commentsEdit->setPlainText(tr("Failed to load report details."));
        return;
    }

    const Response_ReportDetails &resp = response.GetExtension(Response_ReportDetails::ext);
    const ServerInfo_Report &r = resp.report();

    if (selectedReportId != r.report_id()) {
        return;
    }

    loadReportDetails(r);
}

void DlgMyReports::loadReportDetails(const ServerInfo_Report &report)
{
    report_utils::renderReportDetails(chatLogEdit, commentsEdit, report, tr("No comments yet."), tr("[Moderator]"),
                                      tr("[You]"));
}

void DlgMyReports::addComment()
{
    if (selectedReportId < 0) {
        return;
    }

    QString text = commentInput->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    commentButton->setEnabled(false);

    Command_ReportAddComment cmd;
    cmd.set_report_id(selectedReportId);
    cmd.set_comment(text.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgMyReports::addCommentResponse);
    client->sendCommand(pend);
}

void DlgMyReports::addCommentResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        commentInput->clear();
        refreshList();
    } else {
        commentButton->setEnabled(true);
    }
}

void DlgMyReports::setActionsEnabled(bool enabled)
{
    commentButton->setEnabled(enabled);
    commentInput->setEnabled(enabled);
}
