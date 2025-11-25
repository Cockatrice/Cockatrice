//
// Created by Ascor on 25-Nov-25.
//

#ifndef COCKATRICE_BACKGROUND_PLATE_WIDGET_H
#define COCKATRICE_BACKGROUND_PLATE_WIDGET_H

#include <QWidget>

class BackgroundPlateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BackgroundPlateWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // COCKATRICE_BACKGROUND_PLATE_WIDGET_H
