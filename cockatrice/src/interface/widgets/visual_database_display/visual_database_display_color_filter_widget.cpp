#include "visual_database_display_color_filter_widget.h"

#include "../cards/additional_info/mana_symbol_widget.h"
#include "filter_tree_model.h"
#include "libcockatrice/filters/filter_card.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QPersistentModelIndex>
#include <QSharedPointer>
#include <QSignalBlocker>
#include <QString>
#include <QVariant>
#include <QtGlobal>
#include <libcockatrice/filters/filter_tree.h>

/**
 * This widget provides a graphical control element for the CardFilter::Attr::AttrColor filters applied to the filter
 * model.
 * @param parent The Qt Widget that this widget will parent to
 * @param _filterModel The filter model that this widget will manipulate
 */
VisualDatabaseDisplayColorFilterWidget::VisualDatabaseDisplayColorFilterWidget(QWidget *parent,
                                                                               FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);

    QString fullColorIdentity = "WUBRG";
    for (const QChar &color : fullColorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(this, color, false, true);
        manaSymbol->setFixedWidth(25);

        layout->addWidget(manaSymbol);

        // Connect the color toggled signal
        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this,
                &VisualDatabaseDisplayColorFilterWidget::handleColorToggled);
    }

    modeComboBox = new QComboBox(this);
    layout->addWidget(modeComboBox);

    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDatabaseDisplayColorFilterWidget::updateFilterMode);

    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { syncDebouncer.call([this] { syncWithFilterModel(); }); });
    connect(filterModel, &FilterTreeModel::filtersLoaded, this,
            &VisualDatabaseDisplayColorFilterWidget::restoreModeFromLoadedFilters);

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void VisualDatabaseDisplayColorFilterWidget::retranslateUi()
{
    modeComboBox->blockSignals(true);
    modeComboBox->clear();

    modeComboBox->addItem(tr("Exact match"), QVariant::fromValue(FilterMode::ExactMatch));
    modeComboBox->addItem(tr("Includes"), QVariant::fromValue(FilterMode::Includes));
    modeComboBox->addItem(tr("Excludes"), QVariant::fromValue(FilterMode::Excludes));
    modeComboBox->addItem(tr("Include / Exclude"), QVariant::fromValue(FilterMode::IncludeExclude));
    modeComboBox->addItem(tr("Color identity"), QVariant::fromValue(FilterMode::ColorIdentity));

    updateModeToolTip();

    // Restore current mode
    const int index = modeComboBox->findData(QVariant::fromValue(currentMode));
    if (index >= 0) {
        modeComboBox->setCurrentIndex(index);
    }

    modeComboBox->blockSignals(false);
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar color, bool active)
{
    if (active) {
        addFilter(color);
    } else {
        removeFilter(color);
    }
}

static void removeFilters(FilterTreeModel *filterModel, QChar color, CardFilter::Attr attr)
{
    const auto filters = filterModel->getFiltersOfType(attr);
    for (const CardFilter *filter : filters) {
        if (filter->term().compare(QString(color), Qt::CaseInsensitive) == 0) {
            filterModel->removeFilter(filter);
        }
    }
    qDeleteAll(filters);
}

static void addColorFilter(FilterTreeModel *filterModel, QChar color, CardFilter::Type type, CardFilter::Attr attr)
{
    const CardFilter filter(color, type, attr);
    filterModel->addFilter(&filter);
}

static QString colorModeId(FilterMode mode)
{
    switch (mode) {
        case FilterMode::ExactMatch:
            return QStringLiteral("exact");
        case FilterMode::Includes:
            return QStringLiteral("includes");
        case FilterMode::Excludes:
            return QStringLiteral("excludes");
        case FilterMode::IncludeExclude:
            return QStringLiteral("include_exclude");
        case FilterMode::ColorIdentity:
            return QStringLiteral("identity");
    }
    return QStringLiteral("includes");
}

void VisualDatabaseDisplayColorFilterWidget::addFilter(QChar color)
{
    switch (currentMode) {
        case FilterMode::ExactMatch:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            addColorFilter(filterModel, color, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::Includes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            addColorFilter(filterModel, color, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::Excludes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            addColorFilter(filterModel, color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::IncludeExclude:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            addColorFilter(filterModel, color, CardFilter::Type::TypeOr, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::ColorIdentity:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColorIdentity);
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::removeFilter(QChar color)
{
    // Add exclusion filters if the mode demands it
    switch (currentMode) {
        case FilterMode::ExactMatch:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            addColorFilter(filterModel, color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::Includes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::Excludes:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::IncludeExclude:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColor);
            addColorFilter(filterModel, color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor);
            break;

        case FilterMode::ColorIdentity:
            removeFilters(filterModel, color, CardFilter::Attr::AttrColorIdentity);
            addColorFilter(filterModel, color, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColorIdentity);
            break;
    }
}

void VisualDatabaseDisplayColorFilterWidget::updateFilterMode()
{
    const QVariant data = modeComboBox->currentData();
    if (!data.isValid()) {
        return;
    }

    const auto previousMode = currentMode;
    currentMode = data.value<FilterMode>();
    updateModeToolTip();

    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);

    if (previousMode != FilterMode::ColorIdentity || currentMode != FilterMode::ColorIdentity) {
        filterModel->clearFiltersOfType(CardFilter::Attr::AttrColor);
    }
    if (previousMode == FilterMode::ColorIdentity || currentMode == FilterMode::ColorIdentity) {
        filterModel->clearFiltersOfType(CardFilter::Attr::AttrColorIdentity);
    }

    const QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        handleColorToggled(manaSymbolWidget->getSymbolChar(), manaSymbolWidget->isColorActive());
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
    emit filterModeChanged(currentMode);
}

void VisualDatabaseDisplayColorFilterWidget::updateModeToolTip()
{
    filterModel->setProperty("visualDatabaseColorMode", colorModeId(currentMode));
    if (currentMode == FilterMode::Excludes) {
        modeComboBox->setToolTip(tr("Cards containing none of the selected colors."));
    } else if (currentMode == FilterMode::ColorIdentity) {
        modeComboBox->setToolTip(tr("Cards whose color identity fits within the selected colors. "
                                    "With no colors selected, only colorless cards match."));
    } else {
        modeComboBox->setToolTip(tr("How selected and unselected colors are combined in the filter"));
    }
}

void VisualDatabaseDisplayColorFilterWidget::restoreModeFromLoadedFilters(const QString &savedMode)
{
    const auto identityFilters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColorIdentity);
    bool hasWidgetFilter = false;
    for (const CardFilter *filter : identityFilters) {
        const QString term = filter->term();
        if (filter->type() == CardFilter::Type::TypeAndNot && term.size() == 1 &&
            QStringLiteral("WUBRG").contains(term, Qt::CaseInsensitive)) {
            hasWidgetFilter = true;
            break;
        }
    }
    qDeleteAll(identityFilters);
    FilterMode restoredMode = FilterMode::Includes;
    if (savedMode == QLatin1String("exact")) {
        restoredMode = FilterMode::ExactMatch;
    } else if (savedMode == QLatin1String("includes")) {
        restoredMode = FilterMode::Includes;
    } else if (savedMode == QLatin1String("excludes")) {
        restoredMode = FilterMode::Excludes;
    } else if (savedMode == QLatin1String("include_exclude")) {
        restoredMode = FilterMode::IncludeExclude;
    } else if (savedMode == QLatin1String("identity")) {
        restoredMode = FilterMode::ColorIdentity;
    } else if (hasWidgetFilter) {
        restoredMode = FilterMode::ColorIdentity;
    } else {
        const auto colorFilters = filterModel->getFiltersOfType(CardFilter::Attr::AttrColor);
        bool hasAnd = false;
        bool hasOr = false;
        bool hasAndNot = false;
        for (const CardFilter *filter : colorFilters) {
            const QString term = filter->term();
            if (term.size() != 1 || !QStringLiteral("WUBRG").contains(term, Qt::CaseInsensitive)) {
                continue;
            }
            hasAnd |= filter->type() == CardFilter::Type::TypeAnd;
            hasOr |= filter->type() == CardFilter::Type::TypeOr;
            hasAndNot |= filter->type() == CardFilter::Type::TypeAndNot;
        }
        qDeleteAll(colorFilters);
        if (hasAnd) {
            restoredMode = FilterMode::ExactMatch;
        } else if (hasOr && hasAndNot) {
            restoredMode = FilterMode::IncludeExclude;
        } else if (hasAndNot) {
            restoredMode = FilterMode::Excludes;
        }
    }
    const bool modeChanged = currentMode != restoredMode;
    currentMode = restoredMode;
    const QSignalBlocker blocker(modeComboBox);
    modeComboBox->setCurrentIndex(modeComboBox->findData(QVariant::fromValue(currentMode)));
    updateModeToolTip();
    syncWithFilterModel();
    if (modeChanged) {
        emit filterModeChanged(currentMode);
    }
}

void VisualDatabaseDisplayColorFilterWidget::setManaSymbolActive(QChar color, bool active)
{
    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        if (manaSymbolWidget->getSymbolChar() == color) {
            manaSymbolWidget->setColorActive(active);
        }
    }
}

QList<QChar> VisualDatabaseDisplayColorFilterWidget::getActiveColors()
{
    QList<QChar> activeColors;
    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        if (manaSymbolWidget->isColorActive()) {
            activeColors.append(manaSymbolWidget->getSymbolChar());
        }
    }

    return activeColors;
}

void VisualDatabaseDisplayColorFilterWidget::syncWithFilterModel()
{
    QList<const CardFilter *> allColorFilters = filterModel->getFiltersOfType(
        currentMode == FilterMode::ColorIdentity ? CardFilter::Attr::AttrColorIdentity : CardFilter::Attr::AttrColor);

    QList<ManaSymbolWidget *> manaSymbolWidgets = findChildren<ManaSymbolWidget *>();

    for (ManaSymbolWidget *manaSymbolWidget : manaSymbolWidgets) {
        QChar symbol = manaSymbolWidget->getSymbolChar();
        bool found = currentMode == FilterMode::ColorIdentity;
        for (const CardFilter *filter : allColorFilters) {
            if (filter->term().compare(QString(symbol), Qt::CaseInsensitive) == 0) {
                switch (currentMode) {
                    case FilterMode::ExactMatch:
                        if (filter->type() == CardFilter::Type::TypeAnd) {
                            found = true;
                        }
                        break;
                    case FilterMode::Includes:
                    case FilterMode::IncludeExclude:
                        if (filter->type() == CardFilter::Type::TypeOr) {
                            found = true;
                        }
                        break;
                    case FilterMode::Excludes:
                        if (filter->type() == CardFilter::Type::TypeAndNot) {
                            found = true;
                        }
                        break;
                    case FilterMode::ColorIdentity:
                        if (filter->type() == CardFilter::Type::TypeAndNot) {
                            found = false;
                        }
                        break;
                }
                if ((currentMode == FilterMode::ColorIdentity && !found) ||
                    (currentMode != FilterMode::ColorIdentity && found)) {
                    break;
                }
            }
        }

        setManaSymbolActive(manaSymbolWidget->getSymbolChar(), found);
    }
    qDeleteAll(allColorFilters);
}
