/**
 * @file edhrec_top_cards_api_response_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 */
//! \todo Document this file.

#ifndef EDHREC_TOP_CARDS_API_RESPONSE_DISPLAY_WIDGET_H
#define EDHREC_TOP_CARDS_API_RESPONSE_DISPLAY_WIDGET_H

#include <QWidget>
#include <qtmetamacros.h>

class EdhrecTopCardsApiResponse;
class QHBoxLayout;
class QScrollArea;
class QVBoxLayout;

class EdhrecTopCardsApiResponseDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdhrecTopCardsApiResponseDisplayWidget(QWidget *parent, EdhrecTopCardsApiResponse response);
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *layout;
    QVBoxLayout *cardDisplayLayout;
    QScrollArea *scrollArea;
};

#endif // EDHREC_TOP_CARDS_API_RESPONSE_DISPLAY_WIDGET_H
