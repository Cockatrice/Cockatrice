#ifndef COCKATRICE_QT_UTILS_H
#define COCKATRICE_QT_UTILS_H
#include <QDesktopServices>
#include <QFont>
#include <QFontDatabase>
#include <QInputDialog>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <optional>

namespace QtUtils
{
/**
 * @brief Returns the nearest ancestor of @p obj that casts to @c T.
 *
 * Walks the QObject parent chain. QWidget::parentWidget() is the same pointer as
 * QObject::parent() for widgets, so this also covers the common "find the parent tab,
 * splitter or scroll area" lookups.
 *
 * @param obj         Object to start from. May be null.
 * @param includeSelf Also test @p obj itself before walking upwards.
 * @return The first matching ancestor, or nullptr if there is none.
 */
template <typename T> T *findParentOfType(const QObject *obj, bool includeSelf = false)
{
    const QObject *p = includeSelf ? obj : (obj ? obj->parent() : nullptr);
    while (p) {
        if (auto casted = qobject_cast<T *>(const_cast<QObject *>(p))) {
            return casted;
        }
        p = p->parent();
    }
    return nullptr;
}

/**
 * @brief How clearLayoutRec() disposes of the widgets it removes.
 *
 * Deferred is the safe default: deleteLater() lets a widget finish handling the event that
 * triggered the rebuild. Use Immediate when the caller rebuilds the same layout right away
 * and a lingering widget would be visible or otherwise in the way until the next event loop
 * turn.
 */
enum class DeletePolicy
{
    Deferred,
    Immediate
};

/**
 * @brief Removes and deletes every item in @p l, recursing into nested layouts.
 *
 * Widgets are disposed of according to @p policy; spacers and nested layouts are simply
 * deleted.
 */
static inline void clearLayoutRec(QLayout *l, DeletePolicy policy = DeletePolicy::Deferred)
{
    if (!l) {
        return;
    }
    while (QLayoutItem *it = l->takeAt(0)) {
        if (QWidget *w = it->widget()) {
            if (policy == DeletePolicy::Immediate) {
                delete w;
            } else {
                w->deleteLater();
            }
        }
        if (QLayout *sub = it->layout()) {
            clearLayoutRec(sub, policy);
        }
        delete it;
    }
}

/**
 * @brief Creates a layout of type @c LayoutT with zero margins.
 *
 * @param parent  Parent widget, or nullptr when the layout is added later.
 * @param spacing Optional spacing. Values below zero leave the style's default spacing
 *                untouched; pass a non-negative value to call setSpacing().
 * @param margin  Uniform margin. Defaults to zero.
 */
template <typename LayoutT> LayoutT *makeLayout(QWidget *parent = nullptr, int spacing = -1, int margin = 0)
{
    auto *layout = new LayoutT(parent);
    layout->setContentsMargins(margin, margin, margin, margin);
    if (spacing >= 0) {
        layout->setSpacing(spacing);
    }
    return layout;
}

/**
 * @brief Forces @p layout to recompute and repaint immediately.
 *
 * Convenience for the invalidate/activate/update sequence used after changing a layout's
 * contents outside of a resize event.
 */
inline void relayout(QLayout *layout)
{
    if (!layout) {
        return;
    }
    layout->invalidate();
    layout->activate();
    layout->update();
}

/**
 * @brief Creates a QScrollArea using the conventions the client applies nearly everywhere:
 *        the inner widget resizes with the viewport and the horizontal bar is disabled.
 *
 * @param parent           Parent widget, or nullptr for a parentless scroll area.
 * @param verticalPolicy   Vertical scroll bar policy.
 * @param horizontalPolicy Horizontal scroll bar policy.
 */
static inline QScrollArea *makeScrollArea(QWidget *parent = nullptr,
                                          Qt::ScrollBarPolicy verticalPolicy = Qt::ScrollBarAsNeeded,
                                          Qt::ScrollBarPolicy horizontalPolicy = Qt::ScrollBarAlwaysOff)
{
    auto *scrollArea = new QScrollArea(parent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(verticalPolicy);
    scrollArea->setHorizontalScrollBarPolicy(horizontalPolicy);
    return scrollArea;
}

/**
 * @brief A monospace font at the system UI font's point size.
 *
 * Used for log dumps, deck codes and other text that has to line up.
 */
static inline QFont monospaceFont()
{
    QFont font(QStringLiteral("monospace"));
    font.setStyleHint(QFont::Monospace);
    const int systemPointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    if (systemPointSize > 0) {
        font.setPointSize(systemPointSize);
    }
    return font;
}

/**
 * @brief Fades @p window between two opacity values.
 *
 * @param window           Window to animate.
 * @param startOpacity     Starting opacity.
 * @param endOpacity       Ending opacity.
 * @param durationMs       Duration in milliseconds.
 * @param hideWhenFinished Hide the window when the animation finishes.
 * @return The running animation. It is owned by @p window and deletes itself when stopped.
 */
static inline QPropertyAnimation *
fadeWindow(QWidget *window, qreal startOpacity, qreal endOpacity, int durationMs, bool hideWhenFinished = false)
{
    auto *animation = new QPropertyAnimation(window, "windowOpacity", window);
    animation->setDuration(durationMs);
    animation->setStartValue(startOpacity);
    animation->setEndValue(endOpacity);
    if (hideWhenFinished) {
        QObject::connect(animation, &QPropertyAnimation::finished, window, &QWidget::hide);
    }
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    return animation;
}

/**
 * @brief Opens @p path with the platform's file manager or default handler.
 */
static inline bool openPathInFileManager(const QString &path)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

/**
 * @brief Returns @p font with bold styling enabled.
 */
static inline QFont boldFont(QFont font)
{
    font.setBold(true);
    return font;
}

/** Shows an owned message box; parent destruction returns NoButton. */
static inline QMessageBox::StandardButton
messageBox(QWidget *parent,
           QMessageBox::Icon icon,
           const QString &title,
           const QString &text,
           QMessageBox::StandardButtons buttons,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
{
    // A reload can remove the parent while the nested dialog loop is running.
    const bool hadParent = parent != nullptr;
    const QPointer<QWidget> parentGuard(parent);
    QPointer<QMessageBox> dialog = new QMessageBox(icon, title, text, buttons, parent);
    if (defaultButton != QMessageBox::NoButton) {
        dialog->setDefaultButton(defaultButton);
    }
    dialog->exec();
    if (!dialog) {
        return QMessageBox::NoButton;
    }
    const auto result = dialog->standardButton(dialog->clickedButton());
    delete dialog.data();
    return hadParent && !parentGuard ? QMessageBox::NoButton : result;
}

/**
 * @brief Shows a Yes/No confirmation dialog and returns true when the user confirms.
 *
 * @param parent        Parent widget.
 * @param title         Dialog title.
 * @param text          Dialog text.
 * @param defaultButton Button focused by default. NoButton lets Qt choose.
 */
static inline bool confirm(QWidget *parent,
                    const QString &title,
                    const QString &text,
                    QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
{
    return messageBox(parent, QMessageBox::Question, title, text,
                      QMessageBox::Yes | QMessageBox::No, defaultButton) == QMessageBox::Yes;
}

/** Returns accepted text, including an empty value, or nullopt on cancellation. */
static inline std::optional<QString> promptText(QWidget *parent,
                                                const QString &title,
                                                const QString &label,
                                                const QString &text)
{
    const bool hadParent = parent != nullptr;
    const QPointer<QWidget> parentGuard(parent);
    QPointer<QInputDialog> dialog = new QInputDialog(parent);
    dialog->setWindowTitle(title);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(label);
    dialog->setTextEchoMode(QLineEdit::Normal);
    dialog->setTextValue(text);
    const int result = dialog->exec();
    if (!dialog) {
        return std::nullopt;
    }
    const QString value = dialog->textValue();
    delete dialog.data();
    if (result != QDialog::Accepted || (hadParent && !parentGuard)) {
        return std::nullopt;
    }
    return value;
}

} // namespace QtUtils

#endif // COCKATRICE_QT_UTILS_H
