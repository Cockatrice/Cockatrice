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
    explicit DeckPreviewColorCircleWidget(QChar color, QWidget *parent = nullptr);

    void setColorActive(bool active);
    QChar getColorChar() const;

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
    explicit DeckPreviewColorIdentityWidget(const QString &colorIdentity, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_WIDGET_H
