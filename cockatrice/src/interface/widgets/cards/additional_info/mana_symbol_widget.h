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
    bool isColorActive() const
    {
        return isActive;
    };
    QString getSymbol() const
    {
        return symbol;
    };
    QChar getSymbolChar() const
    {
        return symbol[0];
    };

    void loadManaIcon();

public slots:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void colorToggled(QChar symbol, bool isActive);

private:
    QString symbol;
    QPixmap manaIcon;
    bool isActive;
    bool mayBeToggled;
    QGraphicsOpacityEffect *opacityEffect;
};

#endif // MANA_SYMBOL_WIDGET_H
