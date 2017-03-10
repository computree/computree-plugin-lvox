#include "lvox3_computehits.h"

#include "ct_itemdrawable/ct_beam.h"
#include "ct_iterator/ct_pointiterator.h"

#include "mk/tools/lvox3_gridtools.h"
#include "mk/tools/lvox3_rayboxintersectionmath.h"
#include "mk/tools/lvox3_errorcode.h"

LVOX3_ComputeHits::LVOX3_ComputeHits(const CT_ShootingPattern* pattern,
                                     const CT_AbstractPointCloudIndex* pointCloudIndex,
                                     lvox::Grid3Di* hits,
                                     lvox::Grid3Df* shotInDistance,
                                     lvox::Grid3Df* shotOutDistance) : LVOX3_Worker()
{
    m_pattern = pattern;
    m_pointCloudIndex = pointCloudIndex;
    m_hits = hits;
    m_shotInDistance = shotInDistance;
    m_shotOutDistance = shotOutDistance;
}

void LVOX3_ComputeHits::doTheJob()
{
    size_t i = 0, indice, pointCol, pointLin, pointLevel;
    Eigen::Vector3d bottom, top, in, out;

    const size_t n_points = m_pointCloudIndex->size();

    bool computeDistance = (m_shotInDistance != NULL) || (m_shotOutDistance != NULL);

    setProgressRange(0, computeDistance ? n_points+1 : n_points);

    LVOX3_GridTools gridTool(m_hits);

    CT_PointIterator itP(m_pointCloudIndex);

    while (itP.hasNext()
           && !mustCancel())
    {
        ++i;
        const CT_Point &point = itP.next().currentPoint();

        // we now that the grid is perfectly bounding the scene so we can use this tools that don't do
        // many check to reduce the compute time !
        gridTool.computeGridIndexForPoint(point, pointCol, pointLin, pointLevel, indice);

        if(!lvox::FilterCode::isFiltered(m_hits->valueAtIndex(indice))) {
            m_hits->addValueAtIndex(indice, 1);

            if (computeDistance)
            {
                gridTool.computeCellBottomLeftTopRightCornerAtColLinLevel(pointCol, pointLin, pointLevel, bottom, top);
                CT_Shot shot = m_pattern->getShotAt(indice);
                Eigen::Vector3d shotOrig = shot.getOrigin();
                if (LVOX3_RayBoxIntersectionMath::getIntersectionOfRay(bottom, top, shotOrig, point - shotOrig, in, out))
                {
                    if(m_shotInDistance != NULL)
                        m_shotInDistance->addValueAtIndex(indice, (in-point).norm());

                    if(m_shotOutDistance != NULL)
                        m_shotOutDistance->addValueAtIndex(indice, (out-point).norm());
                }
            }
        }

        setProgress(i);
    }

    m_hits->computeMinMax(); // Calcul des limites hautes et basses des valeurs de la grille => Nécessaire à la visualisation

    if (computeDistance
            && !mustCancel())
    {
        // Convert sums into means
        size_t ncells = m_hits->nCells();

        for (size_t i = 0 ; (i < ncells) && !mustCancel(); i++)
        {
            float nHits = m_hits->valueAtIndex(i);

            if (nHits <= 0)
            {
                if(m_shotInDistance != NULL)
                    m_shotInDistance->setValueAtIndex(i, nHits);

                if(m_shotOutDistance != NULL)
                    m_shotOutDistance->setValueAtIndex(i, nHits);
            } else {
                if(m_shotInDistance != NULL)
                    m_shotInDistance->setValueAtIndex(i, m_shotInDistance->valueAtIndex(i) / nHits);

                if(m_shotOutDistance != NULL)
                    m_shotOutDistance->setValueAtIndex(i, m_shotOutDistance->valueAtIndex(i) / nHits);
            }
        }

        if(m_shotInDistance != NULL)
            m_shotInDistance->computeMinMax();

        if(m_shotOutDistance != NULL)
            m_shotOutDistance->computeMinMax();

        setProgress(n_points+1);
    }
}
