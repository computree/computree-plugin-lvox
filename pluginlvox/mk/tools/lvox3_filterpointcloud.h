#ifndef LVOX3_FILTERPOINTCLOUD_H
#define LVOX3_FILTERPOINTCLOUD_H

#include <functional>
#include "ct_defines.h"
#include "ct_global/ct_context.h"
#include "ct_point.h"

/**
 * @brief CT_PointFilter return true to keep the point, false otherwise
 */
typedef std::function<bool (const CT_Point& pt)> CT_FilterPoint;

class CT_FilterPointCloud
{
private:
    CT_FilterPointCloud();
public:
    /**
     * @brief apply returns a filtered point cloud given the filter function
     * @param pcir the origin point cloud
     * @param filter filter function
     * @return the filtered point cloud
     */
    static CT_PCIR apply(CT_PCIR pcir, CT_FilterPoint filter);

    static const CT_FilterPoint filter_not_origin;
    static const CT_FilterPoint filter_default;

};

#endif // LVOX3_FILTERPOINTCLOUD_H
