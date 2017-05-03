/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "htmlwizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <kconfigdialogmanager.h>
#include <klocalizedstring.h>

// Local includes

#include "dwizardpage.h"
#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "galleryinfo.h"
#include "invisiblebuttongroup.h"
#include "htmloutputpage.h"
#include "htmlalbumselectorpage.h"
#include "htmlthemepage.h"
#include "htmlparameterspage.h"
#include "htmlimagesettingspage.h"
#include "htmlfinalpage.h"

namespace Digikam
{

class HTMLWizard::Private
{
public:

    GalleryInfo*           mInfo;
    KConfigDialogManager*  mConfigManager;

    HTMLAlbumSelectorPage* mCollectionSelectorPage;
    HTMLParametersPage*    mParametersPage;
    HTMLImageSettingsPage* mImageSettingsPage;
    HTMLOutputPage*        mOutputPage;
    HTMLThemePage*         mThemePage;
    HTMLFinalPage*         mFinalPage;
};

HTMLWizard::HTMLWizard(QWidget* const parent)
    : QWizard(parent),
      d(new Private)
{
    setWindowTitle(i18n("Export Albums to HTML Pages"));

    d->mInfo = new GalleryInfo;
    d->mInfo->load();

    d->mCollectionSelectorPage = new HTMLAlbumSelectorPage(this, i18n("Albums Selection"));
    d->mThemePage              = new HTMLThemePage(this, i18n("Theme Selection"));
    d->mThemePage->initThemePage(d->mInfo);
    d->mParametersPage         = new HTMLParametersPage(this, i18n("Theme Parameters"));
    d->mImageSettingsPage      = new HTMLImageSettingsPage(this, i18n("Image Settings"));
    d->mOutputPage             = new HTMLOutputPage(this, i18n("Output Settings"));
    d->mFinalPage              = new HTMLFinalPage(this, i18n("Generating Gallery"));
    d->mConfigManager          = new KConfigDialogManager(this, d->mInfo);
    d->mConfigManager->updateWidgets();

    connect(d->mThemePage->mThemeList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThemeSelectionChanged()));

    // Set page states, whoch can only be disabled after they have *all* been added.
    slotThemeSelectionChanged();
}

HTMLWizard::~HTMLWizard()
{
    delete d;
}

void HTMLWizard::slotThemeSelectionChanged()
{
    QListWidget* const listWidget = d->mThemePage->mThemeList;
    QTextBrowser* const browser   = d->mThemePage->mThemeInfo;

    if (listWidget->currentItem())
    {
        Theme::Ptr theme = static_cast<ThemeListBoxItem*>(listWidget->currentItem())->mTheme;

        QString url                   = theme->authorUrl();
        QString author                = theme->authorName();
        bool allowNonsquareThumbnails = theme->allowNonsquareThumbnails();

        if (!url.isEmpty())
        {
            author = QString::fromUtf8("<a href='%1'>%2</a>").arg(url).arg(author);
        }

        QString preview               = theme->previewUrl();
        QString image                 = QLatin1String("");

        if (!preview.isEmpty())
        {
            image = QString::fromUtf8("<img src='%1/%2' /><br/><br/>").arg(theme->directory(),
                                                                           theme->previewUrl());
        }

        QString txt = image +
                      QString::fromUtf8("<b>%3</b><br/><br/>%4<br/><br/>")
                          .arg(theme->name(), theme->comment()) + 
                      i18n("Author: %1", author);

        browser->setHtml(txt);

        d->mImageSettingsPage->kcfg_thumbnailSquare->setEnabled(allowNonsquareThumbnails);

        if (!allowNonsquareThumbnails)
        {
            d->mImageSettingsPage->kcfg_thumbnailSquare->setChecked(true);
        }

        d->mParametersPage->fillThemeParametersPage(theme, d->mInfo);
    }
    else
    {
        browser->clear();
    }
}

int HTMLWizard::parametersPageId() const
{
    return d->mParametersPage->id();
}

int HTMLWizard::imageSettingPageId() const
{
    return d->mImageSettingsPage->id();
}

GalleryInfo* HTMLWizard::galleryInfo() const
{
    return d->mInfo;
}

Theme::Ptr HTMLWizard::theme() const
{
    return (dynamic_cast<ThemeListBoxItem*>(d->mThemePage->mThemeList->currentItem())->mTheme);
}

AlbumList HTMLWizard::albums() const
{
    return d->mCollectionSelectorPage->mCollectionSelector->selectedAlbums();
}

QWidget* HTMLWizard::parametersWidget(const QByteArray& iname) const
{
    return d->mParametersPage->mThemeParameterWidgetFromName[iname];
}

void HTMLWizard::updateSettings()
{
    d->mConfigManager->updateSettings();
}

} // namespace Digikam
