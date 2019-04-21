/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow OSD widget
 *
 * Copyright (C) 2014-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_SLIDE_OSD_H
#define DIGIKAM_SLIDE_OSD_H

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "slideshowsettings.h"

class QEvent;

namespace Digikam
{

class SlideShow;
class SlideToolBar;

class SlideOSD : public QWidget
{
    Q_OBJECT

public:

    explicit SlideOSD(const SlideShowSettings& settings, SlideShow* const parent = 0);
    ~SlideOSD();

    void setCurrentUrl(const QUrl& url);

    void pause(bool b);
    void video(bool b);
    bool isPaused() const;
    bool isUnderMouse() const;
    void toggleProperties();

    SlideToolBar* toolBar() const;

private Q_SLOTS:

    void slotProgressTimer();
    void slotStart();

private:

    bool eventFilter(QObject* obj, QEvent* ev);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SLIDE_OSD_H
