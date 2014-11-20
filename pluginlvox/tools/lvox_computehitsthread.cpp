#include "lvox_computehitsthread.h"
#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"

LVOX_ComputeHitsThread::LVOX_ComputeHitsThread(const CT_Scanner *scanner,
                                               CT_Grid3D<int> *grilleHits,
                                               CT_Grid3D<float> *grilleIn,
                                               CT_Grid3D<float> *grilleOut,
                                               const CT_Scene *scene,
                                               bool computeDistance) : CT_MonitoredQThread()
{
    _scanner = scanner;
    _grilleHits = grilleHits;
    _grilleIn = grilleIn;
    _grilleOut = grilleOut;
    _scene = scene;
    _computeDistance = computeDistance;
}

void LVOX_ComputeHitsThread::run()
{
    qDebug() << "Début de LVOX_ComputeHitsThread / ScanId=" << _scanner->getScanID();
    const CT_AbstractPointCloudIndex *pointCloudIndex = _scene->getPointCloudIndex();
    size_t n_points = pointCloudIndex->size();
    QVector3D scanPos = _scanner->getPosition();
    float res = _grilleHits->resolution();

    int progressStep = n_points / 20;

    for (size_t i = 0 ; i < n_points; i++)
    {
        size_t index;
        const CT_Point &point = pointCloudIndex->constTAt(i, index);
        size_t indice;

        if (_grilleHits->indexAtXYZ(point(CT_Point::X), point(CT_Point::Y), point(CT_Point::Z), indice))
        {
            // Hits Computing
            _grilleHits->addValueAtIndex(indice, 1);

            if (_computeDistance)
            {
                // Distances Sum Computing
                QVector3D direction (point(CT_Point::X) - scanPos.x(),
                                     point(CT_Point::Y) - scanPos.y(),
                                     point(CT_Point::Z) - scanPos.z());

                QVector3D bottom, top, in, out;
                _grilleHits->getCellBottomLeftCornerAtXYZ(point(CT_Point::X), point(CT_Point::Y), point(CT_Point::Z), bottom);
                top.setX(bottom.x() + res);
                top.setY(bottom.y() + res);
                top.setZ(bottom.z() + res);

                CT_Beam beam(NULL, NULL, scanPos, direction);

                if (beam.intersect(bottom, top, in, out))
                {
                    float distanceIn = sqrt(pow(in.x()-point(CT_Point::X), 2) + pow(in.y()-point(CT_Point::Y), 2) + pow(in.z()-point(CT_Point::Z), 2));
                    float distanceOut = sqrt(pow(out.x()-point(CT_Point::X), 2) + pow(out.y()-point(CT_Point::Y), 2) + pow(out.z()-point(CT_Point::Z), 2));

                    _grilleIn->addValueAtIndex(indice, distanceIn);
                    _grilleOut->addValueAtIndex(indice, distanceOut);
                }
            }
        } else
        {
            qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
        }

        if (i % progressStep == 0)
        {
            _progress = 100*i/n_points;
            emit progressChanged();
        }
    }

    _grilleHits->computeMinMax();

    if (_computeDistance)
    {
        // Convert sums into means
        size_t ncells = _grilleHits->nCells();
        for (size_t i = 0 ; i < ncells ; i++)
        {
            float value = _grilleHits->valueAtIndex(i);
            int na = _grilleHits->NA();
            if (value==0 || value==na)
            {
                _grilleIn->setValueAtIndex(i, _grilleIn->NA());
                _grilleOut->setValueAtIndex(i, _grilleOut->NA());
            } else {
                float invalue = _grilleIn->valueAtIndex(i) / value;
                float outvalue = _grilleOut->valueAtIndex(i) / value;

                _grilleIn->setValueAtIndex(i, invalue);
                _grilleOut->setValueAtIndex(i, outvalue);
            }
        }
        _grilleIn->computeMinMax();
        _grilleOut->computeMinMax();
    }

    _progress = 100;
    emit progressChanged();
    qDebug() << "Fin de LVOX_ComputeHitsThread / ScanId=" << _scanner->getScanID();
}

