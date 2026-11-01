#include "visual_database_display_filter_save_load_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../general/layout_containers/flow_widget.h"
#include "filter_tree_model.h"
#include "visual_database_filter_display_widget.h"

#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QVBoxLayout>
#include <QVariant>
#include <libcockatrice/filters/filter_preset.h>
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

    const FilterPreset preset{filterModel->snapshot(), filterModel->property("visualDatabaseColorMode").toString()};
    const QJsonObject root = preset.toJson();

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

    const auto preset = FilterPreset::fromJson(doc.object());
    if (!preset) {
        showFilterError(this, tr("Load Failed"),
                        tr("Filter '%1' contains an unsupported filter. The current filters were kept.").arg(filename));
        return;
    }
    QPointer<FilterTreeModel> model(filterModel);
    model->replaceSnapshot(preset->snapshot);
    // Reset/changed observers can destroy the view and its model synchronously.
    // The accepted document owns this metadata, so notify only a surviving model.
    if (model) {
        emit model->filtersLoaded(preset->colorMode);
    }
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
