/* ============================================================
 * File  : imageplugin_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-10-01
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>
#include <kdebug.h>

// Local includes.

#include "imageeffect_filmgrain.h"
#include "imageplugin_filmgrain.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_filmgrain,
                            KGenericFactory<ImagePlugin_FilmGrain>("digikamimageplugin_filmgrain"));

ImagePlugin_FilmGrain::ImagePlugin_FilmGrain(QObject *parent, const char*, const QStringList &)
                     : Digikam::ImagePlugin(parent, "ImagePlugin_FilmGrain")
{
    new KAction(i18n("Simulate film grain..."), 0, 
                this, SLOT(slotFilmGrain()),
                actionCollection(), "imageplugin_filmgrain");
                
    
    kdDebug() << "ImagePlugin_FilmGrain plugin loaded" << endl;
}

ImagePlugin_FilmGrain::~ImagePlugin_FilmGrain()
{
}

QStringList ImagePlugin_FilmGrain::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/imageplugin_filmgrain/ ");
    return guiDef;
}

void ImagePlugin_FilmGrain::slotFilmGrain()
{
    DigikamFilmGrainImagesPlugin::ImageEffect_FilmGrain dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_filmgrain.moc"
