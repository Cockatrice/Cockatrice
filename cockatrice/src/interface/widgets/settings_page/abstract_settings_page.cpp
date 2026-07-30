#include "abstract_settings_page.h"

#include "settings_search_model.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

QList<SettingsSearchEntry> AbstractSettingsPage::getSearchEntries()
{
    return autoDetectSearchEntries(this, -1);
}

void AbstractSettingsPage::resetToDefaults()
{
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

        for (int i = 0; i < groupLayout->count(); ++i) {
            QLayoutItem *item = groupLayout->itemAt(i);
            if (!item) {
                continue;
            }

            // Direct widget children (checkboxes, labels with buddies, etc.)
            if (QWidget *widget = item->widget()) {
                QString label;

                auto *checkBox = qobject_cast<QCheckBox *>(widget);
                if (checkBox) {
                    label = checkBox->text();
                } else {
                    auto *labelWidget = qobject_cast<QLabel *>(widget);
                    if (labelWidget && labelWidget->text().isEmpty()) {
                        continue;
                    } else if (labelWidget) {
                        label = labelWidget->text();
                    }
                }

                if (!label.isEmpty()) {
                    // Skip QLabels that are not setting names: HTML, path values, or excessively long
                    if (qobject_cast<QLabel *>(widget)) {
                        auto *labelWidget = static_cast<QLabel *>(widget);
                        if (labelWidget->textFormat() != Qt::PlainText) {
                            continue;
                        }
                        if (label.contains(QLatin1Char('/')) || label.contains(QLatin1Char('\\'))) {
                            continue;
                        }
                        if (label.size() > 60) {
                            continue;
                        }
                    }

                    // Strip accelerator markers (&) for search
                    QString cleanLabel = label;
                    cleanLabel.remove(QLatin1Char('&'));

                    SettingsSearchEntry entry;
                    entry.pageIndex = pageIndex;
                    entry.groupTitle = groupTitle;
                    entry.widgetLabel = cleanLabel;
                    entry.fullSearchText = QStringLiteral("%1 %2 %3").arg(groupTitle, cleanLabel, cleanLabel.toLower());
                    entry.widget = widget;
                    entries.append(entry);
                }
            }

            // Layout items (nested grids, hboxes, vboxes)
            if (item->layout()) {
                for (int j = 0; j < item->layout()->count(); ++j) {
                    QLayoutItem *nestedItem = item->layout()->itemAt(j);
                    if (!nestedItem || !nestedItem->widget()) {
                        continue;
                    }

                    QWidget *nestedWidget = nestedItem->widget();
                    QString label;

                    auto *checkBox = qobject_cast<QCheckBox *>(nestedWidget);
                    if (checkBox) {
                        label = checkBox->text();
                    } else {
                        auto *labelWidget = qobject_cast<QLabel *>(nestedWidget);
                        if (labelWidget && !labelWidget->text().isEmpty()) {
                            label = labelWidget->text();
                        }
                    }

                    if (!label.isEmpty()) {
                        // Skip QLabels that are not setting names: HTML, path values, or excessively long
                        if (qobject_cast<QLabel *>(nestedWidget)) {
                            auto *labelWidget = static_cast<QLabel *>(nestedWidget);
                            if (labelWidget->textFormat() != Qt::PlainText) {
                                continue;
                            }
                            if (label.contains(QLatin1Char('/')) || label.contains(QLatin1Char('\\'))) {
                                continue;
                            }
                            if (label.size() > 60) {
                                continue;
                            }
                        }

                        QString cleanLabel = label;
                        cleanLabel.remove(QLatin1Char('&'));

                        SettingsSearchEntry entry;
                        entry.pageIndex = pageIndex;
                        entry.groupTitle = groupTitle;
                        entry.widgetLabel = cleanLabel;
                        entry.fullSearchText =
                            QStringLiteral("%1 %2 %3").arg(groupTitle, cleanLabel, cleanLabel.toLower());
                        entry.widget = nestedWidget;
                        entries.append(entry);
                    }
                }
            }
        }
    }

    return entries;
}
