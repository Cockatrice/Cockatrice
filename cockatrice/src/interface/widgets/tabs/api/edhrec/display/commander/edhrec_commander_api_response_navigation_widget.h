/**
 * @file edhrec_commander_api_response_navigation_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_NAVIGATION_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_NAVIGATION_WIDGET_H

#include "../../api_response/cards/edhrec_commander_api_response_commander_details.h"

#include <QWidget>

class QPushButton;
class EdhrecCommanderApiResponseBudgetNavigationWidget;
class EdhrecCommanderApiResponseBracketNavigationWidget;
class QGridLayout;
class EdhrecCommanderApiResponseNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderApiResponseNavigationWidget(
        QWidget *parent,
        const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
        QString baseUrl);
    void retranslateUi();

public slots:
    void actRequestCommanderNavigation();
    void actRequestComboNavigation();
    void actRequestAverageDeckNavigation();
signals:
    void requestUrl(QString url);

private:
    QGridLayout *layout;

    EdhrecCommanderApiResponseBracketNavigationWidget *bracketNavigationWidget;
    EdhrecCommanderApiResponseBudgetNavigationWidget *budgetNavigationWidget;

    QPushButton *comboPushButton;
    QPushButton *averageDeckPushButton;

    EdhrecCommanderApiResponseCommanderDetails commanderDetails;

    QString addNavigationOptionsToUrl(QString baseUrl);
    QString buildComboUrl() const;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_NAVIGATION_WIDGET_H
