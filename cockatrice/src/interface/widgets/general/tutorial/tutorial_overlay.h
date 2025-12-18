#ifndef COCKATRICE_TUTORIAL_OVERLAY_H
#define COCKATRICE_TUTORIAL_OVERLAY_H

#include "tutorial_bubble_widget.h"

#include <QPointer>
#include <QWidget>

class TutorialOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit TutorialOverlay(QWidget *parent = nullptr);

    void setTargetWidget(QWidget *w);
    void setText(const QString &t);
    void parentResized();

signals:
    void nextStep();
    void prevStep();
    void nextSequence();
    void prevSequence();
    void skipTutorial();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;

private:
    QRect targetRectOnOverlay() const;
    QRect computeBubbleRect(const QRect &hole, const QSize &bubbleSize) const;

    QPointer<QWidget> targetWidget;
    QString tutorialText;

    QRect highlightBubbleRect;
    BubbleWidget *bubble;
};

#endif // COCKATRICE_TUTORIAL_OVERLAY_H
