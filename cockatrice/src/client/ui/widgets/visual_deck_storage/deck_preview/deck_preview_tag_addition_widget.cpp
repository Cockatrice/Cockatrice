#include "deck_preview_tag_addition_widget.h"

#include "../../../../../dialogs/dlg_convert_deck_to_cod_format.h"
#include "../../../../../settings/cache_settings.h"
#include "deck_preview_tag_dialog.h"

#include <QDirIterator>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <utility>

DeckPreviewTagAdditionWidget::DeckPreviewTagAdditionWidget(QWidget *_parent,
                                                           DeckPreviewDeckTagsDisplayWidget *_tagsDisplayWidget,
                                                           QString _tagName)
    : QWidget(_parent), tagsDisplayWidget(_tagsDisplayWidget), tagName_(std::move(_tagName))
{
    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    // Adjust widget size
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QSize DeckPreviewTagAdditionWidget::sizeHint() const
{
    // Calculate the size based on the tag name
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(tagName_);
    int width = textWidth + 50;    // Add extra padding
    int height = fm.height() + 10; // Height based on font size + padding

    return {width, height};
}

static QStringList getAllFiles(const QString &filePath, bool recursive)
{
    QStringList allFiles;

    // QDirIterator with QDir::Files ensures only files are listed (no directories)
    auto flags =
        recursive ? QDirIterator::Subdirectories | QDirIterator::FollowSymlinks : QDirIterator::NoIteratorFlags;
    QDirIterator it(filePath, QDir::Files, flags);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    return allFiles;
}

void DeckPreviewTagAdditionWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit tagClicked();
    }
    QWidget::mousePressEvent(event);

    if (qobject_cast<DeckPreviewWidget *>(tagsDisplayWidget->parentWidget())) {
        auto *deckPreviewWidget = qobject_cast<DeckPreviewWidget *>(tagsDisplayWidget->parentWidget());
        QStringList knownTags = deckPreviewWidget->visualDeckStorageWidget->tagFilterWidget->getAllKnownTags();
        QStringList activeTags = tagsDisplayWidget->deckList->getTags();

        bool canAddTags = true;

        if (DeckLoader::getFormatFromName(deckPreviewWidget->filePath) != DeckLoader::CockatriceFormat) {
            canAddTags = false;
            // Retrieve saved preference if the prompt is disabled
            if (!SettingsCache::instance().getVisualDeckStoragePromptForConversion()) {
                if (SettingsCache::instance().getVisualDeckStorageAlwaysConvert()) {
                    deckPreviewWidget->deckLoader->convertToCockatriceFormat(deckPreviewWidget->filePath);
                    deckPreviewWidget->filePath = deckPreviewWidget->deckLoader->getLastFileName();
                    deckPreviewWidget->refreshBannerCardText();
                    canAddTags = true;
                }
            } else {
                // Show the dialog to the user
                DialogConvertDeckToCodFormat conversionDialog(parentWidget());
                if (conversionDialog.exec() == QDialog::Accepted) {
                    deckPreviewWidget->deckLoader->convertToCockatriceFormat(deckPreviewWidget->filePath);
                    deckPreviewWidget->filePath = deckPreviewWidget->deckLoader->getLastFileName();
                    deckPreviewWidget->refreshBannerCardText();
                    canAddTags = true;

                    if (conversionDialog.dontAskAgain()) {
                        SettingsCache::instance().setVisualDeckStoragePromptForConversion(Qt::CheckState::Unchecked);
                        SettingsCache::instance().setVisualDeckStorageAlwaysConvert(Qt::CheckState::Checked);
                    }
                } else {
                    SettingsCache::instance().setVisualDeckStorageAlwaysConvert(Qt::CheckState::Unchecked);

                    if (conversionDialog.dontAskAgain()) {
                        SettingsCache::instance().setVisualDeckStoragePromptForConversion(Qt::CheckState::Unchecked);
                    } else {
                        SettingsCache::instance().setVisualDeckStoragePromptForConversion(Qt::CheckState::Checked);
                    }
                }
            }
        }

        if (canAddTags) {
            DeckPreviewTagDialog dialog(knownTags, activeTags);
            if (dialog.exec() == QDialog::Accepted) {
                QStringList updatedTags = dialog.getActiveTags();
                tagsDisplayWidget->deckList->setTags(updatedTags);
                deckPreviewWidget->deckLoader->saveToFile(deckPreviewWidget->filePath, DeckLoader::CockatriceFormat);
            }
        }
    } else if (tagsDisplayWidget->parentWidget()) {
        // If we're the child of a TabDeckEditor, we are buried under a ton of childWidgets in the DeckInfoDock.
        QWidget *currentParent = tagsDisplayWidget->parentWidget();
        while (currentParent) {
            if (qobject_cast<TabDeckEditor *>(currentParent)) {
                break;
            }
            currentParent = currentParent->parentWidget();
        }
        if (qobject_cast<TabDeckEditor *>(currentParent)) {
            auto *deckEditor = qobject_cast<TabDeckEditor *>(currentParent);
            QStringList knownTags;
            QStringList allFiles = getAllFiles(SettingsCache::instance().getDeckPath(), true);
            auto *loader = new DeckLoader();
            for (const QString &file : allFiles) {
                loader->loadFromFile(file, DeckLoader::getFormatFromName(file), false);
                QStringList tags = loader->getTags();
                knownTags.append(tags);
                knownTags.removeDuplicates();
            }

            QStringList activeTags = tagsDisplayWidget->deckList->getTags();

            DeckPreviewTagDialog dialog(knownTags, activeTags);
            if (dialog.exec() == QDialog::Accepted) {
                QStringList updatedTags = dialog.getActiveTags();
                tagsDisplayWidget->deckList->setTags(updatedTags);
                deckEditor->setModified(true);
            }
        }
    }
}

void DeckPreviewTagAdditionWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Set background color
    QColor backgroundColor = Qt::lightGray;
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);

    // Draw background
    painter.drawRect(rect());

    // Draw border
    QColor borderColor = Qt::gray;
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(0, 0, -1, -1)); // Adjust for pen width

    // Calculate font size based on widget height
    QFont font = painter.font();
    int fontSize = std::max(10, height() / 2); // Ensure a minimum font size of 10
    font.setPointSize(fontSize);
    painter.setFont(font);

    // Calculate text rect with margin
    int margin = 10; // Left and right margins
    QRect textRect(margin, 0, width() - margin * 2, height());

    // Draw the text with a black border for better legibility
    painter.setPen(Qt::black);

    // Draw text border by offsetting
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                painter.drawText(textRect.translated(dx, dy), Qt::AlignLeft | Qt::AlignVCenter, tagName_);
            }
        }
    }

    // Draw the actual text
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, tagName_);

    QWidget::paintEvent(event);
}
