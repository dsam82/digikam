/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to interface digiKam with kipi library.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kipiinterface.h"
#include "kipiinterface.moc"

// KDE includes

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "globals.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "loadingcacheinterface.h"
#include "scancontroller.h"
#include "imageattributeswatch.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "kipiimageinfo.h"
#include "kipiimagecollection.h"

namespace Digikam
{

KipiInterface::KipiInterface(QObject *parent, const char *name)
             : KIPI::Interface(parent, name)
{
    m_thumbLoadThread = ThumbnailLoadThread::defaultThread();
    m_albumManager    = AlbumManager::instance();

    connect(DigikamApp::instance()->view(), SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(m_thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));
}

KipiInterface::~KipiInterface()
{
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
    Album* currAlbum = m_albumManager->currentAlbum();
    if ( currAlbum )
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::AllItems,
                                         currAlbum, 
#if KIPI_VERSION >= 0x000300
                                         hostSetting("FileExtensions").toString()));
#else
                                         fileExtensions()));
#endif
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

KIPI::ImageCollection KipiInterface::currentSelection()
{
    Album* currAlbum = m_albumManager->currentAlbum();
    if ( currAlbum )
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::SelectedItems,
                                                             currAlbum, 
#if KIPI_VERSION >= 0x000300
                                         hostSetting("FileExtensions").toString()));
#else
                                         fileExtensions()));
#endif
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

QList<KIPI::ImageCollection> KipiInterface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
#if KIPI_VERSION >= 0x000300
    QString fileFilter(hostSetting("FileExtensions").toString());
#else
    QString fileFilter(fileExtensions());
#endif

    const AlbumList palbumList = m_albumManager->allPAlbums();
    for ( AlbumList::ConstIterator it = palbumList.constBegin();
          it != palbumList.constEnd(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
            continue;

        KipiImageCollection* col = new KipiImageCollection(KipiImageCollection::AllItems,
                                                           *it, fileFilter);
        result.append( KIPI::ImageCollection( col ) );
    }

    const AlbumList talbumList = m_albumManager->allTAlbums();
    for ( AlbumList::ConstIterator it = talbumList.constBegin();
          it != talbumList.constEnd(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
            continue;

        KipiImageCollection* col = new KipiImageCollection(KipiImageCollection::AllItems,
                                                           *it, fileFilter);
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo KipiInterface::info(const KUrl& url)
{
    return KIPI::ImageInfo(new KipiImageInfo(this, url));
}

void KipiInterface::refreshImages(const KUrl::List& urls)
{
    KUrl::List ulist = urls;

    // Hard Refresh
    QSet<QString> dirs;
    foreach (const KUrl& url, urls)
    {
        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);
        LoadingCacheInterface::fileChanged(path);
        dirs << url.directory();
    }
    foreach (const QString& dir, dirs)
        ScanController::instance()->scheduleCollectionScan(dir);

}

int KipiInterface::features() const
{
    return(
           KIPI::HostSupportsTags            | KIPI::HostSupportsRating      |
           KIPI::HostAcceptNewImages         | KIPI::HostSupportsThumbnails  |
           KIPI::HostSupportsProgressBar     |
           KIPI::ImagesHasComments           | KIPI::ImagesHasTitlesWritable |
           KIPI::ImagesHasTime               |
           KIPI::CollectionsHaveComments     | KIPI::CollectionsHaveCategory |
           KIPI::CollectionsHaveCreationDate
          );
}

bool KipiInterface::addImage( const KUrl& url, QString& errmsg )
{
    // Note : All copy/move operations are processed by the plugins.

    if ( url.isValid() == false )
    {
        errmsg = i18n("Target URL %1 is not valid.",url.path());
        return false;
    }

    PAlbum *targetAlbum = m_albumManager->findPAlbum(url.directory());

    if ( !targetAlbum )
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    //m_albumManager->refreshItemHandler( url );

    return true;
}

void KipiInterface::delImage( const KUrl& url )
{
    KUrl rootURL(CollectionManager::instance()->albumRoot(url));
    if ( !rootURL.isParentOf(url) )
    {
        kWarning(50003) << "URL not in the album library";
    }

    // Is there a PAlbum for this URL

    PAlbum *palbum = m_albumManager->findPAlbum( KUrl(url.directory()) );

    if ( palbum )
    {
        // delete the item from the database
        DatabaseAccess().db()->deleteItem( palbum->id(), url.fileName() );
    }
    else
    {
        kWarning(50003) << "Cannot find Parent album in the album library";
    }
}

void KipiInterface::slotSelectionChanged(int count)
{
    emit selectionChanged(count);
}

void KipiInterface::slotCurrentAlbumChanged( Album *album )
{
    emit currentAlbumChanged( album != 0 );
}

void KipiInterface::thumbnail(const KUrl& url, int /*size*/)
{
    // NOTE: size is not used here. Cache use the max pixmap size to store thumbs (256).
    m_thumbLoadThread->find(url.path());
}

void KipiInterface::thumbnails(const KUrl::List& list, int size)
{
    for (KUrl::List::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        thumbnail((*it).path(), size);
}

void KipiInterface::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    emit gotThumbnail( KUrl(desc.filePath), pix );
}

KIPI::ImageCollectionSelector* KipiInterface::imageCollectionSelector(QWidget *parent)
{
    return (new KipiImageCollectionSelector(this, parent));
}

KIPI::UploadWidget* KipiInterface::uploadWidget(QWidget *parent)
{
    return (new KipiUploadWidget(this, parent));
}

#if KIPI_VERSION >= 0x000300
QVariant KipiInterface::hostSetting(const QString& settingName)
{
    if (settingName == QString("WriteMetadataUpdateFiletimeStamp"))
    {
        return (AlbumSettings::instance()->getUpdateFileTimeStamp());
    }
    else if (settingName == QString("WriteMetadataToRAW"))
    {
        return (AlbumSettings::instance()->getWriteRawFiles());
    }
    else if (settingName == QString("FileExtensions"))
    {
        // do not save this into a local variable, as this
        // might change in the main app

        AlbumSettings* s = AlbumSettings::instance();
        return (s->getImageFileFilter() + ' ' +
                s->getMovieFileFilter() + ' ' +
                s->getAudioFileFilter() + ' ' +
                s->getRawFileFilter());
    }

    return QVariant();
}
#else
QString KipiInterface::fileExtensions()
{
    // do not save this into a local variable, as this
    // might change in the main app

    AlbumSettings* s = AlbumSettings::instance();
    return (s->getImageFileFilter() + ' ' +
            s->getMovieFileFilter() + ' ' +
            s->getAudioFileFilter() + ' ' +
            s->getRawFileFilter());
}
#endif

}  // namespace Digikam
