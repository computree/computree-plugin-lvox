/**
 * @author Michael Krebs (AMVALOR)
 * @date 25.01.2017
 * @version 1
 */
#ifndef LVOX3_COMPUTETHEORITICALS_H
#define LVOX3_COMPUTETHEORITICALS_H

#include "lvox3_worker.h"
#include "mk/tools/lvox3_gridtype.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/scanner/ct_shootingpattern.h"

/*!
 * @brief Computes the "theoricals" grid of a scene
 */
class LVOX3_ComputeTheoriticals : public LVOX3_Worker
{
    Q_OBJECT

public:
    LVOX3_ComputeTheoriticals(const CT_ShootingPattern* pattern,
                              lvox::Grid3Di* theoricals,
                              lvox::Grid3Df* shotDeltaDistance = NULL);

    ~LVOX3_ComputeTheoriticals();

protected:
    /**
     * @brief Do the job
     */
    void doTheJob();

private:
    const CT_ShootingPattern*   m_pattern;
    lvox::Grid3Di*              m_outputTheoriticalGrid;
    lvox::Grid3Df*              m_outputDeltaTheoriticalGrid;

    friend class Temp;
};

#endif // LVOX3_COMPUTETHEORITICALS_H
