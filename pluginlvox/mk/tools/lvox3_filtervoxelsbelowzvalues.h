#ifndef LVOX3_FILTERVOXELSBELOWZVALUES_H
#define LVOX3_FILTERVOXELSBELOWZVALUES_H

#include "lvox3_worker.h"
#include "ct_itemdrawable/abstract/ct_abstractgrid3d.h"
#include "ct_itemdrawable/abstract/ct_abstractimage2d.h"

class LVOX3_FilterVoxelsBelowZValues : public LVOX3_Worker
{
    Q_OBJECT

public:
    LVOX3_FilterVoxelsBelowZValues(const QList<CT_AbstractGrid3D*>& grids,
                                   const CT_AbstractImage2D* zValues);

protected:
    /**
     * @brief Do the job
     */
    void doTheJob();

private:
    QVector<CT_AbstractGrid3D*>     m_grids;
    CT_AbstractImage2D*             m_zValues;
};

#endif // LVOX3_FILTERVOXELSBELOWZVALUES_H
