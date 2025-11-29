#include "visual_database_display_filter_save_load_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "visual_database_filter_display_widget.h"

#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <libcockatrice/filters/filter_tree.h>

VisualDatabaseDisplayFilterSaveLoadWidget::VisualDatabaseDisplayFilterSaveLoadWidget(QWidget *parent,
                                                                                     FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    setMinimumWidth(300);
    setMaximumHeight(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Input for filter filename
    filenameInput = new QLineEdit(this);
    layout->addWidget(filenameInput);

    // Save button
    saveButton = new QPushButton(this);
    layout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &VisualDatabaseDisplayFilterSaveLoadWidget::saveFilter);

    // File list container
    fileListWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(fileListWidget);

    refreshFilterList(); // Populate the file list on startup
    retranslateUi();
}

void VisualDatabaseDisplayFilterSaveLoadWidget::retranslateUi()
{
    saveButton->setText(tr("Save Filter"));
    saveButton->setToolTip(tr("Save all currently applied filters to a file"));
    filenameInput->setPlaceholderText(tr("Enter filename..."));
}

void VisualDatabaseDisplayFilterSaveLoadWidget::saveFilter()
{
    QString filename = filenameInput->text().trimmed();
    if (filename.isEmpty())
        return;

    QString filePath = SettingsCache::instance().getFiltersPath() + QDir::separator() + filename + ".json";

    // Serialize the filter model to JSON
    QJsonArray filtersArray;
    QList<const CardFilter *> cardFilters = filterModel->allFilters();

    for (const CardFilter *filter : cardFilters) {
        filtersArray.append(filter->toJson());
    }

    QJsonObject root;
    root["filters"] = filtersArray;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
    }

    filenameInput->clear();
    refreshFilterList(); // Update the file list
}

void VisualDatabaseDisplayFilterSaveLoadWidget::loadFilter(const QString &filename)
{
    QString filePath = SettingsCache::instance().getFiltersPath() + QDir::separator() + filename;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();
    if (!root.contains("filters") || !root["filters"].isArray())
        return;

    QJsonArray filtersArray = root["filters"].toArray();

    filterModel->clear();

    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);
    for (const QJsonValue &value : filtersArray) {
        if (value.isObject()) {
            QJsonObject filterObj = value.toObject();
            CardFilter *filter = CardFilter::fromJson(filterObj);
            if (filter) {
                filterModel->addFilter(filter);
            }
        }
    }
    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayFilterSaveLoadWidget::refreshFilterList()
{
    fileListWidget->clearLayout();
    // Clear existing widgets
    for (auto buttonPair : fileButtons) {
        buttonPair.first->deleteLater();
        buttonPair.second->deleteLater();
    }
    fileButtons.clear(); // Clear the list of buttons

    // Refresh the filter file list
    QDir dir(SettingsCache::instance().getFiltersPath());
    QStringList filterFiles = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name);

    // Loop through the filter files and create widgets for them
    for (const QString &filename : filterFiles) {
        bool alreadyAdded = false;

        // Check if the widget for this filter file already exists to avoid duplicates
        for (const auto &pair : fileButtons) {
            if (pair.first->text() == filename) {
                alreadyAdded = true;
                break;
            }
        }

        if (!alreadyAdded) {
            // Create a new custom widget for the filter
            FilterDisplayWidget *filterWidget = new FilterDisplayWidget(this, filename, filterModel);
            fileListWidget->addWidget(filterWidget);

            // Connect signals to handle loading and deletion
            connect(filterWidget, &FilterDisplayWidget::filterLoadRequested, this,
                    &VisualDatabaseDisplayFilterSaveLoadWidget::loadFilter);
            connect(filterWidget, &FilterDisplayWidget::filterDeleted, this,
                    &VisualDatabaseDisplayFilterSaveLoadWidget::refreshFilterList);
        }
    }
}
