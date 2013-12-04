#include "lvox_computehitsthread.h"

LVOX_ComputeHitsThread::LVOX_ComputeHitsThread(CT_Scanner *scanner,
                                               CT_Grid3D<int> *grilleHits,
                                               CT_Grid3D<double> *grilleIn,
                                               CT_Grid3D<double> *grilleOut,
                                               const CT_Scene *scene,
                                               double intensityThresh,
                                               bool greaterThan) : QThread()
{
    _scanner = scanner;
    _grilleHits = grilleHits;
    _grilleIn = grilleIn;
    _grilleOut = grilleOut;
    _scene = scene;
    _intensityThresh = intensityThresh;
    _greaterThan = greaterThan;
}

void LVOX_ComputeHitsThread::run()
{
    const CT_AbstractPointCloud *pointCloud = _scene->getPointCloud();
    const CT_AbstractPointCloudIndex *pointCloudIndex = _scene->getPointCloudIndex();
    quint64 n_points = pointCloudIndex->indexSize();
    QVector3D scanPos = _scanner->getPosition();
    float res = _grilleHits->resolution();

    for (quint64 i = 0 ; i < n_points; i++)
    {
        const int &index = (*pointCloudIndex)[i];
        const CT_Point &point = (*pointCloud)[index];
        int indice = _grilleHits->indexAtXYZ(point.x, point.y, point.z);

        if (indice < 0)
        {
            qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
        } else
        {
            // Hits Computing
            if    ((_greaterThan && (point.intensity > _intensityThresh))
                   || (!_greaterThan && (point.intensity < _intensityThresh)))
            {
                _grilleHits->addValueAtIndex(indice, 1);

                // Distances Sum Computing
                QVector3D direction (point.x - scanPos.x(),
                                     point.y - scanPos.y(),
                                     point.z - scanPos.z());

                QVector3D bottom, top, in, out;
                _grilleHits->getCellBottomLeftCornerAtXYZ(point.x, point.y, point.z, bottom);
                top.setX(bottom.x() + res);
                top.setY(bottom.y() + res);
                top.setZ(bottom.z() + res);

                CT_Beam beam(NULL, NULL, scanPos, direction);

                if (beam.intersect(bottom, top, in, out))
                {
                    double distanceIn = sqrt(pow(in.x()-point.x, 2) + pow(in.y()-point.y, 2) + pow(in.z()-point.z, 2));
                    double distanceOut = sqrt(pow(out.x()-point.x, 2) + pow(out.y()-point.y, 2) + pow(out.z()-point.z, 2));

                    _grilleIn->addValueAtIndex(indice, distanceIn);
                    _grilleOut->addValueAtIndex(indice, distanceOut);
                }
            }
        }
    }

    // Convert sums into means
    int ncells = _grilleHits->nCells();
    for (int i = 0 ; i < ncells ; i++)
    {
        double value = _grilleHits->valueAtIndex(i);
        int na = _grilleHits->NA();
        if (value==0 || value==na)
        {
            _grilleIn->setValueAtIndex(i, _grilleIn->NA());
            _grilleOut->setValueAtIndex(i, _grilleOut->NA());
        } else {
            double invalue = _grilleIn->valueAtIndex(i) / value;
            double outvalue = _grilleOut->valueAtIndex(i) / value;

            _grilleIn->setValueAtIndex(i, invalue);
            _grilleOut->setValueAtIndex(i, outvalue);
        }
    }
    _grilleHits->computeMinMax();
    _grilleIn->computeMinMax();
    _grilleOut->computeMinMax();
}

