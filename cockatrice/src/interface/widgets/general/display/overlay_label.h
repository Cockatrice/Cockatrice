#ifndef COCKATRICE_OVERLAY_LABEL_H
#define COCKATRICE_OVERLAY_LABEL_H

#include <QLabel>

class OverlayLabel : public QLabel
{
    Q_OBJECT
public:
    explicit OverlayLabel(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // COCKATRICE_OVERLAY_LABEL_H
