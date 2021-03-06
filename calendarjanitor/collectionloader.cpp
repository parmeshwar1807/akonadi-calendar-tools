/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "collectionloader.h"

#include <KCalendarCore/Incidence>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <QString>
#include <QSet>

#include <QDebug>

CollectionLoader::CollectionLoader(QObject *parent)
    : QObject(parent)
{
}

void CollectionLoader::load()
{
    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);

    job->fetchScope().setContentMimeTypes(KCalendarCore::Incidence::mimeTypes());
    connect(job, &Akonadi::CollectionFetchJob::result, this, &CollectionLoader::onCollectionsLoaded);
    job->start();
}

Akonadi::Collection::List CollectionLoader::collections() const
{
    return m_collections;
}

void CollectionLoader::onCollectionsLoaded(KJob *job)
{
    if (job->error() == 0) {
        Akonadi::CollectionFetchJob *cfj = qobject_cast<Akonadi::CollectionFetchJob *>(job);
        Q_ASSERT(cfj);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QSet<QString> mimeTypeSet = KCalendarCore::Incidence::mimeTypes().toSet();
#else
        const QStringList mimetypes = KCalendarCore::Incidence::mimeTypes();
        QSet<QString> mimeTypeSet = QSet<QString>(mimetypes.begin(), mimetypes.end());
#endif
        foreach (const Akonadi::Collection &collection, cfj->collections()) {
            const QStringList contentMimeTypesLst = collection.contentMimeTypes();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
            QSet<QString> collectionMimeTypeSet = contentMimeTypesLst.toSet();
#else
            QSet<QString> collectionMimeTypeSet = QSet<QString>(contentMimeTypesLst.begin(), contentMimeTypesLst.end());
#endif
            if (!mimeTypeSet.intersect(collectionMimeTypeSet).isEmpty()) {
                m_collections << collection;
            }
        }
        Q_EMIT loaded(true);
    } else {
        qCritical() << job->errorString();
        Q_EMIT loaded(false);
    }
}
