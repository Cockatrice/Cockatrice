/**
 * @file shadow_background_label.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef STYLEDLABEL_H
#define STYLEDLABEL_H

#include <QLabel>

class ShadowBackgroundLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ShadowBackgroundLabel(QWidget *parent, const QString &text);
    void setLabelText(const QString &text);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override; // Custom painting logic
};

#endif // STYLEDLABEL_H
