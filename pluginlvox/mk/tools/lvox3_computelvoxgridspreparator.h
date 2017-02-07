/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_COMPUTELVOXGRIDSPREPARATOR_H
#define LVOX3_COMPUTELVOXGRIDSPREPARATOR_H

#include <QString>
#include <QHash>

#include "lvox3_gridmode.h"
#include "Eigen/Core"

class CT_AbstractItemGroup;
class CT_Scene;
class CT_ShootingPattern;
class CT_AbstractImage2D;
class CT_ResultGroupIterator;
class CT_VirtualAbstractStep;

/**
 * @brief Use this class to prepare all elements to compute lvox grids
 */
class LVOX3_ComputeLVOXGridsPreparator
{
public:
    struct ToCompute {
        CT_Scene*               scene;
        CT_ShootingPattern*     pattern;
        CT_AbstractImage2D*     mnt;
        CT_AbstractImage2D*     sky;
    };

    struct Result {
        typedef QHash<CT_AbstractItemGroup*, ToCompute >            ToComputeCollection;
        typedef QHashIterator<CT_AbstractItemGroup*, ToCompute >    ToComputeCollectionIterator;

        Result() { valid = false; }

        bool                valid;
        ToComputeCollection elementsToCompute;
        Eigen::Vector3d     minBBox;
        Eigen::Vector3d     maxBBox;
    };

    struct Coordinates {
        Eigen::Vector3d coordinate;
        Eigen::Vector3d dimension;
    };

    LVOX3_ComputeLVOXGridsPreparator();

    /**
     * @brief Call it to prepare elements to compute lvox grids
     * @param step : your step (this)
     * @param itGrp : iterator on group that contains the scene, scanner or shooting pattern
     * @param sceneModelName : scene model name to find it
     * @param scannerModelName : scanner model name to find it
     * @param shotPatternModelName : shooting pattern model name to find it
     * @param mntModelName : mnt model name to find it
     * @param skyModelName : sky model name to find it
     * @param gridResolution : the grid resolution
     * @param gridMode : grid mode to use
     * @param coord : coordinate information if gridMode == ...Coordinates...
     * @param gridFilePath : filepath to .grid file (onlyt if gridMode == FromGridFileParameters)
     * @return result of the preparation
     */
    Result prepare(const CT_VirtualAbstractStep* step,
                   CT_ResultGroupIterator& itGrp,
                   const QString& sceneModelName,
                   const QString& scannerModelName,
                   const QString& shotPatternModelName,
                   const QString& mntModelName,
                   const QString& skyModelName,
                   double gridResolution,
                   lvox::GridMode gridMode,
                   Coordinates coord = Coordinates(),
                   const QString& gridFilePath = "");
};

#endif // LVOX3_COMPUTELVOXGRIDSPREPARATOR_H
