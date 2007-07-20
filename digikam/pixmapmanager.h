/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-14
 * Description : a pixmap manager for album icon view.  
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PIXMAPMANAGER_H
#define PIXMAPMANAGER_H

// Qt includes.

#include <QObject>
#include <QPixmap>

/** @file pixmapmanager.h */

class QPixmap;

class KUrl;

namespace Digikam
{

class AlbumIconView;
class PixmapManagerPriv;

/**
 * Since there are date based folders, the number of pixmaps which
 * could be kept in memory could potentially become too large. The
 * pixmapmanager maintains a fixed size cache of thumbnails and loads
 * pixmaps on demand.
 */
class PixmapManager : public QObject
{
    Q_OBJECT

public:

    PixmapManager(AlbumIconView* view);
    ~PixmapManager();

    QPixmap  find(const KUrl& path);
    void     clear();
    void     setThumbnailSize(int size);
    int      cacheSize() const;

    /** Removes all cached occurrences of the thumbnail for the given file,
        deletes the thumbnail file on-disk.
        Use when you know that the content of the file has changed.
    */
    void     deleteThumbnail(const KUrl& path);

signals:

    void signalPixmap(const KUrl& url);

private slots:

    void slotGotThumbnail(const KUrl& url, const QPixmap& pix);
    void slotFailedThumbnail(const KUrl& url);
    void slotCompleted();

private:

    PixmapManagerPriv *d;
};

}  // namespace Digikam

#endif /* PIXMAPMANAGER_H */
