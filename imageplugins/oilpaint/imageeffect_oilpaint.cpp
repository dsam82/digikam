/* ============================================================
 * File  : imageeffect_oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
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

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qimage.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kimageeffect.h>
#include <knuminput.h>

// Digikam includes.

#include <imageiface.h>
#include <imagepreviewwidget.h>

// Local includes.

#include "imageeffect_oilpaint.h"

namespace DigikamOilPaintImagesPlugin
{

ImageEffect_OilPaint::ImageEffect_OilPaint(QWidget* parent)
                    : KDialogBase(Plain, i18n("Oil paint"),
                                  Help|Ok|Cancel, Ok,
                                  parent, 0, true, true),
                      m_parent(parent)
{
    // About data and help button.
    KAboutData* about = new KAboutData("oilpaint", I18N_NOOP("Oil Paint"), "1.0",
                                       I18N_NOOP("Oil painting image effect plugin for Digikam."),
                                       KAboutData::License_GPL, "(c) 2004, Gilles Caulier", "",
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Oil paint handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Oil painting image preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Radius :"), plainPage());
    m_numInput = new KDoubleNumInput(plainPage());
    m_numInput->setPrecision(1);
    m_numInput->setRange(0.1, 10.0, 0.1, true);
    m_numInput->setValue(1.0);
    hlay->addWidget(label, 1);
    hlay->addWidget(m_numInput, 5);

    // -------------------------------------------------------------
    
    adjustSize();
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_numInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));            
}

ImageEffect_OilPaint::~ImageEffect_OilPaint()
{
}

void ImageEffect_OilPaint::slotHelp()
{
    KApplication::kApplication()->invokeHelp("oilpaint",
                                             "digikamimageplugins");
}

void ImageEffect_OilPaint::closeEvent(QCloseEvent *e)
{
    delete m_numInput;
    delete m_imagePreviewWidget;
    e->accept();    
}

void ImageEffect_OilPaint::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    double factor = m_numInput->value();
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
    QImage newImage = KImageEffect::oilPaintConvolve(image, factor);
    m_imagePreviewWidget->setPreviewImageData(newImage);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void ImageEffect_OilPaint::slotOk()
{
    accept();
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data    = iface.getOriginalData();
    int w         = iface.originalWidth();
    int h         = iface.originalHeight();
    double factor = m_numInput->value();

    if (data) 
        {
        QImage image;
        image.create( w, h, 32 );
        image.setAlphaBuffer(true) ;
        memcpy(image.bits(), data, image.numBytes());

        QImage newImage = KImageEffect::oilPaintConvolve(image, factor);
    
        memcpy(data, newImage.bits(), newImage.numBytes());
        iface.putOriginalData(data);
        delete [] data;
        }
    
    m_parent->setCursor( KCursor::arrowCursor() );        
}

}  // NameSpace DigikamOilPaintImagesPlugin

#include "imageeffect_oilpaint.moc"
