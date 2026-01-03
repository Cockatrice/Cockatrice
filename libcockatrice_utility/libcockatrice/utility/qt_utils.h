#ifndef COCKATRICE_QT_UTILS_H
#define COCKATRICE_QT_UTILS_H
#include <QObject>

namespace QtUtils
{
template <typename T> T *findParentOfType(const QObject *obj)
{
    const QObject *p = obj ? obj->parent() : nullptr;
    while (p) {
        if (auto casted = qobject_cast<T *>(const_cast<QObject *>(p))) {
            return casted;
        }
        p = p->parent();
    }
    return nullptr;
}

static inline void clearLayoutRec(QLayout *l)
{
    if (!l)
        return;
    QLayoutItem *it;
    while ((it = l->takeAt(0)) != nullptr) {
        if (QWidget *w = it->widget())
            w->deleteLater();
        if (QLayout *sub = it->layout())
            clearLayoutRec(sub);
        delete it;
    }
}
} // namespace QtUtils

#endif // COCKATRICE_QT_UTILS_H
