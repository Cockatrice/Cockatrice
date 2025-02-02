#ifndef BANNER_WIDGET_H
#define BANNER_WIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class BannerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannerWidget(QWidget *parent,
                          const QString &text,
                          Qt::Orientation orientation = Qt::Vertical,
                          int transparency = 80);
    void mousePressEvent(QMouseEvent *event) override;
    void setText(const QString &text) const;
    void setBuddy(QWidget *_buddy)
    {
        buddy = _buddy;
    }
    QString getText() const
    {
        return bannerLabel->text();
    }
    void setClickable(bool _clickable)
    {
        clickable = _clickable;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout *layout;
    QLabel *bannerLabel;
    Qt::Orientation gradientOrientation;
    int transparency; // Transparency percentage for the gradient
    QWidget *buddy;
    bool clickable = true;
signals:
    void buddyVisibilityChanged();
private slots:
    void toggleBuddyVisibility() const;
};

#endif // BANNER_WIDGET_H
