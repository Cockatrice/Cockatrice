#include "essence_filter_picker.h"

#include "../../../pixel_map_generator.h"

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
    setMinimumWidth(130);

    popupMenu = new QMenu(this);
    auto *panel = new QWidget(popupMenu);
    auto *vertical = new QVBoxLayout(panel);
    vertical->setContentsMargins(10, 10, 10, 10);
    vertical->setSpacing(7);

    auto *heading = new QLabel(tr("Filter by Essence"), panel);
    vertical->addWidget(heading);

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
        button->setToolTip(essence == QChar('N') ? tr("N — Colorless Essence")
                                               : tr("Essence %1").arg(symbol));
        button->setAccessibleName(essence == QChar('N') ? tr("Colorless Essence N")
                                                       : tr("Essence %1").arg(symbol));
        button->setProperty("essenceCode", symbol);
        connect(button, &QToolButton::toggled, this, [this, essence](bool checked) {
            updateSummary();
            emit essenceToggled(essence, checked);
        });

        grid->addWidget(button, index / 5, index % 5);
        buttons.insert(essence, button);
        ++index;
    }
    vertical->addLayout(grid);

    auto *clearButton = new QPushButton(tr("Clear Essence selection"), panel);
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        // The popup remains open while changing several Essence types.
        // Emit only for entries that actually changed.
        for (const QChar essence : Essence::symbols()) {
            if (auto *button = buttons.value(essence); button && button->isChecked()) {
                button->setChecked(false);
            }
        }
    });
    vertical->addWidget(clearButton);

    auto *panelAction = new QWidgetAction(popupMenu);
    panelAction->setDefaultWidget(panel);
    popupMenu->addAction(panelAction);
    setMenu(popupMenu);

    retranslateUi();
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
    } else if (selected.size() <= 3) {
        setText(tr("Essence: %1").arg(selected.join(QStringLiteral(", "))));
        setToolTip(tr("Selected Essence: %1").arg(selected.join(QStringLiteral(", "))));
    } else {
        setText(tr("Essence (%1)").arg(selected.size()));
        setToolTip(tr("Selected Essence: %1").arg(selected.join(QStringLiteral(", "))));
    }
}
