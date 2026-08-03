#include "abstract_settings_page.h"

#include "settings_search_model.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPair>
#include <QSpinBox>

/**
 * @brief Recursively collects all widgets within a layout
 * @param layout The layout to walk
 * @param widgets Output list of (widget, containing layout) pairs
 */
static void collectWidgets(QLayout *layout, QList<QPair<QWidget *, QLayout *>> &widgets)
{
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);
        if (!item) {
            continue;
        }
        if (QWidget *widget = item->widget()) {
            widgets.append({widget, layout});
        } else if (QLayout *subLayout = item->layout()) {
            collectWidgets(subLayout, widgets);
        }
    }
}

/**
 * @brief Rejects QLabels that are not setting names
 *
 * HTML link labels, path values, and excessively long labels are filtered out.
 */
static bool isValidSettingLabel(const QLabel *label)
{
    const QString &text = label->text();
    if (Qt::mightBeRichText(text)) {
        return false;
    }
    if (text.contains(QLatin1Char('/')) || text.contains(QLatin1Char('\\'))) {
        return false;
    }
    if (text.size() > 60) {
        return false;
    }
    return true;
}

/**
 * @brief Finds the control associated with a setting label
 *
 * Uses the explicit buddy if set, otherwise the widget in the cell (or slot)
 * immediately following the label within the same layout. Falls back to the
 * label itself when no obvious control is found.
 */
static QWidget *controlForLabel(QLabel *label, QLayout *containingLayout)
{
    if (QWidget *buddy = label->buddy()) {
        return buddy;
    }

    if (auto *grid = qobject_cast<QGridLayout *>(containingLayout)) {
        int index = grid->indexOf(label);
        if (index != -1) {
            int row = 0;
            int column = 0;
            int rowSpan = 1;
            int columnSpan = 1;
            grid->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
            if (QLayoutItem *next = grid->itemAtPosition(row, column + columnSpan)) {
                if (QWidget *nextWidget = next->widget()) {
                    return nextWidget;
                }
            }
        }
    } else {
        int index = containingLayout->indexOf(label);
        if (index != -1) {
            for (int i = index + 1; i < containingLayout->count(); ++i) {
                if (QLayoutItem *next = containingLayout->itemAt(i)) {
                    if (QWidget *nextWidget = next->widget()) {
                        return nextWidget;
                    }
                }
            }
        }
    }

    return label;
}

/**
 * @brief Builds the extended search text for an entry
 *
 * Combines the group title, label, and any extra searchable text derived from
 * the associated control (tooltip, placeholder, prefix/suffix, combo text).
 */
static QString buildFullSearchText(const QString &groupTitle, const QString &cleanLabel, QWidget *control)
{
    QStringList parts = {groupTitle, cleanLabel};
    if (control) {
        if (auto *lineEdit = qobject_cast<QLineEdit *>(control)) {
            parts.append(lineEdit->placeholderText());
        } else if (auto *spinBox = qobject_cast<QSpinBox *>(control)) {
            parts.append(spinBox->prefix());
            parts.append(spinBox->suffix());
        } else if (auto *comboBox = qobject_cast<QComboBox *>(control)) {
            parts.append(comboBox->currentText());
        }
        if (!control->toolTip().isEmpty()) {
            parts.append(control->toolTip());
        }
    }
    parts.removeAll(QString());
    return parts.join(QLatin1Char(' '));
}

QList<SettingsSearchEntry> AbstractSettingsPage::getSearchEntries()
{
    return autoDetectSearchEntries(this, -1);
}

QList<SettingsSearchEntry> AbstractSettingsPage::autoDetectSearchEntries(QWidget *page, int pageIndex)
{
    QList<SettingsSearchEntry> entries;

    const auto children = page->children();
    for (QObject *child : children) {
        auto *groupBox = qobject_cast<QGroupBox *>(child);
        if (!groupBox) {
            continue;
        }

        QString groupTitle = groupBox->title();
        if (groupTitle.isEmpty()) {
            continue;
        }

        QLayout *groupLayout = groupBox->layout();
        if (!groupLayout) {
            continue;
        }

        QList<QPair<QWidget *, QLayout *>> widgets;
        collectWidgets(groupLayout, widgets);

        for (const auto &pair : widgets) {
            QWidget *widget = pair.first;
            QString label;

            auto *checkBox = qobject_cast<QCheckBox *>(widget);
            if (checkBox) {
                label = checkBox->text();
            } else {
                auto *labelWidget = qobject_cast<QLabel *>(widget);
                if (!labelWidget || labelWidget->text().isEmpty() || !isValidSettingLabel(labelWidget)) {
                    continue;
                }
                label = labelWidget->text();
            }

            if (label.isEmpty()) {
                continue;
            }

            // Strip accelerator markers (&) for search
            QString cleanLabel = label;
            cleanLabel.remove(QLatin1Char('&'));

            QWidget *control = widget;
            if (auto *labelWidget = qobject_cast<QLabel *>(widget)) {
                control = controlForLabel(labelWidget, pair.second);
            }

            SettingsSearchEntry entry;
            entry.pageIndex = pageIndex;
            entry.groupTitle = groupTitle;
            entry.widgetLabel = cleanLabel;
            entry.widget = control;
            entry.fullSearchText = buildFullSearchText(groupTitle, cleanLabel, control);
            entries.append(entry);
        }
    }

    return entries;
}
