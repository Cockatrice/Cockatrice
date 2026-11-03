#ifndef CONTINUUMATRICE_ESSENCE_FILTER_PICKER_H
#define CONTINUUMATRICE_ESSENCE_FILTER_PICKER_H

#include <QChar>
#include <QHash>
#include <QSet>
#include <QToolButton>

class QMenu;

/**
 * Compact, reusable multi-select Essence picker for the deck and card database
 * toolbars. The menu contains all fifteen Essence symbols in a 5x3 icon grid.
 * Clicking a symbol does not close the popup; programmatic synchronization
 * does not emit a user-selection signal.
 */
class EssenceFilterPicker final : public QToolButton
{
    Q_OBJECT

public:
    explicit EssenceFilterPicker(QWidget *parent = nullptr);

    [[nodiscard]] QSet<QChar> activeEssences() const;
    void setEssenceActive(QChar essence, bool active);
    void retranslateUi();

signals:
    void essenceToggled(QChar essence, bool active);

private:
    void updateSummary();

    QHash<QChar, QToolButton *> buttons;
    QMenu *popupMenu = nullptr;
};

#endif // CONTINUUMATRICE_ESSENCE_FILTER_PICKER_H
