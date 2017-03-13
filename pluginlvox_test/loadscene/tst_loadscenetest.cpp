#include <QString>
#include <QtTest>
#include <QTextStream>

#include "mk/view/loadfileconfiguration.h"
#include "mk/view/loadfileconfigutil.h"

#include "Eigen/Core"

#define MULTI_LINE_STRING(...) #__VA_ARGS__

const char *simple_scene = MULTI_LINE_STRING(
    18600518_tree_v0.asc Z+F 0.0 0.0 1.5 0.036 0 180 60 120\n
    18600518_tree_v1.asc Z+F 15.00000 0 1.5 0.036 0 180 0 360\n
    18600518_tree_v2.asc Z+F 0 15.00000 1.5 0.036 0 180 0 360\n
    18600518_tree_v3.asc Z+F -15.00000 0 1.5 0.036 0 180 0 360\n
    18600518_tree_v4.asc Z+F 0 -15.00000 1.5 0.036 0 180 0 360\n
);

class LoadsceneTest : public QObject
{
    Q_OBJECT

public:
    LoadsceneTest();

private Q_SLOTS:
    void testCheckException();
    void testParseItem();
    void testParseList();
    void testParseFile();
};

LoadsceneTest::LoadsceneTest()
{
}

void LoadsceneTest::testCheckException()
{
    QDir dir(SRCDIR);
    QFile bad(dir.absoluteFilePath("bad_file.in"));
    QList<ConfItem> conf;

    QVERIFY_EXCEPTION_THROWN(LoadFileConfigUtil::loadInFile("file_not_found", conf), LvoxConfigError);
    QVERIFY(bad.exists());
    QVERIFY_EXCEPTION_THROWN(LoadFileConfigUtil::loadInFile(bad.fileName(), conf), LvoxConfigError);
}

void LoadsceneTest::testParseItem()
{
    QString line = "18600518_tree_v0.asc Z+F 1.0 2.0 3.0 0.036 0 180 60 120";

    ConfItem item;
    QVERIFY(LoadFileConfigUtil::parseInLine(line, item));
    QVERIFY(item.filepath == "18600518_tree_v0.asc");
    QCOMPARE(item.clockWise, true);
    QCOMPARE(item.radians, false);
    QVERIFY(item.scannerPosition == Eigen::Vector3d(1, 2, 3));
    QVERIFY(item.scannerResolution == Eigen::Vector2d(0.036, 0.036));
    QVERIFY(item.scannerThetaRange == Eigen::Vector2d(0, 180));
    QVERIFY(item.scannerPhiRange == Eigen::Vector2d(60, 120));
}

void LoadsceneTest::testParseList()
{
    QList<ConfItem> conf;
    QTextStream data(simple_scene);
    LoadFileConfigUtil::loadInData(data, conf);
    QCOMPARE(conf.size(), 5);
}

void LoadsceneTest::testParseFile()
{
    QString path(SRCDIR "/ok.in");
    QList<ConfItem> conf;
    LoadFileConfigUtil::loadInFile(path, conf);
    QCOMPARE(conf.size(), 2);

    QString exp(SRCDIR "/file1.asc");
    QVERIFY(conf.at(0).filepath == exp);
}

QTEST_APPLESS_MAIN(LoadsceneTest)

#include "tst_loadscenetest.moc"
