#ifndef BANNER_WIDGET_H
#define BANNER_WIDGET_H

#include <QLabel>
#include <QWidget>

class BannerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannerWidget(QWidget *parent,
                          const QString &text,
                          Qt::Orientation orientation = Qt::Vertical,
                          int transparency = 80);
    void mousePressEvent(QMouseEvent *event);
    void setBuddy(QWidget *_buddy)
    {
        buddy = _buddy;
    }
    QString getText() const
    {
        return bannerLabel->text();
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *bannerLabel;
    Qt::Orientation gradientOrientation;
    int transparency; // Transparency percentage for the gradient
    QWidget *buddy;
signals:
    void buddyVisibilityChanged();
private slots:
    void toggleBuddyVisibility() const;
};

#endif // BANNER_WIDGET_H
