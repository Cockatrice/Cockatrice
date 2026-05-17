#include "palette_grid_widget.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QMetaEnum>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

static QList<QPalette::ColorRole> allRoles()
{
    QList<QPalette::ColorRole> roles;
    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        auto r = static_cast<QPalette::ColorRole>(i);
        if (r != QPalette::NoRole) {
            roles << r;
        }
    }
    return roles;
}

static const QList<QPalette::ColorGroup> ALL_GROUPS = {QPalette::Active, QPalette::Disabled, QPalette::Inactive};

static const QMap<QPalette::ColorRole, const char *> ROLE_DESCRIPTIONS = {
    {QPalette::Window, QT_TR_NOOP("Main window / dialog background")},
    {QPalette::WindowText, QT_TR_NOOP("Text drawn on Window")},
    {QPalette::Base, QT_TR_NOOP("Background for text input widgets")},
    {QPalette::Text, QT_TR_NOOP("Text in input widgets")},
    {QPalette::Button, QT_TR_NOOP("Button background")},
    {QPalette::ButtonText, QT_TR_NOOP("Button label text")},
    {QPalette::BrightText, QT_TR_NOOP("High-contrast text (e.g. checked items)")},
    {QPalette::Highlight, QT_TR_NOOP("Selection / focus highlight")},
    {QPalette::HighlightedText, QT_TR_NOOP("Text on top of Highlight")},
    {QPalette::Link, QT_TR_NOOP("Unvisited hyperlink")},
    {QPalette::LinkVisited, QT_TR_NOOP("Visited hyperlink")},
    {QPalette::AlternateBase, QT_TR_NOOP("Alternating row background in views")},
    {QPalette::ToolTipBase, QT_TR_NOOP("Tooltip background")},
    {QPalette::ToolTipText, QT_TR_NOOP("Tooltip text")},
    {QPalette::PlaceholderText, QT_TR_NOOP("Placeholder / hint text in inputs")},
    {QPalette::Light, QT_TR_NOOP("Lighter than Button (3-D highlight)")},
    {QPalette::Midlight, QT_TR_NOOP("Between Button and Light")},
    {QPalette::Mid, QT_TR_NOOP("Between Button and Dark")},
    {QPalette::Dark, QT_TR_NOOP("Darker than Button (3-D shadow)")},
    {QPalette::Shadow, QT_TR_NOOP("Very dark shadow colour")},
};

PaletteGridWidget::PaletteGridWidget(QWidget *parent) : QWidget(parent)
{
    scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    gridHost = new QWidget;
    buildGrid(gridHost);
    refreshChromePalettes();
    scroll->setWidget(gridHost);

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);
}

void PaletteGridWidget::buildGrid(QWidget *host)
{
    QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    auto *grid = new QGridLayout(host);
    grid->setSpacing(3);
    grid->setContentsMargins(12, 8, 12, 8);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);
    grid->setColumnStretch(2, 0);
    grid->setColumnStretch(3, 0);

    // Column headers
    const QStringList groupHeaders = {tr("Active"), tr("Disabled"), tr("Inactive")};
    const QStringList groupTips = {
        tr("Normal interactive state"),
        tr("Widget is disabled / not interactive"),
        tr("Window is in background / unfocused"),
    };
    for (int col = 0; col < 3; ++col) {
        auto *label = new QLabel(groupHeaders[col], host);
        label->setAlignment(Qt::AlignCenter);
        label->setToolTip(groupTips[col]);
        QFont f = label->font();
        f.setBold(true);
        label->setFont(f);
        label->setAutoFillBackground(true);
        label->setContentsMargins(4, 4, 4, 4);
        grid->addWidget(label, 0, col + 1);
        headerLabels.push_back(label);
    }

    // Role rows
    const auto roles = allRoles();
    for (int row = 0; row < roles.size(); ++row) {
        auto role = roles[row];
        const char *name = roleEnum.valueToKey(role);

        // Alternating row shade
        if (row % 2 == 0) {
            for (int col = 0; col < 4; ++col) {
                auto *shade = new QWidget(host);
                shade->setAutoFillBackground(true);
                grid->addWidget(shade, row + 1, col);
                rowShadeWidgets.push_back(shade);
            }
        }

        auto *label = new QLabel(QString(name), host);
        label->setToolTip(ROLE_DESCRIPTIONS.value(role, {}));
        label->setContentsMargins(4, 2, 8, 2);
        grid->addWidget(label, row + 1, 0);

        for (int col = 0; col < 3; ++col) {
            auto group = ALL_GROUPS[col];
            auto *btn = new ColorButton(host);
            colorButtons[group][role] = btn;
            grid->addWidget(btn, row + 1, col + 1, Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
}

void PaletteGridWidget::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        QTimer::singleShot(0, this, &PaletteGridWidget::refreshChromePalettes);
    }

    QWidget::changeEvent(e);
}

void PaletteGridWidget::refreshChromePalettes()
{
    const QPalette base = qApp->palette();
    const QColor alt = base.color(QPalette::AlternateBase);

    // Header labels
    for (auto *label : headerLabels) {
        QPalette lp = label->palette();
        lp.setColor(QPalette::Window, alt);
        label->setPalette(lp);
        label->update();
    }

    // Alternating row backgrounds
    for (auto *shade : rowShadeWidgets) {
        QPalette sp = shade->palette();
        sp.setColor(QPalette::Window, alt);
        shade->setPalette(sp);
        shade->update();
    }
}

void PaletteGridWidget::loadPalette(const PaletteConfig &cfg)
{
    for (auto group : ALL_GROUPS) {
        for (auto role : allRoles()) {
            QColor color = cfg.colors.value(group).value(role);
            if (!color.isValid()) {
                color = qApp->palette().color(group, role);
            }
            colorButtons[group][role]->setColor(color);
        }
    }
}

PaletteConfig PaletteGridWidget::currentPaletteConfig() const
{
    PaletteConfig cfg;
    for (auto group : ALL_GROUPS) {
        for (auto role : allRoles()) {
            cfg.colors[group][role] = colorButtons[group][role]->getColor();
        }
    }
    return cfg;
}