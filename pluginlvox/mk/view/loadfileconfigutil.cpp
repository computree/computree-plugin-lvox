#include "loadfileconfigutil.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>

#include <QDebug>

LoadFileConfigUtil::LoadFileConfigUtil()
{

}

void LoadFileConfigUtil::loadInFile(const QString& path, QList<ConfItem>& conf)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        throw LvoxConfigError(file.errorString());
    }
    QFileInfo info(path);
    QString dir = info.path();
    QTextStream data(&file);
    loadInData(data, conf);
    for (ConfItem& item: conf) {
        item.filepath = QString("%1/%2").arg(dir).arg(item.filepath);
    }
}

bool LoadFileConfigUtil::parseInLine(const QString &line, ConfItem& item)
{
    QStringList l = line.split(" ", QString::SkipEmptyParts);

    if (l.count() != 10)
        return false;

    item.filepath = l.at(0);
    item.clockWise = true;
    item.radians = false;
    item.scannerPosition.x() = l.at(2).toDouble();
    item.scannerPosition.y() = l.at(3).toDouble();
    item.scannerPosition.z() = l.at(4).toDouble();
    item.scannerResolution.x() = l.at(5).toDouble();
    item.scannerResolution.y() = l.at(5).toDouble();
    item.scannerThetaRange.x() = l.at(6).toDouble();
    item.scannerThetaRange.y() = l.at(7).toDouble();
    item.scannerPhiRange.x() = l.at(8).toDouble();
    item.scannerPhiRange.y() = l.at(9).toDouble();

    return true;
}

void LoadFileConfigUtil::loadInData(QTextStream &data, QList<ConfItem>& conf)
{
    QString line;
    int n = 1;
    while(data.readLineInto(&line)) {
        ConfItem item;
        if (!parseInLine(line, item)) {
            throw LvoxConfigError(tr("Error parsing line %1").arg(n));
        }
        conf.append(item);
        n++;
    }
}
