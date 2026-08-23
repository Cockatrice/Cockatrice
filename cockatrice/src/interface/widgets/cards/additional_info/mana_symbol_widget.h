/**
 * @file mana_symbol_widget.h
 * @ingroup CardExtraInfoWidgets
 */
//! \todo Document this file.

#ifndef MANA_SYMBOL_WIDGET_H
#define MANA_SYMBOL_WIDGET_H

#include <QGraphicsOpacityEffect>
#include <QLabel>

class ManaSymbolWidget : public QLabel
{

    Q_OBJECT

public:
    ManaSymbolWidget(QWidget *parent, QString symbol, bool isActive = true, bool mayBeToggled = false);
    void toggleSymbol();
    void setColorActive(bool active);
    void updateOpacity();
    [[nodiscard]] bool isColorActive() const
    {
        return isActive;
    }
    [[nodiscard]] QString getSymbol() const
    {
        return symbol;
    }
    [[nodiscard]] QChar getSymbolChar() const
    {
        return symbol[0];
    }

    /**
     * @brief Returns a smooth-scaled rendering of the given mana symbol icon.
     *
     * Results are shared between all instances via a process-wide cache keyed by symbol
     * and size, so scaling work is done once per distinct combination instead of once per
     * widget creation or resize.
     */
    static QPixmap getCachedScaledIcon(const QString &symbol, const QSize &size);

public slots:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void colorToggled(QChar symbol, bool isActive);

private:
    QString symbol;
    bool isActive;
    bool mayBeToggled;
    QGraphicsOpacityEffect *opacityEffect;
};

#endif // MANA_SYMBOL_WIDGET_H
