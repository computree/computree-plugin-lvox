#include "lvox3_computebefore.h"

#include "mk/tools/traversal/woo/lvox3_grid3dwootraversalalgorithm.h"
#include "mk/tools/traversal/woo/visitor/lvox3_countvisitor.h"
#include "mk/tools/traversal/woo/visitor/lvox3_distancevisitor.h"
#include "mk/tools/lvox3_errorcode.h"
#include "mk/tools/lvox3_gridtools.h"

#include "ct_itemdrawable/tools/gridtools/ct_grid3dwootraversalalgorithm.h"
#include "ct_iterator/ct_pointiterator.h"

LVOX3_ComputeBefore::LVOX3_ComputeBefore(const CT_ShootingPattern* pattern,
                                         const CT_AbstractPointCloudIndex* pointCloudIndex,
                                         lvox::Grid3Di* before,
                                         lvox::Grid3Df* shotDeltaDistance)
{
    m_pattern = pattern;
    m_pointCloudIndex = pointCloudIndex;
    m_before = before;
    m_shotDeltaDistance = shotDeltaDistance;
}

void LVOX3_ComputeBefore::doTheJob()
{
    size_t n_points = m_pointCloudIndex->size();

    // Creates visitors
    QVector<LVOX3_Grid3DVoxelWooVisitor*> list;

    LVOX3_CountVisitor<lvox::Grid3DiType> countVisitor(m_before);
    LVOX3_DistanceVisitor<lvox::Grid3DfType> distVisitor(m_shotDeltaDistance);

    list.append(&countVisitor);

    if (m_shotDeltaDistance != NULL)
        list.append(&distVisitor);

    // Creates traversal algorithm
    LVOX3_Grid3DWooTraversalAlgorithm<lvox::Grid3DiType> algo(m_before, false, list);

    setProgressRange(0, (m_shotDeltaDistance != NULL) ? n_points+1 : n_points);
    size_t i = 0;

    CT_PointIterator itP(m_pointCloudIndex);

    LVOX3_GridTools gridTool(m_before);

    while (itP.hasNext()
           && !mustCancel())
    {
        size_t indice;
        const CT_Point &point = itP.next().currentPoint();
        gridTool.computeGridIndexForPoint(point, indice);
        Eigen::Vector3d shotOrigin = m_pattern->getShotAt(indice).getOrigin();

        // algo already check if the beam touch the grid or not so we don't have to do twice !
        algo.compute(point, point - shotOrigin);

        ++i;
        setProgress(i);
    }

    // Don't forget to calculate min and max in order to visualize it as a colored map
    m_before->computeMinMax();

    if ((m_shotDeltaDistance != NULL)
            && !mustCancel())
    {
        // To get the mean distance we have to divide in each voxel the sum of distances by the number of hits
        for (int i = 0 ; i < m_before->nCells() && !mustCancel() ; i++ )
        {
            const float nHits = m_before->valueAtIndex(i);

            if (nHits <= 0)
                m_shotDeltaDistance->setValueAtIndex(i, nHits);  // TODO : check if must set an error code here
            else
                m_shotDeltaDistance->setValueAtIndex(i, m_shotDeltaDistance->valueAtIndex(i)/nHits);
        }

        m_shotDeltaDistance->computeMinMax();

        setProgress(n_points+1);
    }
}
