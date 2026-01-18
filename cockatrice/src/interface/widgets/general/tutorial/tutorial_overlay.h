#ifndef COCKATRICE_TUTORIAL_OVERLAY_H
#define COCKATRICE_TUTORIAL_OVERLAY_H

#include <QWidget>

class BubbleWidget;
class QLabel;
class QFrame;

class TutorialOverlay : public QWidget
{
    Q_OBJECT
public slots:
    void showEvent(QShowEvent *event) override;

public:
    explicit TutorialOverlay(QWidget *parent = nullptr);

    void setTargetWidget(QWidget *w);
    void updateHoleRect();
    void setText(const QString &t);
    QRect currentHoleRect() const;
    void setTitle(const QString &title);
    void setBlocking(bool blockInput);

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
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QRect computeBubbleRect(const QRect &hole, const QSize &bubbleSize) const;
    void recomputeLayout();

    QWidget *targetWidget = nullptr;

    BubbleWidget *bubble = nullptr;
    QFrame *controlBar = nullptr;

    QLabel *titleLabel = nullptr;

    QString tutorialText;

    QRect cachedHoleRect;
    QRect highlightBubbleRect;

    bool blockInput = true;
};

#endif // COCKATRICE_TUTORIAL_OVERLAY_H
