#include "intent.h"

Intent::Intent(QObject *parent) : QObject(parent)
{
    // An intent is done as soon as it reports success or failure. Deleting it
    // also tears down its dependency chain and disconnects any signal wiring.
    connect(this, &Intent::finished, this, &QObject::deleteLater);
    connect(this, &Intent::failed, this, &QObject::deleteLater);
}

Intent::~Intent() = default;

void Intent::execute()
{
    if (checkPrecondition()) {
        onPreconditionSatisfied();
    } else {
        onPreconditionNotSatisfied();
    }
}

void Intent::runDependency(Intent *dependency)
{
    dependency->setParent(this);
    connect(dependency, &Intent::finished, this, [this]() {
        // Re-check after dependency finishes
        this->execute();
    });
    connect(dependency, &Intent::failed, this, &Intent::failed);

    dependency->execute();
}

void Intent::emitFinished()
{
    if (!completed) {
        completed = true;
        emit finished();
    }
}

void Intent::emitFailed(const QString &reason)
{
    if (!completed) {
        completed = true;
        emit failed(reason);
    }
}
