#ifndef ANIMATED_ITEM_H
#define ANIMATED_ITEM_H

/**
 * @file animated_item.h
 * @ingroup GameGraphics
 * @brief Interface for scene items driven by GameScene's shared animation timer.
 *
 * Items that want per-tick animation while a single QBasicTimer runs (instead of
 * owning their own QTimer) implement this interface and register with the scene
 * via GameScene::registerAnimationItem.
 */

class IAnimatedItem
{
public:
    virtual ~IAnimatedItem() = default;

    /**
     * @brief Advances the item's animation by one timer tick.
     * @return true while the animation is still running, false once it has finished.
     */
    virtual bool animationEvent() = 0;
};

#endif
