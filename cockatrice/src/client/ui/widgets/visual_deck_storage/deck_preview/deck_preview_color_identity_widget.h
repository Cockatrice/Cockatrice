#ifndef DECK_PREVIEW_COLOR_IDENTITY_WIDGET_H
#define DECK_PREVIEW_COLOR_IDENTITY_WIDGET_H

#include <QChar>
#include <QHBoxLayout>
#include <QSize>
#include <QWidget>

class DeckPreviewColorCircleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorCircleWidget(QChar color, QWidget *parent);

    void setColorActive(bool active);
    QChar getColorChar() const;
    bool getIsActive() const
    {
        return isActive;
    }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QChar colorChar;
    int circleDiameter;
    bool isActive;
};

class DeckPreviewColorIdentityWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorIdentityWidget(QWidget *parent, const QString &colorIdentity);

public slots:
    void toggleUnusedVisibility(bool _visible) const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *layout;
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_WIDGET_H
