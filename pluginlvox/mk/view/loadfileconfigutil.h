#ifndef LOADFILECONFIGUTIL_H
#define LOADFILECONFIGUTIL_H

#include <QtCore>
#include <QString>
#include <QTextStream>
#include <QList>

#include <Eigen/Core>

#include "loadfileconfiguration.h"

struct LvoxConfigError {
    LvoxConfigError(const QString msg) : m_msg(msg) {}
    QString m_msg;
};

typedef LoadFileConfiguration::Configuration ConfItem;

class LoadFileConfigUtil
{
    Q_DECLARE_TR_FUNCTIONS(LoadFileConfigUtil)
public:
    LoadFileConfigUtil();

    static void loadInFile(const QString& path, QList<ConfItem>& conf);
    static void loadInData(QTextStream& data, QList<ConfItem>& conf);
    static bool parseInLine(const QString& line, ConfItem& item);

};

#endif // LOADFILECONFIGUTIL_H
