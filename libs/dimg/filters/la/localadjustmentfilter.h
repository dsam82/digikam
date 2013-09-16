/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-22
 * Description : Auto Crop analyser
 *
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LOCALADJUSTMENTFILTER_H
#define LOCALADJUSTMENTFILTER_H

// Qt includes

#include <QObject>

#include <QImage>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimgthreadedanalyser.h"

namespace Digikam
{


class DIGIKAM_EXPORT LAContainer
{

public:

    LAContainer();

    LAContainer(DImg part);

    ~LAContainer();

public:


    double red;
    double blue;
    double green;
    double alpha;
    double hue;
    double saturation;
    double vibrance;
    double lightness;
    int radius;
    QPoint center;            //Center with respect to the m_orgImage
    QPoint selectionCenter;   //Equivalent center point in the selection
    DImg selection;
    DImg modSelection;
};

//! kDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const LAContainer& inf);

class DIGIKAM_EXPORT LocalAdjustments : public DImgThreadedFilter
{
public:

    /** Standard constructor with image container to parse
         */
//    explicit LocalAdjustments(DImg* const orgImage, int x, int y, int radius, QObject* const parent = 0);
//    explicit LocalAdjustments(DImg* const orgImage, int n, LAContainer lac[], QObject* const parent = 0);

    explicit LocalAdjustments(DImg* const orgImage, QObject* const parent = 0);

    LocalAdjustments(DImg *const orgImage, QObject *const parent, const LAContainer& settings);

    ~LocalAdjustments();

    /**
     * @brief startAnalyse
     * To start the filter.
     */
    void startAnalyse();

    void filterImage();


    void readParameters(const FilterAction& action);

    virtual FilterAction    filterAction();
    virtual QString         filterIdentifier() const;

    static QString          FilterIdentifier();
    static QString          DisplayableName();
    static QList<int>       SupportedVersions();
    static int              CurrentVersion();



    /**
     * @brief addSelection
     * To add a select
     * @param lac
     * @return
     */
    int addSelection(LAContainer lac);

    /**
     * @brief getSelection
     * Returns the selection of that particular Index (for testing)
     * @param index
     * index of the selection. (integer from 0 to 19)
     * @return
     */
    DImg getSelection(int index);



    /**
         * Applies the DImg selection on the parent photo. Uses normal
         * blending. ##INCOMPLETE##
         */
    DImg applyDImgSelection(int index);

    DImg returnOrigImage();


    /**
     * @brief modifyRGBA   to change the parameters of the RGBA of that selection
     * @param index        to fix the selection
     * @param r            red component
     * @param g            green component
     * @param b            blue component
     * @param a            alpha component
     */
    void modifyRGBA(int index, double r, double g, double b, double a);

//    DImg getModifiedSelection(double r, double g, double b, double a);

    /**
     * @brief getModifiedSelection
     * Returns the immediate modSelection of that index. Used for tests
     * @param index
     * @return
     */

    DImg getModifiedSelection(int index);

    /**
     * @brief getFinalImage
     * It performs all the tasks, and recreates the final image, and gives as a output
     * N.B. - you need to explicitly run, startFilterDirectly, before running this
     *        function
     * @return
     *        a DImg as the same size of m_orgImage
     */
    DImg getFinalImage();


    /**
         * @brief changeBrightness increases or decreases brightness, takes in
         * a value -255 to 255
         * @param brightness integer ranging from -255 to 255 (even for 16bit images)
         */
    //void changeBrightness(int brightness);

    /**
         * @brief printHSL
         * prints the HSL values of the pixels in a file. Check for study only
         * @param image
         */
    //void printHSL(DImg image);

    /**
         * @brief changeRGBA
         * all the parameters are values ranging from -100.0 to 100.0
         * @param r - red component
         * @param g - green component
         * @param b - blue component
         * @param a - alpha component
         * @return
         */
//    void changeRGBA(LAContainer &lac, double r, double g, double b, double a);


    //DImg* applySelection(QString path);


private :

    /** Returns a QPoint of the center of the selection.
         */
    QPoint centerSelection();

    QPoint centerSelection(int x, int y, int radius);

    /** Returns the selection in RGBA32 format
         *  QImage source is the image from which the selection is to be taken
         *  innerRadius and OuterRadius are integers with radius of the selection
         *  origCenter determines the point of the center of the circle
         */
    DImg getSelection();

    /**
     * @brief createSelection
     * Creates a selection based on the pointer of the LAContainer
     * @param lac
     * The pointer of the LAContainer to be worked on.
     */
    void createSelection(LAContainer* lac);

    /**
     * @brief createSelection
     * Creates a selection based on the index of the d->sln
     * @param index
     * Integer which gives the information about which index to work on.
     */
    void createSelection(int index);

    /** This is an overloaded function
         *  Returns the selection in RGBA32 format
         *  QImage source is the image from which the selection is to be taken
         *  OuterRadius are integers with radius of the selection
         *  InnerRadius is considered to be 70% of the outerRadius
         *  origCenter determines the point of the center of the circle
         */
    //QImage getSelection(int outerRadius, QPoint origCenter);


    /**
         *  Returns a DImg of the selection with the soft edge, uses DImg
         *  for support of 16bit images.
         */
    DImg getDImgSoftSelection();

    /**
         * @brief makes the color selection of the center point and the color
         * of all the pixels of the selection. Changes the alpha value of the
         * selection.
         */

    DImg getDImgColorSelection();

    DImg getDImgColorSelection(DImg &selection, QPoint selectionCenter);

    DImg getDImgSoftSelection(int index);

    DImg getDImgSoftSelection(QPoint origCenter, QPoint selectionCenter, int radius);

    /** Returns the file, after applying the selection Image on the
         *  original image, to produce the output.
         */
    void apply8bitSelection(DImg &destination, DImg &source);

    /**
     * @brief apply16bitSelection
     *        ##INCOMPLETE##
     * @param selection
     * @return
     */
    void apply16bitSelection(DImg &destination, DImg &source);

    /**
     * @brief changeSingleRGBA
     * creates modSelection for a single selection identified by index
     * @param index
     */
    void changeSingleRGBA(int index);

    /**
     * @brief changeAllRGBA
     * to create modSelection from Selection
     */
    void changeAllRGBA();

    /**
     * @brief applyAllSelections
     * Applies all the selections to the finalImage.
     */
    void applyAllSelections();

    /** returns the circular selection with color
         *  QImage source is the original image from which the selection is to be made
         *  int outerRadius is the human input value of Outer Radius (value >=0)
         *  QPoint origCenter is the center in the source Image.
         *  QImage getcolorSelection(QImage selection, QPoint selectionCenter);
         */
    //QImage getcolorSelection(QImage selection, QPoint selectionCenter);

    /** Returns the blurred/soft edge selection
         *  QImage selection is the hard selection
         *  int innerRadius determines the radius inside which alpha values are 255
         *  int outerRadius determines the radius beyond which alpha values are 0
         *  QPoint selectionCenter determines the center of the selection with respect to the original Image
         */
    //QImage getSoftSelection(QImage source, int innerRadius, int outerRadius, QPoint origCenter);

    QImage createLayer(QImage selection);

    DImg createDImgLayer(DImg &selection, QPoint SelectionCenter, QPoint origCenter);

    DImg createDImgLayer(int index);

    /** Image Conversions */
    void srgb2lch(float fimg[][4], int size);

    void srgb2lab(float fimg[][4], int size);

    void srgb2xyz(float fimg[][4], int size);

    void xyz2srgb(float fimg[][4], int size);

    void lab2srgb(float fimg[][4], int size);

    //void srgb2hsv(float fimg[][4], int size);

    /** To calculate differences between pixels of a image, and a particular
         *  reference array. Stores the output in the float* difference array
         */
    void colorDifference(float fimg[][4], float reference[4], float* difference, int size);

private :

    class Private;
    Private* d;
};

}   //  namespace Digikam

#endif /*LOCALADJUSTMENTFILTER_H*/

