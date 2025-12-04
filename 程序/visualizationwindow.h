#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QPair>

class Dijkstra;
class GraphDatabase;
class GraphView;
class QLabel;
class QTextEdit;
class QPushButton;
class QLineEdit;
class QGroupBox;
class QProgressBar;

// 自定义图形节点
class GraphNode : public QGraphicsItem
{
public:
    GraphNode(long id, const QString &label, QGraphicsItem *parent = nullptr);
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    long nodeID() const { return m_id; }
    QString label() const { return m_label; }
    void setLabel(const QString &label);
    
    void setHighlighted(bool highlighted);
    void setPathNode(bool isPath);
    void setVisited(bool visited);
    void setCurrent(bool current);
    
    enum { Type = UserType + 1 };
    int type() const override { return Type; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    long m_id;
    QString m_label;
    bool m_highlighted;
    bool m_isPathNode;
    bool m_visited;
    bool m_current;
    static const int NODE_RADIUS = 20;
};

// 自定义图形边
class GraphEdge : public QGraphicsItem
{
public:
    GraphEdge(GraphNode *source, GraphNode *dest, long distance, QGraphicsItem *parent = nullptr);
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    GraphNode *sourceNode() const { return m_source; }
    GraphNode *destNode() const { return m_dest; }
    long distance() const { return m_distance; }
    
    void setHighlighted(bool highlighted);
    void setPathEdge(bool isPath);
    
    enum { Type = UserType + 2 };
    int type() const override { return Type; }

private:
    GraphNode *m_source;
    GraphNode *m_dest;
    long m_distance;
    bool m_highlighted;
    bool m_isPathEdge;
    
    QPointF sourcePoint() const;
    QPointF destPoint() const;
};

class VisualizationWindow : public QMainWindow
{
    Q_OBJECT

public:
    VisualizationWindow(Dijkstra *dijkstra, GraphDatabase *database, QWidget *parent = nullptr);
    ~VisualizationWindow();
    
    static const int MAX_VISUALIZATION_NODES = 500; // 最大可视化节点数

public slots:
    void updateGraph();
    void highlightPath(const QVector<long> &path);
    void startAnimation(long startNode, long endNode);
    void onNodeLabelChanged();
    void onLayoutChanged();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void onQuickCalculate();
    void onQuickAddEdge();
    void showCalculationResult(long startId, long endId, long distance, const QVector<long> &path);
    void onShowHelp();

signals:
    void graphDataChanged();
    void calcResultReady(const QString &resultText);

private slots:
    void onAnimationStep();
    void onAnimationFinished();
    void onSelectionChanged();
    void onSaveLayoutToDatabase();
    void onCalcInputChanged();

private:
    void setupUI();
    void createGraph();
    void clearGraph();
    void updateStatistics();
    void autoLayout();
    QPointF calculateNodePosition(int index, int total);
    
    Dijkstra *m_dijkstra;
    GraphDatabase *m_database;
    QGraphicsScene *m_scene;
    GraphView *m_view;
    
    // 图形元素
    QMap<long, GraphNode*> m_nodes;
    QList<GraphEdge*> m_edges;
    // 记录用户自定义的节点布局位置（按节点ID记忆）
    QMap<long, QPointF> m_nodePositions;
    
    // 动画相关
    QTimer *m_animationTimer;
    QVector<long> m_animationPath;
    int m_animationIndex;
    QSet<long> m_visitedNodes;
    QSet<long> m_pathNodes;
    QSet<QPair<long, long>> m_pathEdges;
    
    // 布局保存定时器（延迟保存，避免频繁写入数据库）
    QTimer *m_layoutSaveTimer;
    
    // UI组件
    QGroupBox *m_statsGroup;
    QTextEdit *m_statsText;
    QGroupBox *m_labelGroup;
    QLineEdit *m_editNodeID;
    QLineEdit *m_editNodeLabel;
    QPushButton *m_btnSetLabel;
    QPushButton *m_btnAutoLayout;
    QPushButton *m_btnClearHighlight;
    QPushButton *m_btnZoomIn;
    QPushButton *m_btnZoomOut;
    QPushButton *m_btnZoomReset;
    QProgressBar *m_animationProgress;
    QLabel *m_animationStatus;
    QLineEdit *m_calcStartEdit;
    QLineEdit *m_calcEndEdit;
    QLineEdit *m_quickNode1Edit;
    QLineEdit *m_quickNode1LabelEdit;
    QLineEdit *m_quickNode2Edit;
    QLineEdit *m_quickNode2LabelEdit;
    QLineEdit *m_quickDistanceEdit;
    QPushButton *m_btnQuickCalculate;
    QPushButton *m_btnQuickAdd;
    QPushButton *m_btnHelp;
};

#endif // VISUALIZATIONWINDOW_H

