#include "visual_database_display_filter_save_load_widget.h"

#include "../../../../game/filters/filter_tree.h"

#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>

VisualDatabaseDisplayFilterSaveLoadWidget::VisualDatabaseDisplayFilterSaveLoadWidget(QWidget *parent,
                                                                                     FilterTreeModel *_filterModel,
                                                                                     const QString &savePath)
    : QWidget(parent), filterModel(_filterModel), saveDirectory(savePath)
{
    setMinimumWidth(300);
    setMaximumHeight(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Input for filter filename
    filenameInput = new QLineEdit(this);
    filenameInput->setPlaceholderText(tr("Enter filename..."));
    layout->addWidget(filenameInput);

    // Save button
    saveButton = new QPushButton(tr("Save Filter"), this);
    layout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &VisualDatabaseDisplayFilterSaveLoadWidget::saveFilter);

    // File list container
    fileListWidget = new FlowWidget(this, Qt::Vertical, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(fileListWidget);

    refreshFilterList(); // Populate the file list on startup
}

void VisualDatabaseDisplayFilterSaveLoadWidget::saveFilter()
{
    QString filename = filenameInput->text().trimmed();
    if (filename.isEmpty())
        return;

    QString filePath = saveDirectory + QDir::separator() + filename + ".json";

    // Serialize the filter model to JSON
    QJsonArray filtersArray;
    QList<const CardFilter *> cardFilters = filterModel->allFilters();

    for (const CardFilter *filter : cardFilters) {
        filtersArray.append(filter->toJson()); // Assuming toJson() returns a QJsonObject
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
    QString filePath = saveDirectory + "/" + filename;

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
            if (filter)
                filterModel->addFilter(filter);
        }
    }
    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    filterModel->filterTree()->nodeAt(0)->nodeChanged();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayFilterSaveLoadWidget::refreshFilterList()
{
    // Clear existing file buttons
    for (auto button : fileButtons) {
        delete button;
    }
    fileButtons.clear();

    QDir dir(saveDirectory);
    QStringList filterFiles = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Time);

    for (const QString &filename : filterFiles) {
        auto *button = new QPushButton(filename, fileListWidget);
        fileListWidget->addWidget(button);
        fileButtons[filename] = button;

        connect(button, &QPushButton::clicked, this, [this, filename]() { loadFilter(filename); });
    }
}
