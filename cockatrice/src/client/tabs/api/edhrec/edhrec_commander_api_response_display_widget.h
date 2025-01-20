#ifndef EDHREC_COMMANDER_API_RESPONSE_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_DISPLAY_WIDGET_H

#include "api_response/edhrec_commander_api_response.h"

#include <QVBoxLayout>
#include <QWidget>

class EdhrecCommanderApiResponseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdhrecCommanderApiResponseDisplayWidget(QWidget *parent, EdhrecCommanderApiResponse response);
    void resizeEvent(QResizeEvent *event) override;

private:
    QVBoxLayout *layout;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_DISPLAY_WIDGET_H
