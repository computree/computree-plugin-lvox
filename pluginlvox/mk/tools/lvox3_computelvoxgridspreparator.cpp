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
                                                                                   const QString& skyModelName,
                                                                                   double gridResolution,
                                                                                   lvox::GridMode gridMode,
                                                                                   Coordinates coord,
                                                                                   const QString& gridFilePath)
{
    Result res;
    Eigen::AlignedBox3d sceneBox;
    Eigen::AlignedBox3d scannerBox;
    Eigen::AlignedBox3d voxelBox;

    // on va rechercher tous les groupes contenant des nuages de points (qui ont été choisi par l'utilisateur)
    while (itGrp.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGrp.next();

        const CT_Scene* scene = (CT_Scene*)group->firstItemByINModelName(step, sceneModelName);
        const CT_Scanner* scanner = (CT_Scanner*)group->firstItemByINModelName(step, scannerModelName);

        const CT_AbstractImage2D* mnt = (CT_AbstractImage2D*)group->firstItemByINModelName(step, mntModelName);
        const CT_AbstractImage2D* sky = (CT_AbstractImage2D*)group->firstItemByINModelName(step, skyModelName);

        if (scene && scanner)
        {
            ToCompute tc;
            tc.scene = (CT_Scene*)scene;
            tc.pattern = scanner->getShootingPattern();
            tc.mnt = (CT_AbstractImage2D*)mnt;
            tc.sky = (CT_AbstractImage2D*)sky;

            res.elementsToCompute.insert(group, tc);

            /* Union of all scanners
             *
             * FIXME: center coordinate may not be enough to compute the
             * bounding box of the parallel shooting pattern.
             */
            const Eigen::Vector3d& scanPos = tc.pattern->getCenterCoordinate();
            scannerBox.extend(scanPos);

            /* Union of all scenes */
            Eigen::Vector3d sceneBBOXMin, sceneBBOXMax;
            scene->getBoundingBox(sceneBBOXMin, sceneBBOXMax);
            sceneBox.extend(sceneBBOXMin);
            sceneBox.extend(sceneBBOXMax);
        }
    }

    PS_LOG->addInfoMessage(LogInterface::trace, "Mode " + lvox::gridModeToString(gridMode));
    Eigen::Vector3d gridLengths = gridResolution * coord.dimension;

    switch(gridMode) {
    case lvox::BoundingBoxOfTheScene:
        voxelBox.extend(sceneBox);
        break;
    case lvox::RelativeToCoordinates:
        voxelBox.extend(coord.coordinate);
        voxelBox.extend(sceneBox.max());
        break;
    case lvox::RelativeToCoordinatesAndCustomDimensions:
        // custom bounding box, where useless voxels are removed
        voxelBox.extend(coord.coordinate);
        voxelBox.extend(coord.coordinate + gridLengths);
        voxelBox.clamp(sceneBox);
        break;
    case lvox::CenteredOnCoordinatesAndCustomDimensions:
        voxelBox.extend(coord.coordinate - gridLengths);
        voxelBox.extend(coord.coordinate + gridLengths);
        break;
    case lvox::FromGridFileParameters:
        Q_UNUSED(gridFilePath);
        PS_LOG->addInfoMessage(LogInterface::step, "LArchitect bounding box not supported");
        return res;
        break;
    default:
        break;
    }

    // extends the bounding box to include the scanner
    if (!voxelBox.contains(scannerBox)) {
        PS_LOG->addMessage(LogInterface::warning, LogInterface::step,
                           QObject::tr("Les dimensions de la grille ont été "
                                       "élargies grille pour inclure les "
                                       "scanneurs"));
        voxelBox.extend(scannerBox);
    }

    // Add padding around the grid
    Eigen::Vector3d v1 = voxelBox.min().array() - gridResolution;
    Eigen::Vector3d v2 = voxelBox.max().array() + gridResolution;
    voxelBox.extend(v1);
    voxelBox.extend(v2);

    res.minBBox = voxelBox.min();
    res.maxBBox = voxelBox.max();

    PS_LOG->addMessage(LogInterface::info, LogInterface::step,
                       QObject::tr("Voxel bounding box: (%1,%2,%3), (%4,%5,%6)")
                        .arg(res.minBBox.x(), 5, 'f', 1)
                        .arg(res.minBBox.y(), 5, 'f', 1)
                        .arg(res.minBBox.z(), 5, 'f', 1)
                        .arg(res.maxBBox.x(), 5, 'f', 1)
                        .arg(res.maxBBox.y(), 5, 'f', 1)
                        .arg(res.maxBBox.z(), 5, 'f', 1)
                       );

    res.valid = true;
    return res;
}

/* legacy untested LArchitect code CT_LARCHIHEADER_H */
#if 0
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
#endif
