#ifndef LVOX3_GRIDMODE_H
#define LVOX3_GRIDMODE_H

#include <QObject>

namespace lvox {

    /**
     * @brief Grid mode to use with a preparator
     */
    enum GridMode {
        BoundingBoxOfTheScene = 0,
        RelativeToCoordinates,
        RelativeToCoordinatesAndCustomDimensions,
        CenteredOnCoordinatesAndCustomDimensions,
        FromGridFileParameters
    };

    static QString gridModeToString(const GridMode& m) {
        if(m == BoundingBoxOfTheScene)
            return QObject::tr("Bounding box of the scene");
        else if(m == RelativeToCoordinates)
            return QObject::tr("Relative to coordinates");
        else if(m == RelativeToCoordinatesAndCustomDimensions)
            return QObject::tr("Relative to coordinates and custom dimensions");
        else if(m == CenteredOnCoordinatesAndCustomDimensions)
            return QObject::tr("Centered to coordinates and custom dimensions");
        else if(m == FromGridFileParameters)
            return QObject::tr("From .grid file");

        Q_ASSERT_X(false, "GridModeToString", QString("GridMode %1 cannot be converted to string because it was not implemented, contact the developper !").arg((int)m).toUtf8());

        return "";
    }
}
#endif // LVOX3_GRIDMODE_H
