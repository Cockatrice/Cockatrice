/**
 * @file edhrec_top_commanders_api_response_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_TOP_COMMANDERS_API_RESPONSE_DISPLAY_WIDGET_H
#define EDHREC_TOP_COMMANDERS_API_RESPONSE_DISPLAY_WIDGET_H

#include "../../api_response/top_commanders/edhrec_top_commanders_api_response.h"

#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class EdhrecTopCommandersApiResponseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdhrecTopCommandersApiResponseDisplayWidget(QWidget *parent, EdhrecTopCommandersApiResponse response);
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *layout;
    QVBoxLayout *cardDisplayLayout;
    QScrollArea *scrollArea;
};

#endif // EDHREC_TOP_COMMANDERS_API_RESPONSE_DISPLAY_WIDGET_H
