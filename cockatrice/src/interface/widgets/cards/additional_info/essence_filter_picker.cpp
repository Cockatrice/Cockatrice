#include "essence_filter_picker.h"

#include "../../../pixel_map_generator.h"

#include <QFontMetrics>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSize>
#include <QSizePolicy>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>
#include <libcockatrice/card/essence_types.h>

EssenceFilterPicker::EssenceFilterPicker(QWidget *parent) : QToolButton(parent)
{
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonTextOnly); // Keep the summary visible even without a button icon.
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // Force a stable toolbar footprint independent of popup and font sizing.
    // Modes and all 16 Essence icons are contained inside the popup.
    setFixedWidth(qMax(156, fontMetrics().horizontalAdvance(tr("Essence: Any")) + 42));

    popupMenu = new QMenu(this);
    auto *panel = new QWidget(popupMenu);
    panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(10, 10, 10, 10);
    panelLayout->setSpacing(7);

    auto *heading = new QLabel(tr("Filter by Essence"), panel);
    panelLayout->addWidget(heading);

    auto *grid = new QGridLayout();
    grid->setSpacing(5);

    int index = 0;
    for (const QChar essence : Essence::symbols()) {
        const QString symbol(essence);
        auto *button = new QToolButton(panel);
        button->setCheckable(true);
        button->setAutoRaise(false);
        button->setFixedSize(42, 42);
        button->setIcon(QIcon(ManaSymbolPixmapGenerator::generatePixmap(symbol, QSize(32, 32))));
        button->setIconSize(QSize(30, 30));
        if (essence == QChar('N')) {
            button->setToolTip(tr("N — Colorless Essence"));
            button->setAccessibleName(tr("Colorless Essence N"));
        } else if (essence == QChar('J')) {
            button->setToolTip(tr("J — Jasper Essence"));
            button->setAccessibleName(tr("Jasper Essence J"));
        } else {
            button->setToolTip(tr("Essence %1").arg(symbol));
            button->setAccessibleName(tr("Essence %1").arg(symbol));
        }
        button->setProperty("essenceCode", symbol);
        connect(button, &QToolButton::toggled, this, [this, essence](bool checked) {
            updateSummary();
            emit essenceToggled(essence, checked);
        });

        // Sixteen Essences form a compact 4x4 grid.
        grid->addWidget(button, index / 4, index % 4);
        buttons.insert(essence, button);
        ++index;
    }
    panelLayout->addLayout(grid);

    auto *clearButton = new QPushButton(tr("Clear Essence selection"), panel);
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        // Clear as ONE model operation, so Exact Match and Include/Exclude
        // don't retain stale "not this Essence" predicates.
        for (const QChar essence : Essence::symbols()) {
            if (auto *button = buttons.value(essence); button && button->isChecked()) {
                const QSignalBlocker blocker(button);
                button->setChecked(false);
            }
        }
        updateSummary();
        emit selectionCleared();
    });
    panelLayout->addWidget(clearButton);

    auto *panelAction = new QWidgetAction(popupMenu);
    panelAction->setDefaultWidget(panel);
    popupMenu->addAction(panelAction);
    setMenu(popupMenu);

    retranslateUi();
}

void EssenceFilterPicker::addPopupControl(QWidget *control)
{
    if (!control || !panelLayout) {
        return;
    }

    // Insert above the Clear button, keeping the entire configuration in one
    // popup rather than competing for horizontal space in the main toolbar.
    panelLayout->insertWidget(panelLayout->count() - 1, control);
}

QSet<QChar> EssenceFilterPicker::activeEssences() const
{
    QSet<QChar> active;
    for (auto it = buttons.cbegin(); it != buttons.cend(); ++it) {
        if (it.value()->isChecked()) {
            active.insert(it.key());
        }
    }
    return active;
}

void EssenceFilterPicker::setEssenceActive(QChar essence, bool active)
{
    auto *button = buttons.value(essence);
    if (!button || button->isChecked() == active) {
        return;
    }

    // Restoring external filter state must not rewrite the filter model.
    const QSignalBlocker blocker(button);
    button->setChecked(active);
    updateSummary();
}

void EssenceFilterPicker::retranslateUi()
{
    updateSummary();
}

void EssenceFilterPicker::updateSummary()
{
    QStringList selected;
    const QSet<QChar> active = activeEssences();
    for (const QChar essence : Essence::symbols()) {
        if (active.contains(essence)) {
            selected.append(QString(essence));
        }
    }

    if (selected.isEmpty()) {
        setText(tr("Essence: Any"));
        setToolTip(tr("Filter by Essence (none selected)"));
    } else if (selected.size() == 1) {
        setText(tr("Essence: %1").arg(selected.first()));
        setToolTip(tr("Selected Essence: %1").arg(selected.first()));
    } else {
        // Keep the toolbar label short; the tooltip lists the exact types.
        setText(tr("Essence (%1)").arg(selected.size()));
        setToolTip(tr("Selected Essence: %1").arg(selected.join(QStringLiteral(", "))));
    }
}
