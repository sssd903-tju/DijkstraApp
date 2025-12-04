#include "graphdatabase.h"
#include "dijkstra.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDir>
#include <QDebug>
#include <QtGlobal>
#include <QDateTime>

GraphDatabase::GraphDatabase(QObject *parent)
    : QObject(parent)
{
    m_connectionName = QStringLiteral("graph_connection_%1").arg(reinterpret_cast<quintptr>(this));
}

GraphDatabase::~GraphDatabase()
{
    if (m_db.isValid())
    {
        m_db.close();
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool GraphDatabase::initialize(const QString &dbPath)
{
    m_dbPath = dbPath;
    if (QSqlDatabase::contains(m_connectionName))
        m_db = QSqlDatabase::database(m_connectionName);
    else
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);

    m_db.setDatabaseName(dbPath);
    if (!ensureOpen())
        return false;

    return createMetaTables();
}

bool GraphDatabase::ensureOpen() const
{
    if (m_db.isOpen())
        return true;

    if (!m_db.open())
    {
        m_lastError = m_db.lastError().text();
        return false;
    }
    return true;
}

QString GraphDatabase::sanitizeTableName(const QString &name) const
{
    // 清理表名，只保留字母数字和下划线
    QString result;
    for (QChar c : name)
    {
        if (c.isLetterOrNumber() || c == '_')
            result += c;
        else
            result += '_';
    }
    if (result.isEmpty())
        result = "table";
    return result;
}

QString GraphDatabase::getNodesTableName(const QString &tableName) const
{
    QString tname = tableName.isEmpty() ? m_currentTable : tableName;
    if (tname.isEmpty())
        return QStringLiteral("nodes");
    return QStringLiteral("nodes_%1").arg(sanitizeTableName(tname));
}

QString GraphDatabase::getEdgesTableName(const QString &tableName) const
{
    QString tname = tableName.isEmpty() ? m_currentTable : tableName;
    if (tname.isEmpty())
        return QStringLiteral("edges");
    return QStringLiteral("edges_%1").arg(sanitizeTableName(tname));
}

QString GraphDatabase::getPositionsTableName(const QString &tableName) const
{
    QString tname = tableName.isEmpty() ? m_currentTable : tableName;
    if (tname.isEmpty())
        return QStringLiteral("positions");
    return QStringLiteral("positions_%1").arg(sanitizeTableName(tname));
}

bool GraphDatabase::createMetaTables()
{
    QSqlQuery query(m_db);
    
    // 创建表格元信息表
    if (!query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS graph_tables ("
        "table_name TEXT PRIMARY KEY,"
        "display_name TEXT,"
        "create_time TEXT,"
        "node_count INTEGER DEFAULT 0,"
        "edge_count INTEGER DEFAULT 0)"
    )))
    {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool GraphDatabase::createGraphTables(const QString &tableName)
{
    if (tableName.isEmpty())
        return false;
        
    QString nodesTable = getNodesTableName(tableName);
    QString edgesTable = getEdgesTableName(tableName);
    QString positionsTable = getPositionsTableName(tableName);
    
    QSqlQuery query(m_db);
    
    // 创建节点表
    QString nodesSQL = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "id INTEGER PRIMARY KEY,"
        "label TEXT)"
    ).arg(nodesTable);
    
    if (!query.exec(nodesSQL))
    {
        m_lastError = query.lastError().text();
        return false;
    }

    // 创建边表
    QString edgesSQL = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "id1 INTEGER,"
        "id2 INTEGER,"
        "distance INTEGER,"
        "PRIMARY KEY (id1, id2))"
    ).arg(edgesTable);
    
    if (!query.exec(edgesSQL))
    {
        m_lastError = query.lastError().text();
        return false;
    }

    // 创建节点位置表（用于保存布局）
    QString posSQL = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "id INTEGER PRIMARY KEY,"
        "x REAL,"
        "y REAL)"
    ).arg(positionsTable);

    if (!query.exec(posSQL))
    {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

QStringList GraphDatabase::getAllTableNames() const
{
    QStringList result;
    if (!ensureOpen())
        return result;
    
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT table_name FROM graph_tables ORDER BY create_time DESC")))
    {
        return result;
    }
    
    while (query.next())
    {
        result.append(query.value(0).toString());
    }
    
    return result;
}

QList<GraphTableInfo> GraphDatabase::getAllTableInfos() const
{
    QList<GraphTableInfo> result;
    if (!ensureOpen())
        return result;
    
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT table_name, display_name, create_time, node_count, edge_count FROM graph_tables ORDER BY create_time DESC")))
    {
        return result;
    }
    
    while (query.next())
    {
        GraphTableInfo info;
        info.tableName = query.value(0).toString();
        info.displayName = query.value(1).toString();
        info.createTime = QDateTime::fromString(query.value(2).toString(), Qt::ISODate);
        info.nodeCount = query.value(3).toInt();
        info.edgeCount = query.value(4).toInt();
        result.append(info);
    }
    
    return result;
}

bool GraphDatabase::createNewTable(const QString &tableName, const QString &displayName)
{
    if (tableName.isEmpty())
        return false;
        
    if (!ensureOpen())
        return false;
    
    QString sanitized = sanitizeTableName(tableName);
    
    // 检查表格是否已存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM graph_tables WHERE table_name = ?"));
    checkQuery.bindValue(0, sanitized);
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0)
    {
        m_lastError = QString("表格 %1 已存在").arg(sanitized);
        return false;
    }
    
    // 创建表格结构
    if (!createGraphTables(sanitized))
        return false;
    
    // 插入元信息
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare(QStringLiteral(
        "INSERT INTO graph_tables (table_name, display_name, create_time) VALUES (?, ?, ?)"
    ));
    insertQuery.bindValue(0, sanitized);
    insertQuery.bindValue(1, displayName.isEmpty() ? sanitized : displayName);
    insertQuery.bindValue(2, QDateTime::currentDateTime().toString(Qt::ISODate));
    
    if (!insertQuery.exec())
    {
        m_lastError = insertQuery.lastError().text();
        return false;
    }
    
    return true;
}

bool GraphDatabase::deleteTable(const QString &tableName)
{
    if (tableName.isEmpty())
        return false;
        
    if (!ensureOpen())
        return false;
    
    QString sanitized = sanitizeTableName(tableName);
    
    // 删除表格结构
    QString nodesTable = getNodesTableName(sanitized);
    QString edgesTable = getEdgesTableName(sanitized);
    QString positionsTable = getPositionsTableName(sanitized);
    
    QSqlQuery query(m_db);
    query.exec(QStringLiteral("DROP TABLE IF EXISTS %1").arg(nodesTable));
    query.exec(QStringLiteral("DROP TABLE IF EXISTS %1").arg(edgesTable));
    query.exec(QStringLiteral("DROP TABLE IF EXISTS %1").arg(positionsTable));
    
    // 删除元信息
    query.prepare(QStringLiteral("DELETE FROM graph_tables WHERE table_name = ?"));
    query.bindValue(0, sanitized);
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    
    if (m_currentTable == sanitized)
        m_currentTable.clear();
    
    return true;
}

bool GraphDatabase::setCurrentTable(const QString &tableName)
{
    QString sanitized = sanitizeTableName(tableName);
    
    // 检查表格是否存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM graph_tables WHERE table_name = ?"));
    checkQuery.bindValue(0, sanitized);
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() == 0)
    {
        m_lastError = QString("表格 %1 不存在").arg(sanitized);
        return false;
    }
    
    m_currentTable = sanitized;
    emit tableChanged(m_currentTable);
    return true;
}

void GraphDatabase::updateTableInfo(const QString &tableName)
{
    if (tableName.isEmpty())
        return;
        
    QString sanitized = sanitizeTableName(tableName);
    QString nodesTable = getNodesTableName(sanitized);
    QString edgesTable = getEdgesTableName(sanitized);
    
    QSqlQuery query(m_db);
    
    // 统计节点数
    query.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(nodesTable));
    int nodeCount = 0;
    if (query.next())
        nodeCount = query.value(0).toInt();
    
    // 统计边数
    query.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(edgesTable));
    int edgeCount = 0;
    if (query.next())
        edgeCount = query.value(0).toInt();
    
    // 更新元信息
    query.prepare(QStringLiteral("UPDATE graph_tables SET node_count = ?, edge_count = ? WHERE table_name = ?"));
    query.bindValue(0, nodeCount);
    query.bindValue(1, edgeCount);
    query.bindValue(2, sanitized);
    query.exec();
}

bool GraphDatabase::loadGraph(Dijkstra *graph, const QString &tableName)
{
    if (!ensureOpen())
        return false;

    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }

    graph->clear();

    QString nodesTable = getNodesTableName(tname);
    QString edgesTable = getEdgesTableName(tname);

    // 加载边
    QSqlQuery edgeQuery(m_db);
    QString edgeSQL = QStringLiteral("SELECT id1, id2, distance FROM %1").arg(edgesTable);
    if (!edgeQuery.exec(edgeSQL))
    {
        m_lastError = edgeQuery.lastError().text();
        return false;
    }

    while (edgeQuery.next())
    {
        long id1 = edgeQuery.value(0).toLongLong();
        long id2 = edgeQuery.value(1).toLongLong();
        long distance = edgeQuery.value(2).toLongLong();
        graph->addNodesDist(id1, id2, distance);
    }

    // 加载节点标签
    QSqlQuery nodeQuery(m_db);
    QString nodeSQL = QStringLiteral("SELECT id, label FROM %1").arg(nodesTable);
    if (!nodeQuery.exec(nodeSQL))
    {
        m_lastError = nodeQuery.lastError().text();
        return false;
    }

    while (nodeQuery.next())
    {
        long id = nodeQuery.value(0).toLongLong();
        QString label = nodeQuery.value(1).toString();
        graph->setNodeLabel(id, label);
    }

    return true;
}

bool GraphDatabase::saveGraph(Dijkstra *graph, const QString &tableName)
{
    if (!ensureOpen())
        return false;

    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }

    QString nodesTable = getNodesTableName(tname);
    QString edgesTable = getEdgesTableName(tname);

    if (!m_db.transaction())
    {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    
    // 清空现有数据
    if (!query.exec(QStringLiteral("DELETE FROM %1").arg(nodesTable)))
    {
        m_lastError = query.lastError().text();
        m_db.rollback();
        return false;
    }
    if (!query.exec(QStringLiteral("DELETE FROM %1").arg(edgesTable)))
    {
        m_lastError = query.lastError().text();
        m_db.rollback();
        return false;
    }

    // 插入节点
    QString nodeInsertSQL = QStringLiteral("INSERT OR REPLACE INTO %1(id, label) VALUES(?, ?)").arg(nodesTable);
    query.prepare(nodeInsertSQL);
    QVector<long> nodeIds = graph->getAllNodeIDs();
    for (long id : nodeIds)
    {
        query.bindValue(0, QVariant::fromValue(id));
        query.bindValue(1, graph->getNodeLabel(id));
        if (!query.exec())
        {
            m_lastError = query.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    // 插入边
    QString edgeInsertSQL = QStringLiteral("INSERT OR REPLACE INTO %1(id1, id2, distance) VALUES(?, ?, ?)").arg(edgesTable);
    QSqlQuery edgeInsert(m_db);
    edgeInsert.prepare(edgeInsertSQL);
    for (long id : nodeIds)
    {
        QMap<long, long> neighbors = graph->getNodeNeighbors(id);
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            long id2 = it.key();
            long distance = it.value();
            long a = qMin(id, id2);
            long b = qMax(id, id2);

            edgeInsert.bindValue(0, QVariant::fromValue(a));
            edgeInsert.bindValue(1, QVariant::fromValue(b));
            edgeInsert.bindValue(2, QVariant::fromValue(distance));
            if (!edgeInsert.exec())
            {
                m_lastError = edgeInsert.lastError().text();
                m_db.rollback();
                return false;
            }
        }
    }

    bool result = m_db.commit();
    if (result)
    {
        updateTableInfo(tname);
    }
    return result;
}

bool GraphDatabase::addOrUpdateNode(long id, const QString &label, const QString &tableName)
{
    if (!ensureOpen())
        return false;

    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }

    QString nodesTable = getNodesTableName(tname);
    QSqlQuery query(m_db);
    QString sql = QStringLiteral("INSERT OR REPLACE INTO %1(id, label) VALUES(?, ?)").arg(nodesTable);
    query.prepare(sql);
    query.bindValue(0, QVariant::fromValue(id));
    query.bindValue(1, label);
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    
    updateTableInfo(tname);
    return true;
}

bool GraphDatabase::addOrUpdateEdge(long id1, long id2, long distance, const QString &tableName)
{
    if (!ensureOpen())
        return false;

    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }

    QString edgesTable = getEdgesTableName(tname);
    long a = qMin(id1, id2);
    long b = qMax(id1, id2);
    QSqlQuery query(m_db);
    QString sql = QStringLiteral("INSERT OR REPLACE INTO %1(id1, id2, distance) VALUES(?, ?, ?)").arg(edgesTable);
    query.prepare(sql);
    query.bindValue(0, QVariant::fromValue(a));
    query.bindValue(1, QVariant::fromValue(b));
    query.bindValue(2, QVariant::fromValue(distance));
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    
    updateTableInfo(tname);
    return true;
}

bool GraphDatabase::clear(const QString &tableName)
{
    if (!ensureOpen())
        return false;
        
    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }
    
    QString nodesTable = getNodesTableName(tname);
    QString edgesTable = getEdgesTableName(tname);
    QString positionsTable = getPositionsTableName(tname);
    
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("DELETE FROM %1").arg(edgesTable)))
    {
        m_lastError = query.lastError().text();
        return false;
    }
    if (!query.exec(QStringLiteral("DELETE FROM %1").arg(nodesTable)))
    {
        m_lastError = query.lastError().text();
        return false;
    }
    if (!query.exec(QStringLiteral("DELETE FROM %1").arg(positionsTable)))
    {
        m_lastError = query.lastError().text();
        return false;
    }
    
    updateTableInfo(tname);
    return true;
}

bool GraphDatabase::saveLayout(const QMap<long, QPointF> &positions, const QString &tableName)
{
    if (!ensureOpen())
        return false;

    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }

    QString posTable = getPositionsTableName(tname);
    QSqlQuery query(m_db);

    // 确保位置表存在（如果不存在则创建）
    QString createPosSQL = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "id INTEGER PRIMARY KEY,"
        "x REAL NOT NULL,"
        "y REAL NOT NULL)"
    ).arg(posTable);
    
    if (!query.exec(createPosSQL))
    {
        m_lastError = QString("创建位置表失败: %1").arg(query.lastError().text());
        return false;
    }

    if (!m_db.transaction())
    {
        m_lastError = m_db.lastError().text();
        return false;
    }

    // 清空旧布局
    if (!query.exec(QStringLiteral("DELETE FROM %1").arg(posTable)))
    {
        m_lastError = query.lastError().text();
        m_db.rollback();
        return false;
    }

    // 插入新布局
    QString insertSQL = QStringLiteral("INSERT OR REPLACE INTO %1(id, x, y) VALUES(?, ?, ?)").arg(posTable);
    query.prepare(insertSQL);
    for (auto it = positions.constBegin(); it != positions.constEnd(); ++it)
    {
        query.bindValue(0, QVariant::fromValue(it.key()));
        query.bindValue(1, it.value().x());
        query.bindValue(2, it.value().y());
        if (!query.exec())
        {
            m_lastError = query.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    return m_db.commit();
}

bool GraphDatabase::loadLayout(QMap<long, QPointF> &positions, const QString &tableName)
{
    positions.clear();

    if (!ensureOpen())
        return false;

    QString tname = tableName.isEmpty() ? m_currentTable : sanitizeTableName(tableName);
    if (tname.isEmpty())
    {
        m_lastError = "未指定表格";
        return false;
    }

    QString posTable = getPositionsTableName(tname);
    QSqlQuery query(m_db);
    QString sql = QStringLiteral("SELECT id, x, y FROM %1").arg(posTable);
    if (!query.exec(sql))
    {
        // 如果位置表不存在或查询失败，就视为“尚未保存布局”，直接返回 false 不报错
        return false;
    }

    while (query.next())
    {
        long id = query.value(0).toLongLong();
        double x = query.value(1).toDouble();
        double y = query.value(2).toDouble();
        positions.insert(id, QPointF(x, y));
    }

    return true;
}
