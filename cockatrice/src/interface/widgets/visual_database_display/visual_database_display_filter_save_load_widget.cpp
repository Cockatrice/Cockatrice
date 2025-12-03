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

    // Filter search input
    searchInput = new QLineEdit(this);
    layout->addWidget(searchInput);

    connect(searchInput, &QLineEdit::textChanged, this, &VisualDatabaseDisplayFilterSaveLoadWidget::applySearchFilter);

    // File list container
    fileListWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(fileListWidget);

    // Input for filter filename
    filenameInput = new QLineEdit(this);
    layout->addWidget(filenameInput);

    // Save button
    saveButton = new QPushButton(this);
    layout->addWidget(saveButton);
    // Disable save if empty
    saveButton->setEnabled(false);
    connect(filenameInput, &QLineEdit::textChanged, this,
            [this](const QString &text) { saveButton->setEnabled(!text.trimmed().isEmpty()); });

    connect(saveButton, &QPushButton::clicked, this, &VisualDatabaseDisplayFilterSaveLoadWidget::saveFilter);

    refreshFilterList(); // Populate the file list on startup
    retranslateUi();
}

void VisualDatabaseDisplayFilterSaveLoadWidget::retranslateUi()
{
    searchInput->setPlaceholderText(tr("Search filter..."));
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

void VisualDatabaseDisplayFilterSaveLoadWidget::applySearchFilter(const QString &text)
{
    fileListWidget->clearLayout();

    QString filter = text.trimmed();
    QStringList filtered = allFilterFiles;

    if (!filter.isEmpty()) {
        filtered = filtered.filter(QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption));
    }

    for (const QString &filename : filtered) {
        FilterDisplayWidget *filterWidget = new FilterDisplayWidget(this, filename, filterModel);
        fileListWidget->addWidget(filterWidget);

        connect(filterWidget, &FilterDisplayWidget::filterLoadRequested, this,
                &VisualDatabaseDisplayFilterSaveLoadWidget::loadFilter);

        connect(filterWidget, &FilterDisplayWidget::filterDeleted, this,
                &VisualDatabaseDisplayFilterSaveLoadWidget::refreshFilterList);
    }
}

void VisualDatabaseDisplayFilterSaveLoadWidget::refreshFilterList()
{
    fileListWidget->clearLayout();
    fileButtons.clear();

    QDir dir(SettingsCache::instance().getFiltersPath());
    allFilterFiles = dir.entryList({"*.json"}, QDir::Files, QDir::Name);

    applySearchFilter(searchInput->text());
}
