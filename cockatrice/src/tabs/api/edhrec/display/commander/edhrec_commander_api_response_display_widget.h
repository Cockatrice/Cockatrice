/**
 * @file edhrec_commander_api_response_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_DISPLAY_WIDGET_H

#include "../../api_response/commander/edhrec_commander_api_response.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class EdhrecCommanderApiResponseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdhrecCommanderApiResponseDisplayWidget(QWidget *parent,
                                                     EdhrecCommanderApiResponse response,
                                                     QString baseUrl);
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onSplitterChange();

private:
    QHBoxLayout *layout;
    QVBoxLayout *cardDisplayLayout;
    QScrollArea *scrollArea;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_DISPLAY_WIDGET_H
