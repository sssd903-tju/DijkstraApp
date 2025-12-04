#ifndef GRAPHDATABASE_H
#define GRAPHDATABASE_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QStringList>
#include <QDateTime>
#include <QMap>
#include <QPointF>

class Dijkstra;

// 数据表格信息
struct GraphTableInfo
{
    QString tableName;
    QString displayName;
    QDateTime createTime;
    int nodeCount;
    int edgeCount;
};

class GraphDatabase : public QObject
{
    Q_OBJECT

public:
    explicit GraphDatabase(QObject *parent = nullptr);
    ~GraphDatabase();

    bool initialize(const QString &dbPath);
    
    // 表格管理
    QStringList getAllTableNames() const;
    QList<GraphTableInfo> getAllTableInfos() const;
    bool createNewTable(const QString &tableName, const QString &displayName = QString());
    bool deleteTable(const QString &tableName);
    bool setCurrentTable(const QString &tableName);
    QString currentTable() const { return m_currentTable; }
    
    // 图数据操作（操作当前表格）
    bool loadGraph(Dijkstra *graph, const QString &tableName = QString());
    bool saveGraph(Dijkstra *graph, const QString &tableName = QString());
    bool addOrUpdateNode(long id, const QString &label, const QString &tableName = QString());
    bool addOrUpdateEdge(long id1, long id2, long distance, const QString &tableName = QString());
    bool clear(const QString &tableName = QString());

    // 布局（节点位置）持久化
    bool saveLayout(const QMap<long, QPointF> &positions, const QString &tableName = QString());
    bool loadLayout(QMap<long, QPointF> &positions, const QString &tableName = QString());
    
    QString databasePath() const { return m_dbPath; }
    QString lastError() const { return m_lastError; }

signals:
    void tableChanged(const QString &tableName);

private:
    bool ensureOpen() const;
    bool createMetaTables();
    bool createGraphTables(const QString &tableName);
    QString getNodesTableName(const QString &tableName) const;
    QString getEdgesTableName(const QString &tableName) const;
    QString getPositionsTableName(const QString &tableName) const;
    QString sanitizeTableName(const QString &name) const;
    void updateTableInfo(const QString &tableName);
    
    mutable QSqlDatabase m_db;
    QString m_connectionName;
    QString m_dbPath;
    mutable QString m_lastError;
    QString m_currentTable;  // 当前使用的表格名
};

#endif // GRAPHDATABASE_H

