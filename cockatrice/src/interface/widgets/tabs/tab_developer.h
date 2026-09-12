/**
 * @file tab_developer.h
 * @ingroup ServerTabs
 */
//! \todo Document this file.

#ifndef TAB_DEVELOPER_H
#define TAB_DEVELOPER_H

#include "tab.h"

class AbstractClient;
class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTimer;
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
    QCheckBox *autoRefreshCheckBox;
    QSpinBox *refreshIntervalSpinBox;
    QTimer *autoRefreshTimer;
    bool requestPending = false;

    void appendStatRow(const QString &name, const QString &value);
    void appendSeparatorRow(const QString &sectionTitle);
    static QString formatBytes(quint64 bytes);
    static QString formatDurationMs(qint64 ms);

private slots:
    void refreshClicked();
    void serverStatsResponse(const Response &resp);
    void autoRefreshToggled(bool checked);
    void refreshIntervalChanged();

public:
    explicit TabDeveloper(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Developer");
    }
};

#endif
