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
#include <QMap>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QVBoxLayout>
#include <QVariant>
#include <libcockatrice/filters/filter_tree.h>
#include <libcockatrice/settings/paths_settings.h>
#include <memory>
#include <optional>
#include <qnamespace.h>
#include <utility>

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

QJsonObject serializeSnapshot(const FilterTreeModel::Snapshot &snapshot)
{
    QJsonArray filters;
    QJsonArray groups;
    for (const auto &attribute : snapshot) {
        const CardFilter attrIdentity(QString(), CardFilter::TypeAnd, attribute.attr);
        QJsonArray types;
        for (const auto &group : attribute.types) {
            const CardFilter typeIdentity(QString(), group.type, attribute.attr);
            types.append(QJsonObject{{"typeId", typeIdentity.toJson()["typeId"]}, {"enabled", group.enabled}});
            for (const auto &term : group.terms) {
                const CardFilter filter(term.term, group.type, attribute.attr);
                QJsonObject object = filter.toJson();
                object["enabled"] = term.enabled;
                filters.append(object);
            }
        }
        groups.append(
            QJsonObject{{"attrId", attrIdentity.toJson()["attrId"]}, {"enabled", attribute.enabled}, {"types", types}});
    }
    // Keep the original flat array and display fields for legacy readers. Group
    // metadata also represents empty groups, which have no entry in that array.
    return QJsonObject{{"filters", filters}, {"filterStateVersion", 1}, {"filterGroups", groups}};
}

std::optional<FilterTreeModel::Snapshot> parseSnapshot(const QJsonObject &root)
{
    if (!root.value("filters").isArray()) {
        return std::nullopt;
    }
    FilterTreeModel::Snapshot snapshot;
    QMap<CardFilter::Attr, qsizetype> attributes;
    QMap<QPair<CardFilter::Attr, CardFilter::Type>, qsizetype> types;
    const bool hasGroups = root.contains("filterGroups") || root.contains("filterStateVersion");
    if (hasGroups) {
        if (!root.value("filterStateVersion").isDouble() || root.value("filterStateVersion").toDouble() != 1 ||
            !root.value("filterGroups").isArray()) {
            return std::nullopt;
        }
        for (const auto &value : root.value("filterGroups").toArray()) {
            const QJsonObject group = value.toObject();
            if (!value.isObject() || !group.value("enabled").isBool() || !group.value("types").isArray()) {
                return std::nullopt;
            }
            std::unique_ptr<CardFilter> identity(
                CardFilter::fromJson(QJsonObject{{"term", ""}, {"typeId", "and"}, {"attrId", group.value("attrId")}}));
            if (!identity || attributes.contains(identity->attr())) {
                return std::nullopt;
            }
            const auto attr = identity->attr();
            attributes.insert(attr, snapshot.size());
            FilterTreeModel::AttributeSnapshot attribute{attr, group.value("enabled").toBool(), {}};
            for (const auto &typeValue : group.value("types").toArray()) {
                const QJsonObject type = typeValue.toObject();
                if (!typeValue.isObject() || !type.value("enabled").isBool()) {
                    return std::nullopt;
                }
                identity.reset(CardFilter::fromJson(
                    QJsonObject{{"term", ""}, {"typeId", type.value("typeId")}, {"attrId", group.value("attrId")}}));
                if (!identity) {
                    return std::nullopt;
                }
                const auto key = qMakePair(attr, identity->type());
                if (types.contains(key)) {
                    return std::nullopt;
                }
                types.insert(key, attribute.types.size());
                attribute.types.append({identity->type(), type.value("enabled").toBool(), {}});
            }
            snapshot.append(std::move(attribute));
        }
    }

    for (const auto &value : root.value("filters").toArray()) {
        const QJsonObject object = value.toObject();
        if (!value.isObject() || !object.value("term").isString() || !object.value("type").isString() ||
            !object.value("attr").isString() ||
            ((hasGroups || object.contains("enabled")) && !object.value("enabled").isBool())) {
            return std::nullopt;
        }
        std::unique_ptr<CardFilter> filter(CardFilter::fromJson(object));
        if (!filter) {
            return std::nullopt;
        }
        const auto attr = filter->attr();
        const auto key = qMakePair(attr, filter->type());
        if (hasGroups && (!attributes.contains(attr) || !types.contains(key))) {
            return std::nullopt;
        }
        // Files predating enabled-state metadata retain their all-enabled behavior.
        if (!attributes.contains(attr)) {
            attributes.insert(attr, snapshot.size());
            snapshot.append({attr, true, {}});
        }
        auto &attribute = snapshot[attributes.value(attr)];
        if (!types.contains(key)) {
            types.insert(key, attribute.types.size());
            attribute.types.append({filter->type(), true, {}});
        }
        auto &terms = attribute.types[types.value(key)].terms;
        const bool enabled = object.value("enabled").toBool(true);
        bool duplicate = false;
        for (const auto &term : terms) {
            if (term.term == filter->term()) {
                if (term.enabled != enabled) {
                    return std::nullopt;
                }
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            terms.append({filter->term(), enabled});
        }
    }
    return snapshot;
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

    QJsonObject root = serializeSnapshot(filterModel->snapshot());
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

    const auto snapshot = parseSnapshot(root);
    if (!snapshot) {
        showFilterError(this, tr("Load Failed"),
                        tr("Filter '%1' contains an unsupported filter. The current filters were kept.").arg(filename));
        return;
    }
    const QString colorMode = root.value("colorMode").toString();
    QPointer<FilterTreeModel> model(filterModel);
    model->replaceSnapshot(*snapshot);
    // Reset/changed observers can destroy the view and its model synchronously.
    // The accepted document owns this metadata, so notify only a surviving model.
    if (model) {
        emit model->filtersLoaded(colorMode);
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
