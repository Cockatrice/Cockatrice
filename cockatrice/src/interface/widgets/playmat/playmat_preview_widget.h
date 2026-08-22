#ifndef COCKATRICE_PLAYMAT_PREVIEW_WIDGET_H
#define COCKATRICE_PLAYMAT_PREVIEW_WIDGET_H

#include <QPixmap>
#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>

/**
 * @brief Preview widget that shows how a playmat card art will appear
 *        across the combined table + stack play area.
 *
 * Renders a miniature mockup with the card art applied using the
 * given PlaymatParams, including faint zone divider lines.
 */
class PlaymatPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaymatPreviewWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    void setParams(const PlaymatParams &params);
    void setAttribution(const QString &attribution);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap sourcePixmap;
    PlaymatParams params;
    QString attributionText;
};

#endif // COCKATRICE_PLAYMAT_PREVIEW_WIDGET_H
