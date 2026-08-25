/**
 * @file tab_developer.h
 * @ingroup ServerTabs
 */
//! \todo Document this file.

#ifndef TAB_DEVELOPER_H
#define TAB_DEVELOPER_H

#include "tab.h"

class AbstractClient;
class QLabel;
class QPushButton;
class QTableWidget;
class Response;

class TabDeveloper : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    QTableWidget *statsTable;
    QTableWidget *commandTable;
    QPushButton *refreshButton;
    QLabel *statusLabel;

    void appendStatRow(const QString &name, const QString &value);
    void appendSeparatorRow(const QString &sectionTitle);
    static QString formatBytes(quint64 bytes);
    static QString formatDurationMs(qint64 ms);

private slots:
    void refreshClicked();
    void serverStatsResponse(const Response &resp);

public:
    explicit TabDeveloper(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Developer");
    }
};

#endif
