/**
 * @file edhrec_top_commanders_api_response_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 */
//! \todo Document this file.

#ifndef EDHREC_TOP_COMMANDERS_API_RESPONSE_DISPLAY_WIDGET_H
#define EDHREC_TOP_COMMANDERS_API_RESPONSE_DISPLAY_WIDGET_H

#include <QWidget>
#include <qtmetamacros.h>

class EdhrecTopCommandersApiResponse;
class QHBoxLayout;
class QScrollArea;
class QVBoxLayout;

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
