#include "visual_database_display_filter_save_load_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../general/layout_containers/flow_widget.h"
#include "filter_tree_model.h"
#include "libcockatrice/filters/filter_card.h"
#include "visual_database_filter_display_widget.h"

#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QPointer>
#include <QVariant>
#include <QRegularExpression>
#include <QSaveFile>
#include <QVBoxLayout>
#include <QtAlgorithms>
#include <memory>
#include <vector>
#include <libcockatrice/filters/filter_tree.h>
#include <libcockatrice/settings/paths_settings.h>
#include <qnamespace.h>

namespace
{
bool isFilterFilename(const QString &filename)
{
    return !filename.isEmpty() && filename != "." && filename != ".." && !filename.contains('/') &&
           !filename.contains('\\');
}

void showFilterError(QWidget *parent, const QString &title, const QString &message)
{
    // A parent can disappear while an error is displayed. A heap-owned, async
    // message box does not leave a stack dialog or a suspended widget method.
    auto *dialog = new QMessageBox(QMessageBox::Warning, title, message, QMessageBox::Ok, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->open();
}
} // namespace

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
    if (filename.isEmpty()) {
        return;
    }
    if (!isFilterFilename(filename)) {
        showFilterError(this, tr("Save Failed"), tr("Enter a filename without folder separators."));
        return;
    }

    QString filePath = SettingsCache::instance().paths().getFiltersPath() + QDir::separator() + filename + ".json";

    // Serialize the filter model to JSON
    QJsonArray filtersArray;
    QList<const CardFilter *> cardFilters = filterModel->allFilters();

    for (const CardFilter *filter : cardFilters) {
        filtersArray.append(filter->toJson());
    }
    qDeleteAll(cardFilters);

    QJsonObject root;
    root["filters"] = filtersArray;
    const QVariant colorMode = filterModel->property("visualDatabaseColorMode");
    if (colorMode.isValid()) {
        root["colorMode"] = QJsonValue::fromVariant(colorMode);
    }

    QSaveFile file(filePath);
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Indented);
    // Check the buffered flush explicitly: Qt 6.4 can otherwise report a
    // successful commit after a short write while closing the device.
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.flush() || !file.commit()) {
        showFilterError(this, tr("Save Failed"),
                        tr("Could not save filter '%1': %2").arg(filename, file.errorString()));
        return;
    }

    filenameInput->clear();
    refreshFilterList(); // Update the file list
}

void VisualDatabaseDisplayFilterSaveLoadWidget::loadFilter(const QString &filename)
{
    if (!isFilterFilename(filename)) {
        showFilterError(this, tr("Load Failed"), tr("Choose a filter from the saved filter list."));
        return;
    }
    QString filePath = SettingsCache::instance().paths().getFiltersPath() + QDir::separator() + filename;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        showFilterError(this, tr("Load Failed"),
                        tr("Could not read filter '%1': %2").arg(filename, file.errorString()));
        return;
    }

    QByteArray jsonData = file.readAll();
    if (file.error() != QFile::NoError) {
        showFilterError(this, tr("Load Failed"),
                        tr("Could not read filter '%1': %2").arg(filename, file.errorString()));
        return;
    }
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        showFilterError(this, tr("Load Failed"),
                        tr("Filter '%1' is not a valid filter file. The current filters were kept.").arg(filename));
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("filters") || !root["filters"].isArray()) {
        showFilterError(this, tr("Load Failed"),
                        tr("Filter '%1' is not a valid filter file. The current filters were kept.").arg(filename));
        return;
    }

    QJsonArray filtersArray = root["filters"].toArray();

    std::vector<std::unique_ptr<CardFilter>> parsedFilters;
    QList<const CardFilter *> filters;
    for (const QJsonValue &value : filtersArray) {
        const QJsonObject filterObj = value.toObject();
        std::unique_ptr<CardFilter> filter;
        if (value.isObject() && filterObj["term"].isString() && filterObj["type"].isString() &&
            filterObj["attr"].isString()) {
            filter.reset(CardFilter::fromJson(filterObj));
        }
        if (!filter || filter->type() < 0 || filter->type() >= CardFilter::TypeEnd || filter->attr() < 0 ||
            filter->attr() >= CardFilter::AttrEnd) {
            showFilterError(this, tr("Load Failed"),
                            tr("Filter '%1' contains an unsupported filter. The current filters were kept.").arg(filename));
            return;
        }
        filters.append(filter.get());
        parsedFilters.push_back(std::move(filter));
    }
    const QString colorMode = root.value("colorMode").toString();
    QPointer<FilterTreeModel> model(filterModel);
    model->replaceFilters(filters);
    // Reset/changed observers can destroy the view and its model synchronously.
    // The accepted document owns this metadata, so notify only a surviving model.
    if (model) emit model->filtersLoaded(colorMode);
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

    QDir dir(SettingsCache::instance().paths().getFiltersPath());
    allFilterFiles = dir.entryList({"*.json"}, QDir::Files, QDir::Name);

    applySearchFilter(searchInput->text());
}
