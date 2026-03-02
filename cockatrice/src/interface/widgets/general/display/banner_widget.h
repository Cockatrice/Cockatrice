/**
 * @file banner_widget.h
 * @ingroup Widgets
 * @ingroup DeckEditorCardGroupWidgets
 * @ingroup DeckStorageWidgets
 * @brief TODO: Document this.
 */

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
    void setClickable(bool _clickable);
    void setBuddy(QWidget *_buddy);
    [[nodiscard]] QString getText() const
    {
        return bannerLabel->text();
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *iconLabel;
    QLabel *bannerLabel;
    Qt::Orientation gradientOrientation;
    int transparency; // Transparency percentage for the gradient
    QWidget *buddy = nullptr;
    bool clickable = true;
signals:
    void buddyVisibilityChanged();
private slots:
    void toggleBuddyVisibility() const;
    void updateDropdownIconState() const;
};

#endif // BANNER_WIDGET_H
