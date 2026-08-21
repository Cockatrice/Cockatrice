#ifndef COCKATRICE_PLAYMAT_UTILS_H
#define COCKATRICE_PLAYMAT_UTILS_H

#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <libcockatrice/deck_list/deck_list.h>

inline qreal playmatMaxZoom()
{
    return 4.0;
}

/**
 * @brief Width of the outer viewing window: full card width trimmed by the
 *        horizontal margins. Guarded against margins summing to >= 1.
 */
inline qreal playmatVisibleWidth(const QSize &fullCardSize, const PlaymatParams &params)
{
    const qreal srcW = fullCardSize.width();
    const qreal marginL = params.marginPctL * srcW;
    const qreal marginR = params.marginPctR * srcW;
    return qMax(0.0, srcW - marginL - marginR);
}

/**
 * @brief Zoom clamped to the range where every step renders differently.
 *
 * The square sampling window is visibleWidth / zoom, zooming out past
 * visibleWidth / min(card width, height) would sample beyond the card itself,
 * which both looks broken and makes whole ranges of the parameter dead. The
 * floor is therefore derived from the actual image instead of a static value,
 * and is shared verbatim by the render path and the editor's gesture math so
 * the two can never disagree.
 */
inline qreal playmatClampedZoom(const QSize &fullCardSize, const PlaymatParams &params)
{
    const qreal minDim = qMin<qreal>(fullCardSize.width(), fullCardSize.height());
    const qreal visibleW = playmatVisibleWidth(fullCardSize, params);
    const qreal zoomOutFloor = (minDim > 0.0 && visibleW > 0.0) ? visibleW / minDim : 1.0;
    return qBound(qMin(zoomOutFloor, playmatMaxZoom()), params.zoom, playmatMaxZoom());
}

/**
 * @brief Side of the square sampling window actually rendered for these
 *        parameters. Never exceeds either card dimension, so the source rect
 *        always lies within the image (vertical travel remains for panning
 *        whenever the art is taller than it is wide).
 */
inline qreal playmatWindowSide(const QSize &fullCardSize, const PlaymatParams &params)
{
    const qreal visibleW = playmatVisibleWidth(fullCardSize, params);
    if (visibleW <= 0.0) {
        return 0.0;
    }
    return visibleW / playmatClampedZoom(fullCardSize, params);
}

/**
 * @brief Computes the source region of the full resolution card image to use as a playmat.
 *
 * Parameters are relative to the full card image. horizontal margins trim the
 * card borders (shifting them pans the window), verticalOffset places the top
 * edge of the sampling window within its available travel, and zoom scales
 * into the trimmed span. The result always lies within the card image bounds.
 *
 * @param fullCardSize Size of the full card image.
 * @param params Positioning parameters.
 * @return Source rectangle in full card image pixel coordinates.
 */
inline QRectF computeArtSourceRect(const QSize &fullCardSize, const PlaymatParams &params)
{
    const qreal srcW = fullCardSize.width();
    const qreal srcH = fullCardSize.height();

    // Square sampling window, keeps art unskewed, never exceeds the card on
    // either axis thanks to the zoom floor in playmatWindowSide().
    const qreal side = playmatWindowSide(fullCardSize, params);

    // verticalOffset places the TOP edge of the sampling window itself within
    // its travel, so the full [0, 1] parameter range is live at every zoom and
    // the window can always reach the very top (0.0) and bottom (1.0) of the
    // art. At zoom 1 this is byte identical to the released formula, because
    // the sampling window then equals the outer trimmed span.
    const qreal offset = qBound(0.0, params.verticalOffset, 1.0);
    const qreal y = offset * qMax(0.0, srcH - side);

    // Horizontally the sampling window sits centered inside the trimmed span
    // (margins pan it), zooming out can make it wider than that span, so it
    // is then kept within the image, an edge stop, never an invalid rect.
    const qreal outerW = playmatVisibleWidth(fullCardSize, params);
    const qreal x = qBound(0.0, params.marginPctL * srcW + (outerW - side) / 2.0, qMax(0.0, srcW - side));

    return QRectF(x, y, side, side);
}

/**
 * @brief Returns the destination rectangle that fits a source of the given aspect
 *        ratio into dstArea using "cover" semantics (no distortion, overflows cropped).
 *
 * @param dstArea Area to fill.
 * @param srcSize Size of the source, only its aspect ratio matters.
 * @return Destination rectangle centered in dstArea.
 */
inline QRectF coverFitRect(const QRectF &dstArea, const QSizeF &srcSize)
{
    const qreal srcAspect = srcSize.width() / srcSize.height();
    const qreal dstAspect = dstArea.width() / dstArea.height();

    if (srcAspect > dstAspect) {
        const qreal dstW = dstArea.height() * srcAspect;
        return QRectF(dstArea.left() + (dstArea.width() - dstW) / 2.0, dstArea.top(), dstW, dstArea.height());
    }

    const qreal dstH = dstArea.width() / srcAspect;
    return QRectF(dstArea.left(), dstArea.top() + (dstArea.height() - dstH) / 2.0, dstArea.width(), dstH);
}

/**
 * @brief Fits a rectangle of the given aspect ratio into dstArea, centered,
 *        touching the constraining dimension ("aspect fit" of the FRAME
 *        itself, not of a source image).
 */
inline QRectF aspectFitRect(const QRectF &dstArea, qreal aspect)
{
    if (aspect <= 0.0) {
        return dstArea;
    }
    qreal w = qMin(dstArea.width(), dstArea.height() * aspect);
    qreal h = w / aspect;
    return QRectF(dstArea.left() + (dstArea.width() - w) / 2.0, dstArea.top() + (dstArea.height() - h) / 2.0, w, h);
}

#endif // COCKATRICE_PLAYMAT_UTILS_H
