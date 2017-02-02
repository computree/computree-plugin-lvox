#include "lvox3_computelvoxgridspreparator.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_scanner.h"
#include "ct_itemdrawable/ct_shootingpatternd.h"
#include "ct_itemdrawable/abstract/ct_abstractimage2d.h"

#include "ctlibio/readers/ct_reader_larchitect_grid.h"
#include "ct_iterator/ct_resultgroupiterator.h"

LVOX3_ComputeLVOXGridsPreparator::LVOX3_ComputeLVOXGridsPreparator()
{

}

LVOX3_ComputeLVOXGridsPreparator::Result LVOX3_ComputeLVOXGridsPreparator::prepare(const CT_VirtualAbstractStep* step,
                                                                                   CT_ResultGroupIterator& itGrp,
                                                                                   const QString& sceneModelName,
                                                                                   const QString& scannerModelName,
                                                                                   const QString& shotPatternModelName,
                                                                                   const QString& mntModelName,
                                                                                   double gridResolution,
                                                                                   lvox::GridMode gridMode,
                                                                                   Coordinates coord,
                                                                                   const QString& gridFilePath)
{
    Result res;

    double dNumericsLimit = std::numeric_limits<double>::max();

    // Global limits of generated grids
    res.minBBox = Eigen::Vector3d(dNumericsLimit, dNumericsLimit, dNumericsLimit);
    res.maxBBox = res.minBBox * -1;

    Eigen::Vector3d minScene = res.minBBox;
    Eigen::Vector3d maxScene = res.maxBBox;

    Eigen::Vector3d minShootingPattern = res.minBBox;
    Eigen::Vector3d maxShootingPattern = res.maxBBox;

    Eigen::Vector3d sceneBBOXMin, sceneBBOXMax;

    // on va rechercher tous les groupes contenant des nuages de points (qui ont été choisi par l'utilisateur)
    while (itGrp.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGrp.next();

        const CT_Scene* scene = (CT_Scene*)group->firstItemByINModelName(step, sceneModelName);
        const CT_Scanner* scanner = (CT_Scanner*)group->firstItemByINModelName(step, scannerModelName);
        const CT_ShootingPatternD* pattern = (CT_ShootingPatternD*)group->firstItemByINModelName(step, shotPatternModelName);
        const CT_AbstractImage2D* mnt = (CT_AbstractImage2D*)group->firstItemByINModelName(step, mntModelName);

        if ((scene != NULL)
                && ((scanner != NULL) || (pattern != NULL)))
        {
            ToCompute tc;
            tc.scene = (CT_Scene*)scene;
            tc.pattern = (CT_ShootingPattern*)((scanner == NULL) ? pattern->getShootingPattern() : scanner->getShootingPattern());
            tc.mnt = (CT_AbstractImage2D*)mnt;

            const Eigen::Vector3d& origin = tc.pattern->getOrigin();

            res.elementsToCompute.insert(group, tc);

            scene->getBoundingBox(sceneBBOXMin, sceneBBOXMax);

            if (sceneBBOXMin.x() < minScene.x()) {minScene.x() = sceneBBOXMin.x();}
            if (sceneBBOXMin.y() < minScene.y()) {minScene.y() = sceneBBOXMin.y();}
            if (sceneBBOXMin.z() < minScene.z()) {minScene.z() = sceneBBOXMin.z();}
            if (sceneBBOXMax.x() > maxScene.x()) {maxScene.x() = sceneBBOXMax.x();}
            if (sceneBBOXMax.y() > maxScene.y()) {maxScene.y() = sceneBBOXMax.y();}
            if (sceneBBOXMax.z() > maxScene.z()) {maxScene.z() = sceneBBOXMax.z();}

            if (origin.x() < minShootingPattern.x()) {minShootingPattern.x() = origin.x();}
            if (origin.y() < minShootingPattern.y()) {minShootingPattern.y() = origin.y();}
            if (origin.z() < minShootingPattern.z()) {minShootingPattern.z() = origin.z();}

            if (origin.x() > maxShootingPattern.x()) {maxShootingPattern.x() = origin.x();}
            if (origin.y() > maxShootingPattern.y()) {maxShootingPattern.y() = origin.y();}
            if (origin.z() > maxShootingPattern.z()) {maxShootingPattern.z() = origin.z();}
        }
    }

    PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + lvox::gridModeToString(gridMode));

    bool mustBeControledAndAdjustedIfNecessary = false;

    const Eigen::Vector3d minAdjusted(qMin(minScene.x(), minShootingPattern.x()),
                                      qMin(minScene.y(), minShootingPattern.y()),
                                      qMin(minScene.z(), minShootingPattern.z()));

    const Eigen::Vector3d maxAdjusted(qMax(maxScene.x(), maxShootingPattern.x()),
                                      qMax(maxScene.y(), maxShootingPattern.y()),
                                      qMax(maxScene.z(), maxShootingPattern.z()));

    if (gridMode == lvox::BoundingBoxOfTheScene) {

        res.minBBox.x() = qMin(minScene.x(), minShootingPattern.x());
        res.minBBox.y() = qMin(minScene.y(), minShootingPattern.y());
        res.minBBox.z() = qMin(minScene.z(), minShootingPattern.z());

        res.maxBBox.x() = qMax(maxScene.x(), maxShootingPattern.x());
        res.maxBBox.y() = qMax(maxScene.y(), maxShootingPattern.y());
        res.maxBBox.z() = qMax(maxScene.z(), maxShootingPattern.z());

        res.minBBox.array() -= gridResolution;
        res.maxBBox.array() += gridResolution;

    } else if (gridMode == lvox::RelativeToCoordinates) {

        res.minBBox = coord.coordinate;
        res.maxBBox.array() += gridResolution;

        mustBeControledAndAdjustedIfNecessary = true;

    } else if (gridMode == lvox::RelativeToCoordinatesAndCustomDimensions) {

        res.minBBox = coord.coordinate;
        res.maxBBox = res.minBBox + (gridResolution * coord.dimension);

        mustBeControledAndAdjustedIfNecessary = true;

    } else if (gridMode == lvox::CenteredOnCoordinatesAndCustomDimensions)   {

        res.minBBox = coord.coordinate - (gridResolution * coord.dimension);
        res.maxBBox = coord.coordinate + (gridResolution * coord.dimension);

        mustBeControledAndAdjustedIfNecessary = true;

    } else if (gridMode == lvox::FromGridFileParameters) {

        bool ok = false;

#ifdef CT_LARCHIHEADER_H
        QFileInfo fileInfo(gridFilePath);

        if (fileInfo.exists())
        {
            QString extension = fileInfo.completeSuffix();

            CT_Reader_LArchitect_Grid reader;

            if(reader.isExtensionPresentInReadableFormat(extension)){
                PS_LOG->addMessage(LogInterface::trace, LogInterface::step, QObject::tr("File reader created"));

                if (reader.setFilePath(gridFilePath) != NULL)
                {
                    reader.init();

                    CT_LARCHIHEADER* header = (CT_LARCHIHEADER*)reader.readHeader();

                    if(header != NULL) {
                        PS_LOG->addInfoMessage(LogInterface::step, "  : DIM XYZ [" + QString::number(header->getcoord.dimension.x()()) + ";" + QString::number(header->getcoord.dimension.y()()) + ";" + QString::number(header->getcoord.dimension.z()()) + "] ");
                        PS_LOG->addInfoMessage(LogInterface::step, "  : RES XYZ [" + QString::number(header->getresolution(0)) + ";" + QString::number(header->getresolution(1)) + ";" + QString::number(header->getresolution(2)) + "] ");
                        PS_LOG->addInfoMessage(LogInterface::step, "  : MIN XYZ [" + QString::number(header->get_res.minBBox.x()()) + ";" + QString::number(header->get_res.minBBox.y()()) + ";" + QString::number(header->get_res.minBBox.z()()) + "] ");

                        res.minBBox.x() = header->get_res.minBBox.x()();
                        res.minBBox.y() = header->get_res.minBBox.y()();
                        res.minBBox.z() = header->get_res.minBBox.z()();

                        coord.dimension.x() = header->getcoord.dimension.x()()*header->getresolution(0)/gridResolution;
                        coord.dimension.y() = header->getcoord.dimension.y()()*header->getresolution(1)/gridResolution;
                        coord.dimension.z() = header->getcoord.dimension.z()()*header->getresolution(2)/gridResolution;

                        res.maxBBox.x() = res.minBBox.x() + header->getcoord.dimension.x()()*header->getresolution(0);
                        res.maxBBox.y() = res.minBBox.y() + header->getcoord.dimension.y()()*header->getresolution(1);
                        res.maxBBox.z() = res.minBBox.z() + header->getcoord.dimension.z()()*header->getresolution(2);

                        PS_LOG->addInfoMessage(LogInterface::step, "  : NEW Base position [" + QString::number(res.minBBox.x()) + ";" + QString::number(res.minBBox.y()) + ";" + QString::number(res.minBBox.z()) + "] " + "gridResolution out : " + QString::number(gridResolution));
                        PS_LOG->addInfoMessage(LogInterface::step, "  : SIZE (m) [" + QString::number(coord.dimension.x()*gridResolution) + ";" + QString::number(coord.dimension.y()*gridResolution) + ";" + QString::number(coord.dimension.z()*gridResolution) + "] " + "gridResolution in : " + QString::number(header->getresolution(0)));
                        PS_LOG->addInfoMessage(LogInterface::step, "  : NEW DIM [" + QString::number(coord.dimension.x()) + ";" + QString::number(coord.dimension.y()) + ";" + QString::number(coord.dimension.z()) + "] " + "gridResolution out : " + QString::number(gridResolution));

                        delete header;

                        ok = true;
                    }
                }
            }
        }
#else
        Q_UNUSED(gridFilePath);
#endif
        if(!ok)
            return res;
      }

    if(mustBeControledAndAdjustedIfNecessary) {

        bool adjusted = false;

        while ((res.minBBox.x()+gridResolution) < minAdjusted.x()) {res.minBBox.x() += gridResolution; adjusted = true;}
        while ((res.minBBox.y()+gridResolution) < minAdjusted.y()) {res.minBBox.y() += gridResolution; adjusted = true;}
        while ((res.minBBox.z()+gridResolution) < minAdjusted.z()) {res.minBBox.z() += gridResolution; adjusted = true;}

        while (res.minBBox.x() > minAdjusted.x()) {res.minBBox.x() -= gridResolution; adjusted = true;}
        while (res.minBBox.y() > minAdjusted.y()) {res.minBBox.y() -= gridResolution; adjusted = true;}
        while (res.minBBox.z() > minAdjusted.z()) {res.minBBox.z() -= gridResolution; adjusted = true;}

        while ((res.maxBBox.x()-gridResolution) > maxAdjusted.x()) {res.maxBBox.x() -= gridResolution; adjusted = true;}
        while ((res.maxBBox.y()-gridResolution) > maxAdjusted.y()) {res.maxBBox.y() -= gridResolution; adjusted = true;}
        while ((res.maxBBox.z()-gridResolution) > maxAdjusted.z()) {res.maxBBox.z() -= gridResolution; adjusted = true;}

        while (res.maxBBox.x() < maxAdjusted.x()) {res.maxBBox.x() += gridResolution; adjusted = true;}
        while (res.maxBBox.y() < maxAdjusted.y()) {res.maxBBox.y() += gridResolution; adjusted = true;}
        while (res.maxBBox.z() < maxAdjusted.z()) {res.maxBBox.z() += gridResolution; adjusted = true;}

        if ((gridMode != lvox::RelativeToCoordinates) && adjusted)
            PS_LOG->addMessage(LogInterface::warning, LogInterface::step, QObject::tr("Dimensions spécifiées ne contenant pas les positions de scans : la grille a du être élargie !"));
    }

    res.valid = true;

    return res;
}
