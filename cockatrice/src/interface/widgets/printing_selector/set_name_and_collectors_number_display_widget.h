/**
 * @file set_name_and_collectors_number_display_widget.h
 * @ingroup CardExtraInfoWidgets
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef SET_NAME_AND_COLLECTORS_NUMBER_DISPLAY_WIDGET_H
#define SET_NAME_AND_COLLECTORS_NUMBER_DISPLAY_WIDGET_H

#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

class SetNameAndCollectorsNumberDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    SetNameAndCollectorsNumberDisplayWidget(QWidget *parent,
                                            const QString &setName,
                                            const QString &collectorsNumber,
                                            QSlider *cardSizeSlider);
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void adjustFontSize(int scalePercentage);

private:
    QVBoxLayout *layout;
    QLabel *setName;
    QLabel *collectorsNumber;
    QSlider *cardSizeSlider;
};

#endif // SET_NAME_AND_COLLECTORS_NUMBER_DISPLAY_WIDGET_H
