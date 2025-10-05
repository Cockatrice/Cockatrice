/**
 * @file get_text_with_max.h
 * @ingroup UI
 * @brief Custom QInputDialog::getText implementation that allows configuration of the max length
 */

#ifndef GETTEXTWITHMAX_H
#define GETTEXTWITHMAX_H

#include <QInputDialog>
#include <libcockatrice/utility/trice_limits.h>

QString getTextWithMax(QWidget *parent,
                       const QString &title,
                       const QString &label,
                       QLineEdit::EchoMode echo = QLineEdit::Normal,
                       const QString &text = QString(),
                       bool *ok = nullptr,
                       int max = MAX_NAME_LENGTH,
                       Qt::WindowFlags flags = Qt::WindowFlags(),
                       Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
static inline QString getTextWithMax(QWidget *parent, const QString &title, const QString &label, int max)
{
    return getTextWithMax(parent, title, label, QLineEdit::Normal, QString(), nullptr, max);
}

#endif // GETTEXTWITHMAX_H
