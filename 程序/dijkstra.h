#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QList>
#include <functional>
#include <limits>

// 回调函数类型：用于算法执行动画
// 参数：当前访问的节点索引，当前距离，是否完成
typedef std::function<void(int, long, bool)> AnimationCallback;

// Dijkstra算法类
class Dijkstra
{
public:
    Dijkstra();
    ~Dijkstra();

    // 从文件加载数据
    bool loadFileData(const QString &fileName, std::function<void(float)> progressCallback = nullptr);

    // 手动添加节点和距离关系
    bool addNodesDist(long idNode1, long idNode2, long distance);

    // 设置节点标签/名称
    void setNodeLabel(long idNode, const QString &label);
    QString getNodeLabel(long idNode) const;

    // 计算从起始节点到终止节点的最短路径（支持动画回调）
    int getDistance(long idNodeStart, long idNodeEnd, long &distance, QVector<long> &path, 
                    AnimationCallback animCallback = nullptr);

    // 获取当前已加载的节点数量
    int nodeCount() const;

    // 根据节点ID获取节点索引
    int nodeIndex(long idNode) const;

    // 根据节点索引获取节点ID
    long nodeID(int idxNode) const;

    // 获取节点的所有邻接节点和距离
    QMap<long, long> getNodeNeighbors(long idNode) const;

    // 获取所有节点ID
    QVector<long> getAllNodeIDs() const;

    // 获取图的统计信息
    struct GraphStats {
        int nodeCount;
        int edgeCount;
        double avgDegree;
        int maxDegree;
        int minDegree;
        long totalDistance;
    };
    GraphStats getGraphStats() const;

    // 清空所有数据
    void clear();

    // 错误信息
    QString errorDescription() const { return m_errorDescription; }

private:
    // 节点信息结构
    struct NodeInfo
    {
        long id;                    // 节点ID
        QString label;              // 节点标签/名称
        QMap<int, long> edges;      // 邻接边：key=邻接节点索引，value=距离
        bool visited;               // 是否已访问
        long distance;              // 当前最短距离
        QList<int> parents;         // 父节点列表（支持多条最短路径）

        NodeInfo() : id(0), visited(false), distance(std::numeric_limits<long>::max()) {}
    };

    // 计算从起始节点开始的最短路径（支持动画回调）
    bool calculate(long idNodeStart, AnimationCallback animCallback = nullptr);

    static const long MAX_DISTANCE;  // 最大距离值

    QVector<NodeInfo> m_nodes;      // 节点数组（索引从1开始，0不使用）
    QMap<long, int> m_idToIndex;    // 节点ID到索引的映射
    int m_nodesCount;                // 节点数量
    int m_indexStart;                // 当前计算的起始节点索引
    QString m_errorDescription;      // 错误描述
};

#endif // DIJKSTRA_H

