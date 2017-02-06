/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_GRIDTYPE_H
#define LVOX3_GRIDTYPE_H

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/ct_image2d.h"

#include <QMutex>

namespace lvox {
    typedef qint32                      Grid3DiType;
    typedef float                       Grid3DfType;
    typedef QMutex                      MutexType;

    static QMetaType::Type Grid3DiMetaType() { return QMetaType::Int; }
    static QMetaType::Type Grid3DfMetaType() { return QMetaType::Float; }

    typedef CT_Grid3D<Grid3DiType>      Grid3Di;
    typedef CT_Grid3D<Grid3DfType>      Grid3Df;
    typedef std::vector<MutexType*>     MutexCollection;
    typedef CT_Image2D<float>           SkyRaster;
}

#endif // LVOX3_GRIDTYPE_H
