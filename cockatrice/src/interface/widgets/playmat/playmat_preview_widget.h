#ifndef COCKATRICE_PLAYMAT_PREVIEW_WIDGET_H
#define COCKATRICE_PLAYMAT_PREVIEW_WIDGET_H

#include <QFocusEvent>
#include <QPixmap>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>

/**
 * @brief Interactive crop surface showing how a playmat card art will appear
 *        across the combined table + stack play area.
 *
 * Renders a fixed frame shaped like a fresh board's stack+table area (the
 * most generous framing the game produces): it shows the tallest slice of
 * the square crop in normal play, with dimmed strips marking where a wider,
 * developed table crops further, exactly the game's own render pipeline.
 * The widget doubles as the editor's primary crop control: dragging pans the
 * art behind the frame, the wheel zooms, and arrow keys nudge, mirroring
 * the stored parameters (margins pan horizontally, verticalOffset
 * vertically, zoom scales) so no separate numeric controls are needed.
 */
class PlaymatPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaymatPreviewWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    void setParams(const PlaymatParams &params);
    void setAttribution(const QString &attribution);

signals:
    /** @brief Emitted whenever direct manipulation (drag, wheel, keys) changes the crop parameters. */
    void paramsEdited(const PlaymatParams &params);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    QRectF activePlayArea() const;    ///< destination rect used for rendering AND gesture math
    qreal samplingWindowSide() const; ///< clamped square window side, shared with the render path
    qreal widgetToSourceScale() const;
    void applyCropDelta(qreal dMarginL, qreal dMarginR, qreal dOffset, qreal zoomFactor);
    bool sameCrop(const PlaymatParams &a, const PlaymatParams &b) const;
    void restoreSnapshot();

    QPixmap sourcePixmap;
    PlaymatParams params;
    PlaymatParams paramsAtFocusIn; ///< crop as of the latest focus gain, restored by Esc or Backspace
    QString attributionText;
    QPoint lastDragPos; ///< widget space position of the previous mouse move while panning
};

#endif // COCKATRICE_PLAYMAT_PREVIEW_WIDGET_H
