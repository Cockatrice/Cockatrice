/**
 * @file edhrec_top_tags_api_response_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_TOP_TAGS_API_RESPONSE_DISPLAY_WIDGET_H
#define EDHREC_TOP_TAGS_API_RESPONSE_DISPLAY_WIDGET_H

#include <QWidget>

class EdhrecTopTagsApiResponse;
class QHBoxLayout;
class QVBoxLayout;
class QScrollArea;
class EdhrecTopTagsApiResponseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdhrecTopTagsApiResponseDisplayWidget(QWidget *parent, EdhrecTopTagsApiResponse response);
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *layout;
    QVBoxLayout *cardDisplayLayout;
    QScrollArea *scrollArea;
};

#endif // EDHREC_TOP_TAGS_API_RESPONSE_DISPLAY_WIDGET_H
