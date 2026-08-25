/**
 * @file tab_developer.cpp
 * @ingroup ServerTabs
 */
//! \todo Document this file.

#include "tab_developer.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_get_server_stats.pb.h>
#include <libcockatrice/protocol/pb/response_get_server_stats.pb.h>
#include <libcockatrice/protocol/pending_command.h>

TabDeveloper::TabDeveloper(TabSupervisor *_tabSupervisor, AbstractClient *_client)
    : Tab(_tabSupervisor), client(_client)
{
    statsTable = new QTableWidget(0, 2);
    statsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    statsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    statsTable->verticalHeader()->setVisible(false);
    statsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    statsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    statsTable->horizontalHeader()->setStretchLastSection(true);

    commandTable = new QTableWidget(0, 4);
    commandTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    commandTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    commandTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    commandTable->setSelectionMode(QAbstractItemView::SingleSelection);
    commandTable->verticalHeader()->setVisible(false);
    commandTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    commandTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    commandTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    commandTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);

    statusLabel = new QLabel;

    refreshButton = new QPushButton;
    refreshButton->setAutoDefault(true);
    connect(refreshButton, &QPushButton::clicked, this, &TabDeveloper::refreshClicked);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(statusLabel, 1, Qt::AlignLeft);
    buttonLayout->addWidget(refreshButton, 0, Qt::AlignRight);

    auto *tableLayout = new QHBoxLayout;
    tableLayout->addWidget(statsTable, 1);
    tableLayout->addWidget(commandTable, 2);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(tableLayout, 1);
    mainLayout->addLayout(buttonLayout);

    auto *central = new QWidget;
    central->setLayout(mainLayout);
    setCentralWidget(central);

    retranslateUi();
}

void TabDeveloper::retranslateUi()
{
    refreshButton->setText(tr("Refresh server stats"));
    statsTable->setHorizontalHeaderLabels(QString(tr("Statistic;Value")).split(";"));
    commandTable->setHorizontalHeaderLabels(QString(tr("Command;Count;Total ms;Avg ms")).split(";"));
    if (statsTable->rowCount() == 0) {
        statusLabel->clear();
    }
}

QString TabDeveloper::formatBytes(quint64 bytes)
{
    const quint64 kib = 1024;
    const quint64 mib = 1024 * kib;
    const quint64 gib = 1024 * mib;
    if (bytes >= gib) {
        return tr("%1 GiB").arg(QString::number(bytes / static_cast<double>(gib), 'f', 2));
    }
    if (bytes >= mib) {
        return tr("%1 MiB").arg(QString::number(bytes / static_cast<double>(mib), 'f', 2));
    }
    if (bytes >= kib) {
        return tr("%1 KiB").arg(QString::number(bytes / static_cast<double>(kib), 'f', 2));
    }
    return tr("%1 bytes").arg(bytes);
}

QString TabDeveloper::formatDurationMs(qint64 ms)
{
    if (ms >= 1000) {
        return tr("%1 s").arg(QString::number(ms / 1000.0, 'f', 2));
    }
    return tr("%1 ms").arg(ms);
}

void TabDeveloper::appendStatRow(const QString &name, const QString &value)
{
    const int row = statsTable->rowCount();
    statsTable->insertRow(row);
    statsTable->setItem(row, 0, new QTableWidgetItem(name));
    statsTable->setItem(row, 1, new QTableWidgetItem(value));
}

void TabDeveloper::appendSeparatorRow(const QString &sectionTitle)
{
    const int row = statsTable->rowCount();
    statsTable->insertRow(row);
    auto *labelItem = new QTableWidgetItem(sectionTitle);
    auto font = labelItem->font();
    font.setBold(true);
    labelItem->setFont(font);
    labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsSelectable);
    statsTable->setItem(row, 0, labelItem);
    statsTable->setItem(row, 1, new QTableWidgetItem(QString()));
}

void TabDeveloper::refreshClicked()
{
    Command_GetServerStats cmd;
    PendingCommand *pend = client->prepareDeveloperCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &TabDeveloper::serverStatsResponse);
    client->sendCommand(pend);
}

void TabDeveloper::serverStatsResponse(const Response &resp)
{
    if (resp.response_code() != Response::RespOk) {
        statusLabel->setText(tr("No server statistics available yet."));
        return;
    }

    const Response_GetServerStats &response = resp.GetExtension(Response_GetServerStats::ext);

    statsTable->setRowCount(0);

    // Overview section
    appendStatRow(tr("Registered users online"), QString::number(response.users_count()));
    appendStatRow(tr("Moderators online"), QString::number(response.mods_count()));
    appendStatRow(tr("Games running"), QString::number(response.games_count()));
    appendStatRow(tr("Traffic sent (last tick)"), formatBytes(response.tx_bytes()));
    appendStatRow(tr("Traffic received (last tick)"), formatBytes(response.rx_bytes()));

    const qint64 uptime = static_cast<qint64>(response.uptime_secs());
    const int days = static_cast<int>(uptime / 86400);
    const int hours = static_cast<int>((uptime % 86400) / 3600);
    const int minutes = static_cast<int>((uptime % 3600) / 60);
    appendStatRow(tr("Server uptime"), days > 0 ? tr("%1d %2h %3m").arg(days).arg(hours).arg(minutes)
                                                : tr("%1h %2m").arg(hours).arg(minutes));

    const QDateTime snapshotTime = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(response.timest()));
    appendStatRow(tr("Snapshot taken"), snapshotTime.toLocalTime().toString("yyyy-MM-dd HH:mm"));

    // Live metrics section
    appendSeparatorRow(tr("Live Metrics"));
    appendStatRow(tr("Cards in live games"), QString::number(response.cards_in_games()));
    appendStatRow(tr("Total commands processed"), QString::number(response.total_commands()));

    if (response.total_commands() > 0) {
        const double avgMs = static_cast<double>(response.total_command_time_ms()) / response.total_commands();
        appendStatRow(tr("Avg command time"), QString::number(avgMs, 'f', 2) + " ms");
    }
    appendStatRow(tr("Active command types"), QString::number(response.active_command_types()));

    appendStatRow(tr("Event loop stalls"), QString::number(response.eventloop_stalls_total()));
    appendStatRow(tr("Last stall overshoot"), formatDurationMs(response.eventloop_last_stall_ms()));
    appendStatRow(tr("Worst stall overshoot"), formatDurationMs(response.eventloop_max_stall_ms()));

    if (response.game_start_count() > 0) {
        appendStatRow(tr("Game starts"), QString::number(response.game_start_count()));
        const double avgStartMs = static_cast<double>(response.game_start_total_ms()) / response.game_start_count();
        appendStatRow(tr("Avg game start time"), QString::number(avgStartMs, 'f', 1) + " ms");
    }

    // Per-command breakdown table
    QList<CommandStats> sortedStats(response.command_stats().begin(), response.command_stats().end());
    std::sort(sortedStats.begin(), sortedStats.end(),
              [](const auto &a, const auto &b) { return a.total_ms() > b.total_ms(); });

    commandTable->setRowCount(0);
    for (const auto &cs : sortedStats) {
        const int row = commandTable->rowCount();
        commandTable->insertRow(row);
        commandTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(cs.command_name())));

        auto *countItem = new QTableWidgetItem(QString::number(cs.count()));
        countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        commandTable->setItem(row, 1, countItem);

        auto *totalItem = new QTableWidgetItem(QString::number(cs.total_ms()));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        commandTable->setItem(row, 2, totalItem);

        const double avg = cs.count() > 0 ? static_cast<double>(cs.total_ms()) / cs.count() : 0.0;
        auto *avgItem = new QTableWidgetItem(QString::number(avg, 'f', 2));
        avgItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        commandTable->setItem(row, 3, avgItem);
    }
    commandTable->resizeColumnsToContents();
    statsTable->resizeColumnsToContents();
    commandTable->resizeColumnsToContents();

    statusLabel->setText(tr("Updated %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
}
