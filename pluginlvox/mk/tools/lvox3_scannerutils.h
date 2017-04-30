#ifndef LVOX3_SCANNERUTILS_H
#define LVOX3_SCANNERUTILS_H

#include <QVector>
#include "ct_itemdrawable/tools/scanner/ct_shootingpattern.h"
#include "mk/tools/lvox3_factory.hpp"

/*
 * Scanner definitions
 * TODO: move to pluginshared
 */

#define LVOX_SP_SPHERE_THEORETIC "SphericTheroretic"
#define LVOX_SP_SPHERE_POINTCLOUD "SphericPointCloud"
#define LVOX_SP_PLANE_POINTCLOUD "PlanePointCloud"

enum ScannerTypeEnum {
    ScannerSphericTheoretic = 0,
    ScannerSphericPointCloud,
    ScannerPlanePointCloud,
};

/*
 * TODO: implement following geometries for completeness:
 *   - CYLINDRIC_THERORETIC
 *   - CYLINDRIC_POINTCLOUD
 *   - PLANE_THEORETIC
 */

struct ScannerDefinitionStruct {
    ScannerTypeEnum id;
    QString label;
    QString name;
};

class LVOX3_ScannerUtils
{
public:
    LVOX3_ScannerUtils();
    static const QVector<ScannerDefinitionStruct> &getScannerDefinitions();
    static const ScannerDefinitionStruct &getScannerDefinition(ScannerTypeEnum id);

private:
    Factory<CT_ShootingPattern> f;
    static const QVector<ScannerDefinitionStruct> m_scannerDefinitions;
};

#endif // LVOX3_SCANNERUTILS_H
