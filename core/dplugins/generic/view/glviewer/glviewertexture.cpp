/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * Copyright (C) 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * Copyright (C) 2008-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "glviewertexture.h"

// Qt includes

#include <QUrl>

// Local includes

#include "dimg.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "previewloadthread.h"
#include "dmetadata.h"
#include "digikam_debug.h"
#include "glviewertimer.h"

namespace DigikamGenericGLViewerPlugin
{

class GLViewerTexture::Private
{
public:

    Private()
    {
        rdx            = 0.0;
        rdy            = 0.0;
        z              = 0.0;
        ux             = 0.0;
        uy             = 0.0;
        rtx            = 0.0;
        rty            = 0.0;
        vtop           = 0.0;
        vbottom        = 0.0;
        vleft          = 0.0;
        vright         = 0.0;
        display_x      = 0;
        display_y      = 0;
        rotate_idx     = 0;
        rotate_list[0] = DMetadata::ORIENTATION_ROT_90;
        rotate_list[1] = DMetadata::ORIENTATION_ROT_180;
        rotate_list[2] = DMetadata::ORIENTATION_ROT_270;
        rotate_list[3] = DMetadata::ORIENTATION_ROT_180;
        iface          = 0;
    }

    float                              rdx, rdy, z, ux, uy, rtx, rty;
    float                              vtop, vbottom, vleft, vright;
    int                                display_x, display_y;
    QString                            filename;
    QImage                             qimage;
    QSize                              initial_size;
    DMetadata::ImageOrientation        rotate_list[4];
    int                                rotate_idx;
    IccProfile                         iccProfile;
    DInfoInterface*                    iface;

private:

    // No copy constructor
    Private(const Private&);
};

GLViewerTexture::GLViewerTexture(DInfoInterface* const iface)
    : QOpenGLTexture(QOpenGLTexture::TargetRectangle),
      d(new Private)
{
    d->iface = iface;

    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (settings.enableCM && settings.useManagedPreviews)
    {
        d->iccProfile = IccProfile(settings.monitorProfile);
    }

    reset();
}

GLViewerTexture::~GLViewerTexture()
{
    delete d;
}

/*!
    \fn GLViewerTexture::load(QString fn, QSize size)
    \brief load file from disc and save it in texture
    \param fn filename to load
    \param size size of image which is downloaded to texture mem
    if "size" is set to image size, scaling is only performed by the GPU but not
    by the CPU, however the AGP usage to texture memory is increased (20MB for a 5mp image)
 */
bool GLViewerTexture::load(const QString& fn, const QSize& size)
{
    d->filename     = fn;
    d->initial_size = size;
    d->qimage       = PreviewLoadThread::loadFastSynchronously(d->filename,
                                                               qMax(size.width(), size.height()),
                                                               d->iccProfile).copyQImage();

    if (d->qimage.isNull())
    {
        return false;
    }

    // handle rotation

    DItemInfo info(d->iface->itemInfo(QUrl::fromLocalFile(d->filename)));

    if (info.orientation() != DMetadata::ORIENTATION_UNSPECIFIED)
    {
        DMetadata meta;
        meta.rotateExifQImage(d->qimage, (DMetadata::ImageOrientation)info.orientation());
    }

    if (d->qimage.isNull())
    {
        return false;
    }

    loadInternal();
    reset();
    d->rotate_idx = 0;
    return true;
}

/*!
    \fn GLViewerTexture::load(QImage im, QSize size)
    \brief copy file from QImage to texture
    \param im Qimage to be copied from
    \param size size of image which is downloaded to texture mem
    if "size" is set to image size, scaling is only performed by the GPU but not
    by the CPU, however the AGP usage to texture memory is increased (20MB for a 5mp image)
 */
bool GLViewerTexture::load(const QImage& im, const QSize& size)
{
    d->qimage       = im;
    d->initial_size = size;
    loadInternal();
    reset();
    d->rotate_idx   = 0;
    return true;
}

/*!
    \brief load full size image from disc and save it in texture
 */
bool GLViewerTexture::loadFullSize()
{
    d->qimage       = PreviewLoadThread::loadHighQualitySynchronously(d->filename,
                                                                      PreviewSettings::RawPreviewAutomatic,
                                                                      d->iccProfile).copyQImage();

    if (d->qimage.isNull())
    {
        return false;
    }

    d->initial_size = d->qimage.size();

    // handle rotation

    DItemInfo info(d->iface->itemInfo(QUrl::fromLocalFile(d->filename)));

    if (info.orientation() != DMetadata::ORIENTATION_UNSPECIFIED)
    {
        DMetadata meta;
        meta.rotateExifQImage(d->qimage, (DMetadata::ImageOrientation)info.orientation());
    }

    if (d->qimage.isNull())
    {
        return false;
    }

    loadInternal();
    reset();
    d->rotate_idx = 0;
    return true;
}

/*!
    \fn GLViewerTexture::load()
    internal load function
    rt[xy] <= 1
 */
bool GLViewerTexture::loadInternal()
{
    int w = d->initial_size.width();
    int h = d->initial_size.height();

    destroy();

    if (w == 0 || w > d->qimage.width() || h > d->qimage.height())
    {
        setData(d->qimage.mirrored());
    }
    else
    {
        setData(d->qimage.scaled(w, h, Qt::KeepAspectRatio,
                                 Qt::FastTransformation).mirrored());
    }

    setMinificationFilter(QOpenGLTexture::Linear);
    setMagnificationFilter(QOpenGLTexture::Linear);

    w = width();
    h = height();

    if (h < w)
    {
        d->rtx = 1;
        d->rty = float(h) / float(w);
    }
    else
    {
        d->rtx = float(w) / float(h);
        d->rty = 1;
    }

    return true;
}

/*!
    \fn GLViewerTexture::zoom(float delta, QPoint mousepos)
    \brief calculate new tex coords on zooming
    \param delta delta between previous zoom and current zoom
    \param mousepos mouse position returned by QT
    \TODO rename mousepos to something more generic
*/
void GLViewerTexture::zoom(float delta, const QPoint& mousepos)
// u: start in texture, u = [0..1], u = 0 is begin, u = 1 is end of texture
// z = [0..1], z = 1 -> no zoom
// l: length of tex in glFrustum coordinate system
// rt: ratio of tex, rt <= 1, see loadInternal() for definition
// rd: ratio of display, rd >= 1
// m: mouse pos normalized, cd = [0..rd]
// c: mouse pos normalized to zoom * l, c = [0..1]
{
    d->z    *= delta;
    delta    =  d->z * (1.0 / delta - 1.0); //convert to real delta = z_old - z_new

    float mx = mousepos.x() / (float)d->display_x * d->rdx;
    float cx = (mx-d->rdx / 2.0 + d->rtx / 2.0) / d->rtx;
    float vx = d->ux + cx * d->z;
    d->ux    = d->ux + (vx - d->ux) * delta / d->z;

    float my = mousepos.y() / (float)d->display_y * d->rdy;
    float cy = (my-d->rdy / 2.0 + d->rty / 2.0) / d->rty;
    cy       = 1 - cy;
    float vy = d->uy + cy * d->z;
    d->uy    = d->uy + (vy - d->uy) * delta / d->z;

    calcVertex();
}

/*!
    \fn GLViewerTexture::calcVertex()
    Calculate vertices according internal state variables
    z, ux, uy are calculated in GLViewerTexture::zoom()
 */
void GLViewerTexture::calcVertex()
// rt: ratio of tex, rt <= 1, see loadInternal() for definition
// u: start in texture, u = [0..1], u=0 is begin, u=1 is end of texture
// l: length of tex in glFrustum coordinate system
// halftexel: the color of a texel is determined by a corner of the texel and not its center point
//            this seems to introduce a visible jump on changing the tex-size.
//
// the glFrustum coord-sys is visible in [-rdx..rdx] ([-1..1] for square screen) for z = 1 (no zoom)
// the tex coord-sys goes from [-rtx..rtx] ([-1..1] for square texture)
{
    // x part
    float lx          = 2 * d->rtx / d->z;   //length of tex
    float tsx         = lx / (float)width(); //texelsize in glFrustum coordinates
    float halftexel_x = tsx / 2.0;
    float wx          = lx * (1 - d->ux - d->z);
    d->vleft          = -d->rtx - d->ux * lx - halftexel_x; //left
    d->vright         = d->rtx + wx - halftexel_x;          //right

    // y part
    float ly          = 2 * d->rty / d->z;
    float tsy         = ly / (float)height(); //texelsize in glFrustum coordinates
    float halftexel_y = tsy / 2.0;
    float wy          = ly * (1 - d->uy - d->z);
    d->vbottom        = -d->rty - d->uy * ly + halftexel_y; //bottom
    d->vtop           = d->rty + wy + halftexel_y;          //top
}

/*!
    \fn GLViewerTexture::vertex_bottom() const
 */
GLfloat GLViewerTexture::vertex_bottom() const
{
    return (GLfloat) d->vbottom;
}

/*!
    \fn GLViewerTexture::vertex_top() const
 */
GLfloat GLViewerTexture::vertex_top() const
{
    return (GLfloat) d->vtop;
}

/*!
    \fn GLViewerTexture::vertex_left() const
 */
GLfloat GLViewerTexture::vertex_left() const
{
    return (GLfloat) d->vleft;
}

/*!
    \fn GLViewerTexture::vertex_right() const
 */
GLfloat GLViewerTexture::vertex_right() const
{
    return (GLfloat) d->vright;
}

/*!
    \fn GLViewerTexture::setViewport(int w, int h)
    \param w width of window
    \param h height of window
    Set widget's viewport. Ensures that rdx & rdy are always > 1
 */
void GLViewerTexture::setViewport(int w, int h)
{
    if (h > w)
    {
        d->rdx = 1.0;
        d->rdy = h / float(w);
    }
    else
    {
        d->rdx = w / float(h);
        d->rdy = 1.0;
    }

    d->display_x = w;
    d->display_y = h;
}

/*!
    \fn GLViewerTexture::move(QPoint diff)
    new tex coordinates have to be calculated if the view is panned
 */
void GLViewerTexture::move(const QPoint& diff)
{
    d->ux = d->ux - diff.x() / float(d->display_x) * d->z * d->rdx / d->rtx;
    d->uy = d->uy + diff.y() / float(d->display_y) * d->z * d->rdy / d->rty;
    calcVertex();
}

/*!
    \fn GLViewerTexture::reset()
 */
void GLViewerTexture::reset()
{
    d->ux           = 0;
    d->uy           = 0;
    d->z            = 1.0;
    float zoomdelta = 0;

    if ((d->rtx < d->rty) && (d->rdx < d->rdy) && (d->rtx / d->rty < d->rdx / d->rdy))
    {
        zoomdelta = d->z - d->rdx / d->rdy;
    }

    if ((d->rtx < d->rty) && (d->rtx / d->rty > d->rdx / d->rdy))
    {
        zoomdelta = d->z - d->rtx;
    }

    if ((d->rtx >= d->rty) && (d->rdy < d->rdx) && (d->rty / d->rtx < d->rdy / d->rdx))
    {
        zoomdelta = d->z - d->rdy / d->rdx;
    }

    if ((d->rtx >= d->rty) && (d->rty / d->rtx > d->rdy / d->rdx))
    {
        zoomdelta = d->z - d->rty;
    }

    QPoint p = QPoint(d->display_x / 2, d->display_y / 2);
    zoom(1.0 - zoomdelta, p);

    calcVertex();
}

/*!
    \fn GLViewerTexture::setNewSize(QSize size)
    \param size desired texture size. QSize(0,0) will take the full image
    \return true if size has changed, false otherwise
    set new texture size in order to reduce AGP bandwidth
 */
bool GLViewerTexture::setNewSize(QSize size)
{
    // don't allow larger textures than the original image. the image will be upsampled by
    // OpenGL if necessary and not by QImage::scale
    size = size.boundedTo(d->qimage.size());

    if (width() == size.width())
    {
        return false;
    }

    int w = size.width();
    int h = size.height();

    destroy();

    if (w == 0)
    {
        setData(d->qimage.mirrored());
    }
    else
    {
        setData(d->qimage.scaled(w, h, Qt::KeepAspectRatio,
                                 Qt::FastTransformation).mirrored());
    }

    setMinificationFilter(QOpenGLTexture::Linear);
    setMagnificationFilter(QOpenGLTexture::Linear);

    // recalculate half-texel offset
    calcVertex();

    return true;
}

/*!
    \fn GLViewerTexture::rotate()
    \brief smart image rotation
    since the two most frequent usecases are a CW or CCW rotation of 90,
    perform these rotation with one (+90) or two (-90) calls of rotation()
 */
void GLViewerTexture::rotate()
{
    DMetadata meta;
    meta.rotateExifQImage(d->qimage, (DMetadata::ImageOrientation)d->rotate_list[d->rotate_idx % 4]);

    loadInternal();

    // save new rotation in host application

    DInfoInterface::DInfoMap info;
    DItemInfo item(info);
    item.setOrientation(d->rotate_list[d->rotate_idx % 4]);
    d->iface->setItemInfo(QUrl::fromLocalFile(d->filename), info);

    reset();
    d->rotate_idx++;
}

/*!
    \fn GLViewerTexture::setToOriginalSize()
    zoom image such that each pixel of the screen corresponds to a pixel in the jpg
    remember that OpenGL is not a pixel exact specification, and the image will still be filtered by OpenGL
 */
void GLViewerTexture::zoomToOriginal()
{
    float zoomfactorToOriginal;
    reset();

    if (float(d->qimage.width()) / float(d->qimage.height()) > float(d->display_x) / float(d->display_y))
    {
        //image touches right and left edge of window
        zoomfactorToOriginal = float(d->display_x) / d->qimage.width();
    }
    else
    {
        //image touches upper and lower edge of window
        zoomfactorToOriginal = float(d->display_y) / d->qimage.height();
    }

    zoom(zoomfactorToOriginal, QPoint(d->display_x / 2, d->display_y / 2));
}

} // namespace DigikamGenericGLViewerPlugin
