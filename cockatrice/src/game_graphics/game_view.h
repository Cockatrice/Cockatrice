/**
 * @file game_view.h
 * @ingroup GameGraphics
 */
//! \todo Document this file.

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include "../game/selection_subtype_tally.h"

#include <QGraphicsView>

class GameScene;
class QGridLayout;
class QLabel;
class QRubberBand;

class GameView : public QGraphicsView
{
    Q_OBJECT
private:
    QAction *aCloseMostRecentZoneView;
    QRubberBand *rubberBand;
    QLabel *dragCountLabel;
    QLabel *totalCountLabel;
    QWidget *subtypeTallyContainer;
    QGridLayout *subtypeTallyLayout;
    QPointF selectionOrigin;
    QList<SubtypeEntry> cachedSubtypeEntries; ///< Cached entries to avoid redundant rebuilds

    QSize rebuildSubtypeLabels(const QList<SubtypeEntry> &entries);
    void clearSubtypeLabels();

protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void startRubberBand(const QPointF &selectionOrigin);
    void resizeRubberBand(const QPointF &cursorPoint, int selectedCount);
    void stopRubberBand();
    void refreshShortcuts();
    void updateTotalSelectionCount(const QSize &viewSize = QSize());
    void setFocusDisabled(bool disabled);
public slots:
    void updateSceneRect(const QRectF &rect);

public:
    explicit GameView(GameScene *scene, QWidget *parent = nullptr);
};

#endif
