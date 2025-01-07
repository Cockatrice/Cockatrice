#include "visual_deck_storage_sort_widget.h"

#include "../../../../settings/cache_settings.h"

/**
 * @brief Constructs a PrintingSelectorCardSortWidget for searching cards by set name or set code.
 *
 * This widget provides a search bar that allows users to search for cards by either their set name
 * or set code. It uses a debounced timer to trigger the search action after the user stops typing.
 *
 * @param parent The parent PrintingSelector widget that will handle the search results.
 */
VisualDeckStorageSortWidget::VisualDeckStorageSortWidget(VisualDeckStorageWidget *parent)
    : parent(parent), sortOrder(Alphabetical)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    // ComboBox for sorting options
    sortComboBox = new QComboBox(this);
    sortComboBox->addItem("Sort Alphabetically (Filename)", Alphabetical);
    sortComboBox->addItem("Sort by Last Modified", ByLastModified);

    // Connect sorting change signal to refresh the file list
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VisualDeckStorageSortWidget::updateSortOrder);
    connect(this, &VisualDeckStorageSortWidget::sortOrderChanged, parent, &VisualDeckStorageWidget::updateSortOrder);

    layout->addWidget(sortComboBox);
}

void VisualDeckStorageSortWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    sortComboBox->setCurrentIndex(SettingsCache::instance().getVisualDeckStorageSortingOrder());
}

void VisualDeckStorageSortWidget::updateSortOrder()
{
    sortOrder = static_cast<SortOrder>(sortComboBox->currentData().toInt());
    SettingsCache::instance().setVisualDeckStorageSortingOrder(sortComboBox->currentData().toInt());
    emit sortOrderChanged();
}

QStringList VisualDeckStorageSortWidget::filterFiles(QStringList &files)
{
    // Sort files based on the current sort order
    std::sort(files.begin(), files.end(), [this](const QString &file1, const QString &file2) {
        QFileInfo info1(file1);
        QFileInfo info2(file2);

        switch (sortOrder) {
            case Alphabetical:
                return info1.fileName().toLower() < info2.fileName().toLower();
            case ByLastModified:
                return info1.lastModified() > info2.lastModified();
        }

        return false; // Default case
    });

    return files;
}
