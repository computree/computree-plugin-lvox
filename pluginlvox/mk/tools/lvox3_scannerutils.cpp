#include "lvox3_scannerutils.h"
#include "ct_itemdrawable/tools/scanner/ct_thetaphishootingpattern.h"
#include "ct_itemdrawable/tools/scanner/ct_shootingpatternfrompointcloud.h"
#include "ct_itemdrawable/tools/scanner/ct_parallelshootingpatternfrompointcloud.h"

const QVector<ScannerDefinitionStruct>
LVOX3_ScannerUtils::m_scannerDefinitions = {
    { ScannerSphericTheoretic,  LVOX_SP_SPHERE_THEORETIC, "Spheric (theoretical)" },
    { ScannerSphericPointCloud, LVOX_SP_SPHERE_POINTCLOUD, "Spheric (point cloud)" },
    { ScannerPlanePointCloud, LVOX_SP_PLANE_POINTCLOUD,  "Plannar (point cloud)" }
};


LVOX3_ScannerUtils::LVOX3_ScannerUtils()
{
    f.registerType<CT_ThetaPhiShootingPattern>(LVOX_SP_SPHERE_THEORETIC);
    f.registerType<CT_ShootingPatternFromPointCloud>(LVOX_SP_SPHERE_POINTCLOUD);
    f.registerType<CT_ParallelShootingPatternFromPointCloud>(LVOX_SP_PLANE_POINTCLOUD);
}

const QVector<ScannerDefinitionStruct> &LVOX3_ScannerUtils::getScannerDefinitions()
{
    return m_scannerDefinitions;
}

const ScannerDefinitionStruct &LVOX3_ScannerUtils::getScannerDefinition(ScannerTypeEnum id)
{
    return m_scannerDefinitions[id];
}
