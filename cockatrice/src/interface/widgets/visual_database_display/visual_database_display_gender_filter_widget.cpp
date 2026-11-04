#include "visual_database_display_gender_filter_widget.h"

#include "../../../filters/filter_tree_model.h"
#include "../general/layout_containers/flow_widget.h"
#include "libcockatrice/filters/filter_card.h"
#include "libcockatrice/filters/filter_tree.h"
#include "visual_database_display_filter_button.h"

#include <QList>
#include <QPair>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <qnamespace.h>

VisualDatabaseDisplayGenderFilterWidget::VisualDatabaseDisplayGenderFilterWidget(QWidget *parent,
                                                                                 FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    setObjectName("genderFilterWidget");
    setMinimumWidth(300);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    auto *flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    flowWidget->setMaximumHeight(300);
    layout->addWidget(flowWidget);

    // Authoritative source: continuum-modules.mse-include/genders/choice_names
    // The MSE "group horde" option is intentionally omitted.
    const QList<QPair<QString, QString>> genders = {
        {QStringLiteral("none"), tr("None")},
        {QStringLiteral("sex female"), tr("Female")},
        {QStringLiteral("sex male"), tr("Male")},
        {QStringLiteral("sex neuter"), tr("Neuter")},
        {QStringLiteral("sex hermaphrodite"), tr("Hermaphrodite")},
        {QStringLiteral("sex null"), tr("Sex Null")},
        {QStringLiteral("artificial artifice"), tr("Artifice")},
        {QStringLiteral("artificial masculine"), tr("Masculine (Artificial)")},
        {QStringLiteral("artificial feminine"), tr("Feminine (Artificial)")},
        {QStringLiteral("duos couple"), tr("Couple")},
        {QStringLiteral("duos brothers"), tr("Brothers")},
        {QStringLiteral("duos sisters"), tr("Sisters")},
        {QStringLiteral("duos siblings"), tr("Siblings")},
        {QStringLiteral("special not applicable"), tr("Not Applicable")},
        {QStringLiteral("custom gender one"), tr("Custom One")},
        {QStringLiteral("custom gender two"), tr("Custom Two")},
        {QStringLiteral("custom gender three"), tr("Custom Three")},
    };

    for (const auto &gender : genders) {
        auto *button = new QPushButton(gender.second, flowWidget);
        button->setObjectName(QStringLiteral("genderFilter_") + gender.first);
        button->setToolTip(gender.first);
        button->setCheckable(true);
        button->setStyleSheet(visualDatabaseDisplayFilterButtonStyle);
        flowWidget->addWidget(button);
        genderButtons.insert(gender.first, button);
        connect(button, &QPushButton::toggled, this, [this](bool) { updateGenderFilter(); });
    }

    auto *clearButton = new QPushButton(tr("Clear gender filter"), this);
    layout->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        for (auto *button : genderButtons) {
            const QSignalBlocker blocker(button);
            button->setChecked(false);
        }
        updateGenderFilter();
    });

    connect(filterModel, &FilterTreeModel::layoutChanged, this, [this]() { syncWithFilterModel(); });
    syncWithFilterModel();
}

void VisualDatabaseDisplayGenderFilterWidget::updateGenderFilter()
{
    // Selected genders are ORed together; other filter attributes still use AND.
    // No selections means there is no gender restriction.
    {
        const QSignalBlocker modelBlocker(filterModel);
        const QSignalBlocker treeBlocker(filterModel->filterTree());
        filterModel->clearFiltersOfType(CardFilter::AttrGender);

        for (auto it = genderButtons.cbegin(); it != genderButtons.cend(); ++it) {
            if (it.value()->isChecked()) {
                QString gender = it.key();
                filterModel->addFilter(new CardFilter(gender, CardFilter::TypeOr, CardFilter::AttrGender));
            }
        }
    }

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayGenderFilterWidget::syncWithFilterModel()
{
    QSet<QString> selected;
    const auto filters = filterModel->getFiltersOfType(CardFilter::AttrGender);
    for (const CardFilter *filter : filters) {
        if (filter->type() == CardFilter::TypeOr || filter->type() == CardFilter::TypeAnd) {
            selected.insert(filter->term());
        }
        delete filter; // FilterTreeModel::getFiltersOfType returns heap copies.
    }

    for (auto it = genderButtons.cbegin(); it != genderButtons.cend(); ++it) {
        const QSignalBlocker blocker(it.value());
        it.value()->setChecked(selected.contains(it.key()));
    }
}
