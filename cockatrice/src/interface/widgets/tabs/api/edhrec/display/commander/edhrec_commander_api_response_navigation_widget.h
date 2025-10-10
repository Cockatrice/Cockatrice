/**
 * @file edhrec_commander_api_response_navigation_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_NAVIGATION_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_NAVIGATION_WIDGET_H

#include "edhrec_api_response_commander_details_display_widget.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class EdhrecCommanderApiResponseNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderApiResponseNavigationWidget(
        QWidget *parent,
        const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
        QString baseUrl);
    void retranslateUi();
    void applyOptionsFromUrl(const QString &url);

public slots:
    void actRequestCommanderNavigation();
    void actRequestComboNavigation();
    void actRequestAverageDeckNavigation();
signals:
    void requestUrl(QString url);

private:
    QGridLayout *layout;

    QLabel *gameChangerLabel;
    QLabel *budgetLabel;

    QStringList gameChangerOptions = {"", "core", "upgraded", "optimized"};
    QStringList budgetOptions = {"", "budget", "expensive"};

    QString selectedGameChanger;
    QString selectedBudget;

    QMap<QString, QPushButton *> gameChangerButtons;
    QMap<QString, QPushButton *> budgetButtons;

    QPushButton *comboPushButton;
    QPushButton *averageDeckPushButton;

    EdhrecCommanderApiResponseCommanderDetails commanderDetails;

    void updateOptionButtonSelection(QMap<QString, QPushButton *> &buttons, const QString &selectedKey);
    QString addNavigationOptionsToUrl(QString baseUrl);
    QString buildComboUrl() const;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_NAVIGATION_WIDGET_H
