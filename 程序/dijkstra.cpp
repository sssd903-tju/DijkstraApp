#include "dijkstra.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <algorithm>
#include <cmath>

const long Dijkstra::MAX_DISTANCE = 999999999;

Dijkstra::Dijkstra()
    : m_nodesCount(0)
    , m_indexStart(0)
{
    m_nodes.append(NodeInfo());
}

Dijkstra::~Dijkstra()
{
    clear();
}

bool Dijkstra::loadFileData(const QString &fileName, std::function<void(float)> progressCallback)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_errorDescription = QString("无法打开文件: %1").arg(fileName);
        return false;
    }

    // 获取文件大小用于进度计算
    qint64 fileSize = file.size();
    qint64 bytesRead = 0;

    // 清空现有数据
    clear();

    QTextStream in(&file);
    int lineNumber = 0;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        lineNumber++;
        bytesRead += line.toUtf8().size() + 1; // 估算读取的字节数

        if (line.isEmpty())
            continue;

        // 分割行（支持空格、制表符、逗号、分号）
        QStringList parts;
        if (line.contains('\t'))
            parts = line.split('\t', Qt::SkipEmptyParts);
        else if (line.contains(','))
            parts = line.split(',', Qt::SkipEmptyParts);
        else if (line.contains(';'))
            parts = line.split(';', Qt::SkipEmptyParts);
        else
        {
            parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() < 3)
            {
                parts = line.split('\t', Qt::SkipEmptyParts);
            }
        }

        if (parts.size() < 3)
        {
            m_errorDescription = QString("第 %1 行格式错误：需要至少3个字段（节点1ID 节点2ID 距离值）").arg(lineNumber);
            file.close();
            return false;
        }

        bool ok1, ok2, ok3;
        long id1 = parts[0].toLong(&ok1);
        long id2 = parts[1].toLong(&ok2);
        long dist = parts[2].toLong(&ok3);

        if (!ok1 || !ok2 || !ok3)
        {
            m_errorDescription = QString("第 %1 行数据格式错误：无法解析数字").arg(lineNumber);
            file.close();
            return false;
        }

        if (!addNodesDist(id1, id2, dist))
        {
            file.close();
            return false;
        }

        // 每1000行更新一次进度
        if (lineNumber % 1000 == 0 && progressCallback)
        {
            float progress = fileSize > 0 ? (float)bytesRead / fileSize : 0.0f;
            progressCallback(progress);
        }
    }

    file.close();

    if (progressCallback)
    {
        progressCallback(1.0f);
    }

    return true;
}

bool Dijkstra::addNodesDist(long idNode1, long idNode2, long distance)
{
    // 获取或创建节点索引
    int index1, index2;

    // 处理节点1
    if (m_idToIndex.contains(idNode1))
    {
        index1 = m_idToIndex[idNode1];
    }
    else
    {
        // 创建新节点
        m_nodesCount++;
        index1 = m_nodesCount;
        m_nodes.append(NodeInfo());
        m_nodes[index1].id = idNode1;
        m_nodes[index1].label = QString::number(idNode1); // 默认标签为ID
        m_idToIndex[idNode1] = index1;
    }

    // 处理节点2
    if (m_idToIndex.contains(idNode2))
    {
        index2 = m_idToIndex[idNode2];
    }
    else
    {
        // 创建新节点
        m_nodesCount++;
        index2 = m_nodesCount;
        m_nodes.append(NodeInfo());
        m_nodes[index2].id = idNode2;
        m_nodes[index2].label = QString::number(idNode2); // 默认标签为ID
        m_idToIndex[idNode2] = index2;
    }

    // 添加边（双向）
    if (m_nodes[index1].edges.contains(index2))
    {
        if (m_nodes[index1].edges[index2] != distance)
        {
            m_errorDescription = QString("节点 %1 和节点 %2 之间存在冲突的距离值: %3 和 %4")
                .arg(idNode1).arg(idNode2)
                .arg(m_nodes[index1].edges[index2])
                .arg(distance);
            return false;
        }
    }
    else
    {
        m_nodes[index1].edges[index2] = distance;
    }

    if (m_nodes[index2].edges.contains(index1))
    {
        if (m_nodes[index2].edges[index1] != distance)
        {
            m_errorDescription = QString("节点 %1 和节点 %2 之间存在冲突的距离值: %3 和 %4")
                .arg(idNode2).arg(idNode1)
                .arg(m_nodes[index2].edges[index1])
                .arg(distance);
            return false;
        }
    }
    else
    {
        m_nodes[index2].edges[index1] = distance;
    }

    // 图结构改变，重置计算状态，强制下次重新计算
    m_indexStart = 0;

    return true;
}

void Dijkstra::setNodeLabel(long idNode, const QString &label)
{
    if (m_idToIndex.contains(idNode))
    {
        int index = m_idToIndex[idNode];
        m_nodes[index].label = label.isEmpty() ? QString::number(idNode) : label;
    }
}

QString Dijkstra::getNodeLabel(long idNode) const
{
    if (m_idToIndex.contains(idNode))
    {
        int index = m_idToIndex[idNode];
        return m_nodes[index].label;
    }
    return QString();
}

int Dijkstra::getDistance(long idNodeStart, long idNodeEnd, long &distance, QVector<long> &path, 
                          AnimationCallback animCallback)
{
    path.clear();

    if (!m_idToIndex.contains(idNodeStart))
    {
        m_errorDescription = QString("未找到起始节点: %1").arg(idNodeStart);
        return 0;
    }
    int iStart = m_idToIndex[idNodeStart];

    if (!m_idToIndex.contains(idNodeEnd))
    {
        m_errorDescription = QString("未找到终止节点: %1").arg(idNodeEnd);
        return 0;
    }
    int iEnd = m_idToIndex[idNodeEnd];

    if (iStart == iEnd)
    {
        distance = 0;
        path.append(idNodeStart);
        if (animCallback)
            animCallback(iStart, 0, true);
        return 1;
    }

    // 计算最短路径
    if (m_indexStart != iStart)
    {
        if (!calculate(idNodeStart, animCallback))
            return 0;
    }

    // 从终止节点回溯路径
    QVector<int> pathIndices;
    int current = iEnd;

    while (current != iStart && pathIndices.size() < m_nodesCount)
    {
        pathIndices.append(current);

        if (m_nodes[current].parents.isEmpty())
        {
            distance = MAX_DISTANCE;
            return -1;
        }

        current = m_nodes[current].parents.first();
    }

    if (current != iStart)
    {
        m_errorDescription = "无法回溯到起始节点";
        distance = 0;
        return 0;
    }

    // 构建路径
    path.append(idNodeStart);
    for (int i = pathIndices.size() - 1; i >= 0; i--)
    {
        path.append(m_nodes[pathIndices[i]].id);
    }

    distance = m_nodes[iEnd].distance;
    return path.size();
}

bool Dijkstra::calculate(long idNodeStart, AnimationCallback animCallback)
{
    if (m_nodesCount == 0)
    {
        m_errorDescription = "没有节点数据";
        return false;
    }

    if (!m_idToIndex.contains(idNodeStart))
    {
        m_errorDescription = QString("未找到起始节点: %1").arg(idNodeStart);
        return false;
    }
    int iStart = m_idToIndex[idNodeStart];
    
    // 如果起始节点改变或图结构改变，重置计算状态
    if (m_indexStart != iStart)
    {
        m_indexStart = 0; // 强制重新计算
    }

    // 初始化所有节点
    for (int i = 1; i <= m_nodesCount; i++)
    {
        m_nodes[i].distance = MAX_DISTANCE;
        m_nodes[i].visited = false;
        m_nodes[i].parents.clear();
    }

    // 设置起始节点
    m_nodes[iStart].visited = true;
    m_nodes[iStart].distance = 0;
    if (animCallback)
        animCallback(iStart, 0, false);

    // 初始化起始节点的邻接节点
    QList<int> unvisitedNodes;
    for (auto it = m_nodes[iStart].edges.begin(); it != m_nodes[iStart].edges.end(); ++it)
    {
        int adjIndex = it.key();
        long edgeDist = it.value();
        m_nodes[adjIndex].distance = edgeDist;
        m_nodes[adjIndex].parents.append(iStart);
        unvisitedNodes.append(adjIndex);
        if (animCallback)
            animCallback(adjIndex, edgeDist, false);
    }

    // Dijkstra主循环
    while (!unvisitedNodes.isEmpty())
    {
        // 找到未访问节点中距离最小的
        int minIndex = -1;
        long minDist = MAX_DISTANCE;
        int minPos = -1;

        for (int i = 0; i < unvisitedNodes.size(); i++)
        {
            int idx = unvisitedNodes[i];
            if (!m_nodes[idx].visited && m_nodes[idx].distance < minDist)
            {
                minDist = m_nodes[idx].distance;
                minIndex = idx;
                minPos = i;
            }
        }

        if (minIndex == -1)
            break;

        // 标记为已访问
        m_nodes[minIndex].visited = true;
        unvisitedNodes.removeAt(minPos);
        if (animCallback)
            animCallback(minIndex, m_nodes[minIndex].distance, false);

        // 更新邻接节点
        for (auto it = m_nodes[minIndex].edges.begin(); it != m_nodes[minIndex].edges.end(); ++it)
        {
            int adjIndex = it.key();
            long edgeDist = it.value();

            if (m_nodes[adjIndex].visited)
                continue;

            long newDist = m_nodes[minIndex].distance + edgeDist;

            if (newDist < m_nodes[adjIndex].distance)
            {
                m_nodes[adjIndex].distance = newDist;
                m_nodes[adjIndex].parents.clear();
                m_nodes[adjIndex].parents.append(minIndex);

                if (!unvisitedNodes.contains(adjIndex))
                    unvisitedNodes.append(adjIndex);
                
                if (animCallback)
                    animCallback(adjIndex, newDist, false);
            }
            else if (newDist == m_nodes[adjIndex].distance)
            {
                if (!m_nodes[adjIndex].parents.contains(minIndex))
                    m_nodes[adjIndex].parents.append(minIndex);
            }
        }
    }

    m_indexStart = iStart;
    if (animCallback)
        animCallback(iStart, 0, true);
    return true;
}

int Dijkstra::nodeCount() const
{
    return m_nodesCount;
}

int Dijkstra::nodeIndex(long idNode) const
{
    if (m_idToIndex.contains(idNode))
        return m_idToIndex[idNode];
    return 0;
}

long Dijkstra::nodeID(int idxNode) const
{
    if (idxNode >= 1 && idxNode <= m_nodesCount)
        return m_nodes[idxNode].id;
    return 0;
}

QMap<long, long> Dijkstra::getNodeNeighbors(long idNode) const
{
    QMap<long, long> neighbors;
    if (m_idToIndex.contains(idNode))
    {
        int index = m_idToIndex[idNode];
        for (auto it = m_nodes[index].edges.begin(); it != m_nodes[index].edges.end(); ++it)
        {
            long neighborID = m_nodes[it.key()].id;
            neighbors[neighborID] = it.value();
        }
    }
    return neighbors;
}

QVector<long> Dijkstra::getAllNodeIDs() const
{
    QVector<long> ids;
    for (int i = 1; i <= m_nodesCount; i++)
    {
        ids.append(m_nodes[i].id);
    }
    return ids;
}

Dijkstra::GraphStats Dijkstra::getGraphStats() const
{
    GraphStats stats;
    stats.nodeCount = m_nodesCount;
    stats.edgeCount = 0;
    stats.totalDistance = 0;
    int totalDegree = 0;
    stats.maxDegree = 0;
    stats.minDegree = m_nodesCount > 0 ? INT_MAX : 0;

    for (int i = 1; i <= m_nodesCount; i++)
    {
        int degree = m_nodes[i].edges.size();
        stats.edgeCount += degree;
        totalDegree += degree;
        if (degree > stats.maxDegree)
            stats.maxDegree = degree;
        if (degree < stats.minDegree)
            stats.minDegree = degree;

        for (auto it = m_nodes[i].edges.begin(); it != m_nodes[i].edges.end(); ++it)
        {
            stats.totalDistance += it.value();
        }
    }

    stats.edgeCount /= 2; // 无向图，每条边计算了两次
    stats.totalDistance /= 2;
    stats.avgDegree = m_nodesCount > 0 ? (double)totalDegree / m_nodesCount : 0.0;

    return stats;
}

void Dijkstra::clear()
{
    m_nodes.clear();
    m_nodes.append(NodeInfo());
    m_idToIndex.clear();
    m_nodesCount = 0;
    m_indexStart = 0;
    m_errorDescription.clear();
}

