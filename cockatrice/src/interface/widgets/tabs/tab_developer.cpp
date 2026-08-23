/**
 * @file tab_developer.cpp
 * @ingroup ServerTabs
 */
//! \todo Document this file.

#include "tab_developer.h"

#include <QDateTime>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_get_server_stats.pb.h>
#include <libcockatrice/protocol/pb/response_get_server_stats.pb.h>
#include <libcockatrice/protocol/pending_command.h>

TabDeveloper::TabDeveloper(TabSupervisor *_tabSupervisor, AbstractClient *_client)
    : Tab(_tabSupervisor), client(_client)
{
    statsTable = new QTableWidget(0, 2);
    statsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    statsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    statsTable->horizontalHeader()->setStretchLastSection(true);
    statsTable->verticalHeader()->setVisible(false);

    statusLabel = new QLabel;

    refreshButton = new QPushButton;
    refreshButton->setAutoDefault(true);
    connect(refreshButton, &QPushButton::clicked, this, &TabDeveloper::refreshClicked);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(statusLabel, 1, Qt::AlignLeft);
    buttonLayout->addWidget(refreshButton, 0, Qt::AlignRight);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statsTable);
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

void TabDeveloper::appendStatRow(const QString &name, const QString &value)
{
    const int row = statsTable->rowCount();
    statsTable->insertRow(row);
    statsTable->setItem(row, 0, new QTableWidgetItem(name));
    statsTable->setItem(row, 1, new QTableWidgetItem(value));
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
        statusLabel->setText(tr("Failed to collect server statistics."));
        return;
    }

    const Response_GetServerStats &response = resp.GetExtension(Response_GetServerStats::ext);

    statsTable->setRowCount(0);
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

    statsTable->resizeColumnsToContents();
    statusLabel->setText(tr("Updated %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
}
