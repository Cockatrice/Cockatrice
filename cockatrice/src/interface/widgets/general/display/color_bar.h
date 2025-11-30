#ifndef COCKATRICE_COLOR_BAR_H
#define COCKATRICE_COLOR_BAR_H

#include <QColor>
#include <QMap>
#include <QString>
#include <QWidget>

/**
 * @class ColorBar
 * @brief A widget for visualizing proportional color distributions as a horizontal bar.
 *
 * This widget renders a horizontal bar divided into colored segments whose widths reflect
 * the relative values associated with each color key in a `QMap<QString, int>`. The class
 * is designed as a small, lightweight, and self-contained visualization component suitable
 * for representing distributions such as color counts, mana statistics, categorical frequencies, and similar data sets.
 *
 * Key features:
 *  - Filled segments for better visual clarity.
 *  - Deterministic alphabetical ordering of color keys.
 *  - Optional minimum percentage threshold for filtering out insignificant segments.
 *  - Mouse-hover tooltips showing each segment’s key, count, and percentage of total.
 *
 * Default color mappings exist for `"R"`, `"G"`, `"U"`, `"W"`, and `"B"`, using named
 * colors, but any string recognized by `QColor` may be used. If an unknown name is provided,
 * the segment will fall back to gray.
 *
 * This component is display-only and does not interpret or mutate domain-level data.
 */
class ColorBar : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a ColorBar widget.
     *
     * @param colors Map of color identifiers to integer counts.
     * @param parent Optional parent widget.
     */
    explicit ColorBar(const QMap<QString, int> &colors, QWidget *parent = nullptr);

    /**
     * @brief Updates the color distribution map.
     * @param colors New color → count mapping.
     *
     * Triggers an immediate repaint.
     */
    void setColors(const QMap<QString, int> &colors);

    /**
     * @brief Sets a minimum percentage threshold below which segments are not drawn.
     *
     * @param treshold Percentage from 0 to 100.
     *
     * Internally converted into a ratio (0.05 = 5%).
     */
    void setMinPercentThreshold(double treshold)
    {
        minRatioThreshold = treshold / 100.0;
    }

    /**
     * @brief Returns the recommended minimum size.
     */
    QSize minimumSizeHint() const override;

protected:
    /**
     * @brief Paints the color distribution bar.
     *
     * Draws:
     *  - A rounded border
     *  - Filled segments for each color
     *  - Only segments above the minimum ratio threshold
     */
    void paintEvent(QPaintEvent *event) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    /**
     * @brief Handles mouse hover entering (Qt6 version).
     */
    void enterEvent(QEnterEvent *event) override;
#else
    /**
     * @brief Handles mouse hover entering (Qt5 version).
     */
    void enterEvent(QEvent *event) override;
#endif

    /**
     * @brief Handles mouse hover leaving.
     */
    void leaveEvent(QEvent *event) override;

    /**
     * @brief Handles mouse movement to update contextual tooltips.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    /// Map of color keys to counts used for rendering.
    QMap<QString, int> colors;

    /// True if the mouse is currently inside the widget.
    bool isHovered = false;

    /// Minimum ratio a segment must exceed to be drawn.
    double minRatioThreshold = 0.0;

    /**
     * @brief Converts a color name into a display QColor.
     *
     * Recognized special keys: `"R", "G", "U", "W", "B"`.
     * Other strings are treated as QColor names or fall back to gray.
     */
    QColor colorFromName(const QString &name) const;

    /**
     * @brief Returns tooltip text for a given x-coordinate in the bar.
     *
     * @param x Horizontal coordinate relative to widget.
     * @return Tooltip text or empty string if no segment applies.
     */
    QString tooltipForPosition(int x) const;
};

#endif // COCKATRICE_COLOR_BAR_H
