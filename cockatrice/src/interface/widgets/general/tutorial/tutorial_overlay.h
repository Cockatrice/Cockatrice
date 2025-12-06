#ifndef TUTORIAL_OVERLAY_H
#define TUTORIAL_OVERLAY_H

#include <QWidget>

class QFrame;
class QLabel;
class QPushButton;
class BubbleWidget;

class TutorialOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit TutorialOverlay(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setBlocking(bool block);
    void setTargetWidget(QWidget *w);
    void setText(const QString &t);
    void setInteractive(bool interactive, bool clickThrough);
    void setInteractionHint(const QString &hint);
    void showValidationHint(const QString &hint);
    void setProgress(int stepNum, int totalSteps, int overallStep, int overallTotal, const QString &sequenceTitle);

    void parentResized();
    QRect currentHoleRect() const;

signals:
    void nextStep();
    void prevStep();
    void nextSequence();
    void prevSequence();
    void skipTutorial();
    void targetClicked();

protected:
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void updateMask();
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void recomputeLayout();
    QRect computeBubbleRect(const QRect &hole, const QSize &bubbleSize) const;

    QWidget *targetWidget = nullptr;
    QFrame *controlBar = nullptr;
    QLabel *titleLabel = nullptr;
    QPushButton *nextButton = nullptr;
    QPushButton *nextSeqButton = nullptr;
    BubbleWidget *bubble = nullptr;

    QString tutorialText;
    QRect highlightBubbleRect;
    bool blockInput = true;
    bool isInteractive = false;
    bool allowClickThrough = false;
};

#endif // TUTORIAL_OVERLAY_H