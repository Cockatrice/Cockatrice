#ifndef STYLEDLABEL_H
#define STYLEDLABEL_H

#include <QLabel>

class ShadowBackgroundLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ShadowBackgroundLabel(QWidget *parent, const QString &text);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override; // Custom painting logic
};

#endif // STYLEDLABEL_H
