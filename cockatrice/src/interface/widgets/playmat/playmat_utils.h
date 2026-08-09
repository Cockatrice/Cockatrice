#ifndef COCKATRICE_PLAYMAT_UTILS_H
#define COCKATRICE_PLAYMAT_UTILS_H

#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <libcockatrice/deck_list/deck_list.h>

/**
 * @brief Computes the source region of the full-resolution card image to use as a playmat.
 *
 * Parameters are relative to the full card image: horizontal margins trim the card
 * borders, the vertical offset positions a square viewing window, and zoom scales
 * into that window. The result is clamped to the card image bounds.
 *
 * @param fullCardSize Size of the full card image.
 * @param params Positioning parameters.
 * @return Source rectangle in full-card image pixel coordinates.
 */
inline QRectF computeArtSourceRect(const QSize &fullCardSize, const PlaymatParams &params)
{
    const qreal srcW = fullCardSize.width();
    const qreal srcH = fullCardSize.height();

    const qreal marginL = params.marginPctL * srcW;
    const qreal marginR = params.marginPctR * srcW;
    // Guard against margins summing to >= 1 (both are individually in range),
    // which would otherwise make the viewing window negative or zero.
    const qreal visibleW = qMax(0.0, srcW - marginL - marginR);
    const qreal visibleH = visibleW; // square viewing window, keeps art unskewed

    const qreal vCenter = params.verticalOffset * srcH;
    qreal srcY = vCenter - visibleH / 2.0;
    srcY = qBound(0.0, srcY, srcH - visibleH);

    // Guard the zoom divisor; everything that produces params clamps zoom to
    // [0.1, 4.0] already, this keeps the render path self-contained.
    const qreal zoom = qBound(0.1, params.zoom, 4.0);
    const qreal zoomedW = visibleW / zoom;
    const qreal zoomedH = visibleH / zoom;
    const qreal zoomedX = marginL + (visibleW - zoomedW) / 2.0;
    const qreal zoomedY = srcY + (visibleH - zoomedH) / 2.0;

    return QRectF(zoomedX, zoomedY, zoomedW, zoomedH);
}

/**
 * @brief Returns the destination rectangle that fits a source of the given aspect
 *        ratio into dstArea using "cover" semantics (no distortion, overflows cropped).
 *
 * @param dstArea Area to fill.
 * @param srcSize Size of the source; only its aspect ratio matters.
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

#endif // COCKATRICE_PLAYMAT_UTILS_H
