#include "lvox3_filterpointcloud.h"

#include "ct_global/ct_context.h"
#include "ct_iterator/ct_pointiterator.h"

/*
 * FIXME: without ct_scene.h include, the compiler complains:
 *   error: cannot convert
 *   QSharedPointer<CT_AbstractNotModifiableCloudIndexRegisteredT<CT_PointData> >::Type* to
 *   QSharedPointer<CT_AbstractCloudIndexRegisteredT<CT_PointData> >::Type*
 */
#include "ct_itemdrawable/ct_scene.h"

CT_FilterPointCloud::CT_FilterPointCloud()
{
}

CT_PCIR CT_FilterPointCloud::apply(CT_PCIR pcir, CT_FilterPoint filter)
{
    CT_AbstractUndefinedSizePointCloud *cloud = PS_REPOSITORY->createNewUndefinedSizePointCloud();
    CT_PointIterator it(pcir);
    while(it.hasNext()) {
        it.next();
        const CT_Point &p = it.currentPoint();
        if (filter(p)) {
            cloud->addPoint(p);
        }
    }
    return PS_REPOSITORY->registerUndefinedSizePointCloud(cloud);
}

const CT_FilterPoint CT_FilterPointCloud::filter_not_origin = [](const CT_Point& pt)
{
    static const Eigen::Vector3d origin(0, 0, 0);
    return (pt != origin);
};


const CT_FilterPoint CT_FilterPointCloud::filter_default = [](const CT_Point& pt)
{
    Q_UNUSED(pt);
    return true;
};
