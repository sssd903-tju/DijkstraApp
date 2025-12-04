#include "visualizationwindow.h"
#include "dijkstra.h"
#include "graphdatabase.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QProgressBar>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QScrollBar>
#include <QPair>
#include <QWheelEvent>
#include <QFormLayout>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>
#include <climits>

class GraphView : public QGraphicsView
{
public:
    explicit GraphView(QWidget *parent = nullptr) : QGraphicsView(parent)
    {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::RubberBandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorUnderMouse);
        
        // 启用触控板手势支持（Mac）
        grabGesture(Qt::PinchGesture);
        setAttribute(Qt::WA_AcceptTouchEvents, true);
    }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        // 检查是否是触控板手势（Mac上触控板滚动会触发wheelEvent）
        if (event->source() == Qt::MouseEventNotSynthesized)
        {
            // 普通鼠标滚轮
            const double factor = 1.15;
            if (event->angleDelta().y() > 0)
                scale(factor, factor);
            else
                scale(1.0 / factor, 1.0 / factor);
        }
        else
        {
            // 触控板滚动，使用更平滑的缩放
            const double factor = 1.0 + (event->angleDelta().y() / 1200.0);
            scale(factor, factor);
        }
    }
    
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::Gesture)
        {
            QGestureEvent *gestureEvent = static_cast<QGestureEvent*>(event);
            if (QPinchGesture *pinch = static_cast<QPinchGesture *>(gestureEvent->gesture(Qt::PinchGesture)))
            {
                QPinchGesture::ChangeFlags changeFlags = pinch->changeFlags();
                if (changeFlags & QPinchGesture::ScaleFactorChanged)
                {
                    qreal scaleFactor = pinch->scaleFactor();
                    // 相对于初始缩放的比例
                    static qreal lastScaleFactor = 1.0;
                    qreal deltaScale = scaleFactor / lastScaleFactor;
                    lastScaleFactor = scaleFactor;
                    
                    qreal currentScale = transform().m11();
                    qreal newScale = currentScale * deltaScale;
                    
                    // 限制缩放范围
                    if (newScale > 0.1 && newScale < 10.0)
                    {
                        scale(deltaScale, deltaScale);
                    }
                    
                    // 手势结束后重置缩放参考值
                    if (pinch->state() == Qt::GestureFinished ||
                        pinch->state() == Qt::GestureCanceled)
                    {
                        lastScaleFactor = 1.0;
                    }
                }
                return true;
            }
        }
        return QGraphicsView::event(event);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton &&
            !(event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)))
        {
            // 没有按 Ctrl/Command 时，如果点在图元上，
            // 临时加上 Ctrl 修饰键，让 Qt 按"切换选中（toggle）"处理，但仍保持拖动等默认行为
            QPoint viewPos = event->position().toPoint(); // Qt6: 使用position()
            if (itemAt(viewPos))
            {
                event->setModifiers(event->modifiers() | Qt::ControlModifier);
            }
        }
        // 统一交给基类处理：保证 RubberBand 框选、清空选中、拖动等默认行为正常
        QGraphicsView::mousePressEvent(event);
    }
};

// ==================== GraphNode 实现 ====================

GraphNode::GraphNode(long id, const QString &label, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_id(id)
    , m_label(label.isEmpty() ? QString::number(id) : label)
    , m_highlighted(false)
    , m_isPathNode(false)
    , m_visited(false)
    , m_current(false)
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setFlag(ItemIsSelectable, true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(2);
}

QRectF GraphNode::boundingRect() const
{
    return QRectF(-NODE_RADIUS - 2, -NODE_RADIUS - 2, 
                  (NODE_RADIUS + 2) * 2, (NODE_RADIUS + 2) * 2);
}

void GraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 确定颜色
    QColor fillColor;
    QColor borderColor = Qt::black;
    int borderWidth = 2;

    if (m_current)
    {
        fillColor = QColor(255, 165, 0); // 橙色 - 当前节点
        borderColor = QColor(255, 140, 0);
        borderWidth = 3;
    }
    else if (m_isPathNode)
    {
        fillColor = QColor(76, 175, 80); // 绿色 - 路径节点
        borderColor = QColor(56, 142, 60);
        borderWidth = 3;
    }
    else if (m_visited)
    {
        fillColor = QColor(33, 150, 243); // 蓝色 - 已访问
        borderColor = QColor(25, 118, 210);
    }
    else if (m_highlighted)
    {
        fillColor = QColor(255, 235, 59); // 黄色 - 高亮
        borderColor = QColor(255, 193, 7);
    }
    else
    {
        // 选中状态高亮显示
        if (isSelected())
        {
            fillColor = QColor(255, 249, 196); // 浅黄色
            borderColor = QColor(255, 193, 7);
            borderWidth = 3;
        }
        else
        {
            fillColor = QColor(224, 224, 224); // 灰色 - 默认
        }
    }

    // 绘制节点
    painter->setPen(QPen(borderColor, borderWidth));
    painter->setBrush(fillColor);
    painter->drawEllipse(-NODE_RADIUS, -NODE_RADIUS, NODE_RADIUS * 2, NODE_RADIUS * 2);

    // 绘制标签
    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 8, QFont::Bold));
    QRectF textRect = boundingRect();
    painter->drawText(textRect, Qt::AlignCenter, m_label);
}

void GraphNode::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    update();
}

void GraphNode::setPathNode(bool isPath)
{
    m_isPathNode = isPath;
    update();
}

void GraphNode::setVisited(bool visited)
{
    m_visited = visited;
    update();
}

void GraphNode::setCurrent(bool current)
{
    m_current = current;
    update();
}

void GraphNode::setLabel(const QString &label)
{
    m_label = label.isEmpty() ? QString::number(m_id) : label;
    update();
}

QVariant GraphNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged)
    {
        // 更新所有连接的边
        foreach (QGraphicsItem *item, scene()->items())
        {
            if (GraphEdge *edge = qgraphicsitem_cast<GraphEdge*>(item))
            {
                if (edge->sourceNode() == this || edge->destNode() == this)
                {
                    edge->update();
                }
            }
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void GraphNode::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) &&
        !(event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)))
    {
        // 无 Ctrl/Command：只移动当前节点，不整体拖动选中集合
        QPointF delta = event->scenePos() - event->lastScenePos();
        setPos(pos() + delta);
        event->accept();
        return;
    }
    // Ctrl/Command 下交给默认逻辑，实现多选整体拖动
    QGraphicsItem::mouseMoveEvent(event);
}

// ==================== GraphEdge 实现 ====================

GraphEdge::GraphEdge(GraphNode *source, GraphNode *dest, long distance, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_source(source)
    , m_dest(dest)
    , m_distance(distance)
    , m_highlighted(false)
    , m_isPathEdge(false)
{
    setZValue(1);
    if (m_source && m_source->scene())
    {
        m_source->scene()->addItem(this);
    }
}

QRectF GraphEdge::boundingRect() const
{
    if (!m_source || !m_dest)
        return QRectF();

    QPointF s = sourcePoint();
    QPointF d = destPoint();
    qreal penWidth = m_isPathEdge ? 4 : (m_highlighted ? 3 : 1);
    qreal extra = (penWidth + 20) / 2.0;

    return QRectF(qMin(s.x(), d.x()) - extra,
                  qMin(s.y(), d.y()) - extra,
                  qAbs(s.x() - d.x()) + 2 * extra,
                  qAbs(s.y() - d.y()) + 2 * extra);
}

void GraphEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_source || !m_dest)
        return;

    QPointF s = sourcePoint();
    QPointF d = destPoint();

    // 确定颜色和宽度
    QColor lineColor;
    int lineWidth = 1;

    if (m_isPathEdge)
    {
        lineColor = QColor(76, 175, 80); // 绿色 - 路径边
        lineWidth = 4;
    }
    else if (m_highlighted)
    {
        lineColor = QColor(255, 193, 7); // 黄色 - 高亮
        lineWidth = 3;
    }
    else
    {
        lineColor = QColor(158, 158, 158); // 灰色 - 默认
    }

    painter->setPen(QPen(lineColor, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(s, d);

    // 绘制距离标签
    QPointF midPoint = (s + d) / 2;
    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 7));
    painter->setBrush(Qt::white);
    QString distText = QString::number(m_distance);
    QRectF textRect(midPoint.x() - 15, midPoint.y() - 8, 30, 16);
    painter->drawRect(textRect);
    painter->drawText(textRect, Qt::AlignCenter, distText);
}

QPointF GraphEdge::sourcePoint() const
{
    if (!m_source) return QPointF();
    QPointF center = m_source->pos();
    QPointF dest = m_dest->pos();
    QPointF dir = dest - center;
    qreal len = sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    if (len > 0)
    {
        dir /= len;
        return center + dir * 20; // 20是节点半径
    }
    return center;
}

QPointF GraphEdge::destPoint() const
{
    if (!m_dest) return QPointF();
    QPointF center = m_dest->pos();
    QPointF source = m_source->pos();
    QPointF dir = source - center;
    qreal len = sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    if (len > 0)
    {
        dir /= len;
        return center + dir * 20;
    }
    return center;
}

void GraphEdge::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    update();
}

void GraphEdge::setPathEdge(bool isPath)
{
    m_isPathEdge = isPath;
    update();
}

// ==================== VisualizationWindow 实现 ====================

VisualizationWindow::VisualizationWindow(Dijkstra *dijkstra, GraphDatabase *database, QWidget *parent)
    : QMainWindow(parent)
    , m_dijkstra(dijkstra)
    , m_database(database)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_animationTimer(nullptr)
    , m_animationIndex(0)
    , m_layoutSaveTimer(nullptr)
    , m_calcStartEdit(nullptr)
    , m_calcEndEdit(nullptr)
    , m_quickNode1Edit(nullptr)
    , m_quickNode1LabelEdit(nullptr)
    , m_quickNode2Edit(nullptr)
    , m_quickNode2LabelEdit(nullptr)
    , m_quickDistanceEdit(nullptr)
{
    setupUI();

    // 从数据库加载已保存的布局（如果有）
    if (m_database)
    {
        if (m_database->loadLayout(m_nodePositions))
        {
            qDebug() << "从数据库加载布局成功，节点数:" << m_nodePositions.size();
        }
        else
        {
            qDebug() << "未找到已保存的布局，将使用自动布局";
        }
    }

    // 初始化布局保存定时器（每200ms检查一次并保存，确保布局及时持久化）
    m_layoutSaveTimer = new QTimer(this);
    m_layoutSaveTimer->setSingleShot(true);
    m_layoutSaveTimer->setInterval(200); // 200ms 延迟，更及时地持久化布局
    connect(m_layoutSaveTimer, &QTimer::timeout, this, &VisualizationWindow::onSaveLayoutToDatabase);

    updateGraph();
}

VisualizationWindow::~VisualizationWindow()
{
    // 关闭前立即保存当前布局到数据库，保证布局持久化
    onSaveLayoutToDatabase();
    clearGraph();
}

void VisualizationWindow::setupUI()
{
    setWindowTitle("图可视化界面");
    setMinimumSize(1000, 700);
    resize(1200, 800);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 左侧：图形视图
    QVBoxLayout *viewLayout = new QVBoxLayout();
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-500, -500, 1000, 1000);
    connect(m_scene, &QGraphicsScene::selectionChanged,
            this, &VisualizationWindow::onSelectionChanged);
    
    m_view = new GraphView(this);
    m_view->setScene(m_scene);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    viewLayout->addWidget(m_view);

    // 视图控制按钮
    QHBoxLayout *viewControlLayout = new QHBoxLayout();
    m_btnZoomIn = new QPushButton("放大", this);
    m_btnZoomOut = new QPushButton("缩小", this);
    m_btnZoomReset = new QPushButton("重置缩放", this);
    m_btnAutoLayout = new QPushButton("自动布局", this);
    m_btnClearHighlight = new QPushButton("清除高亮", this);
    m_btnHelp = new QPushButton("使用说明", this);
    viewControlLayout->addWidget(m_btnZoomIn);
    viewControlLayout->addWidget(m_btnZoomOut);
    viewControlLayout->addWidget(m_btnZoomReset);
    viewControlLayout->addWidget(m_btnAutoLayout);
    viewControlLayout->addWidget(m_btnClearHighlight);
    viewControlLayout->addWidget(m_btnHelp);
    viewControlLayout->addStretch();
    viewLayout->addLayout(viewControlLayout);

    connect(m_btnZoomIn, &QPushButton::clicked, this, &VisualizationWindow::zoomIn);
    connect(m_btnZoomOut, &QPushButton::clicked, this, &VisualizationWindow::zoomOut);
    connect(m_btnZoomReset, &QPushButton::clicked, this, &VisualizationWindow::resetZoom);
    connect(m_btnAutoLayout, &QPushButton::clicked, this, &VisualizationWindow::onLayoutChanged);
    connect(m_btnClearHighlight, &QPushButton::clicked, this, [this]() {
        highlightPath(QVector<long>());
    });
    connect(m_btnHelp, &QPushButton::clicked, this, &VisualizationWindow::onShowHelp);

    mainLayout->addLayout(viewLayout, 3);

    // 右侧：控制面板
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(10);

    // 统计信息面板
    m_statsGroup = new QGroupBox("数据统计", this);
    QVBoxLayout *statsLayout = new QVBoxLayout();
    m_statsText = new QTextEdit(this);
    m_statsText->setReadOnly(true);
    m_statsText->setMaximumHeight(200);
    statsLayout->addWidget(m_statsText);
    m_statsGroup->setLayout(statsLayout);
    controlLayout->addWidget(m_statsGroup);

    // 节点标签设置
    m_labelGroup = new QGroupBox("节点标签设置", this);
    QGridLayout *labelLayout = new QGridLayout();
    labelLayout->addWidget(new QLabel("节点ID:", this), 0, 0);
    m_editNodeID = new QLineEdit(this);
    labelLayout->addWidget(m_editNodeID, 0, 1);
    labelLayout->addWidget(new QLabel("标签名称:", this), 1, 0);
    m_editNodeLabel = new QLineEdit(this);
    labelLayout->addWidget(m_editNodeLabel, 1, 1);
    m_btnSetLabel = new QPushButton("设置标签", this);
    labelLayout->addWidget(m_btnSetLabel, 2, 0, 1, 2);
    m_labelGroup->setLayout(labelLayout);
    controlLayout->addWidget(m_labelGroup);

    connect(m_btnSetLabel, &QPushButton::clicked, this, &VisualizationWindow::onNodeLabelChanged);

    // 路径计算面板
    QGroupBox *calcGroup = new QGroupBox("路径计算", this);
    QVBoxLayout *calcLayout = new QVBoxLayout();
    QGridLayout *calcGrid = new QGridLayout();
    calcGrid->addWidget(new QLabel("起始节点:", this), 0, 0);
    m_calcStartEdit = new QLineEdit(this);
    calcGrid->addWidget(m_calcStartEdit, 0, 1);
    calcGrid->addWidget(new QLabel("终止节点:", this), 1, 0);
    m_calcEndEdit = new QLineEdit(this);
    calcGrid->addWidget(m_calcEndEdit, 1, 1);
    calcLayout->addLayout(calcGrid);
    m_btnQuickCalculate = new QPushButton("计算并高亮", this);
    calcLayout->addWidget(m_btnQuickCalculate);
    calcGroup->setLayout(calcLayout);
    controlLayout->addWidget(calcGroup);
    connect(m_btnQuickCalculate, &QPushButton::clicked, this, &VisualizationWindow::onQuickCalculate);
    // 输入变化时更新“计算并高亮”按钮状态（是否高亮为黄色）
    connect(m_calcStartEdit, &QLineEdit::textChanged, this, &VisualizationWindow::onCalcInputChanged);
    connect(m_calcEndEdit, &QLineEdit::textChanged, this, &VisualizationWindow::onCalcInputChanged);

    // 快速添加边
    QGroupBox *quickAddGroup = new QGroupBox("快速添加边", this);
    QGridLayout *addLayout = new QGridLayout();
    addLayout->addWidget(new QLabel("节点1 ID:", this), 0, 0);
    m_quickNode1Edit = new QLineEdit(this);
    addLayout->addWidget(m_quickNode1Edit, 0, 1);
    addLayout->addWidget(new QLabel("节点1标签:", this), 0, 2);
    m_quickNode1LabelEdit = new QLineEdit(this);
    addLayout->addWidget(m_quickNode1LabelEdit, 0, 3);
    addLayout->addWidget(new QLabel("节点2 ID:", this), 1, 0);
    m_quickNode2Edit = new QLineEdit(this);
    addLayout->addWidget(m_quickNode2Edit, 1, 1);
    addLayout->addWidget(new QLabel("节点2标签:", this), 1, 2);
    m_quickNode2LabelEdit = new QLineEdit(this);
    addLayout->addWidget(m_quickNode2LabelEdit, 1, 3);
    addLayout->addWidget(new QLabel("距离:", this), 2, 0);
    m_quickDistanceEdit = new QLineEdit(this);
    addLayout->addWidget(m_quickDistanceEdit, 2, 1);
    m_btnQuickAdd = new QPushButton("添加并刷新图形", this);
    addLayout->addWidget(m_btnQuickAdd, 3, 0, 1, 4);
    quickAddGroup->setLayout(addLayout);
    controlLayout->addWidget(quickAddGroup);
    connect(m_btnQuickAdd, &QPushButton::clicked, this, &VisualizationWindow::onQuickAddEdge);

    // 动画控制
    QGroupBox *animGroup = new QGroupBox("算法动画", this);
    QVBoxLayout *animLayout = new QVBoxLayout();
    m_animationProgress = new QProgressBar(this);
    m_animationStatus = new QLabel("就绪", this);
    animLayout->addWidget(m_animationStatus);
    animLayout->addWidget(m_animationProgress);
    animGroup->setLayout(animLayout);
    controlLayout->addWidget(animGroup);

    // 动画定时器
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &VisualizationWindow::onAnimationStep);

    // 监听场景变化，当节点位置变化时触发布局保存
    if (m_scene)
    {
        // 使用定时器定期检查节点位置并保存（每3秒检查一次）
        QTimer *checkTimer = new QTimer(this);
        connect(checkTimer, &QTimer::timeout, this, [this]() {
            // 检查是否有节点位置变化
            bool hasChange = false;
            QMap<long, QPointF> currentPositions;
            for (auto it = m_nodes.constBegin(); it != m_nodes.constEnd(); ++it)
            {
                if (it.value())
                {
                    QPointF currentPos = it.value()->pos();
                    currentPositions.insert(it.key(), currentPos);
                    // 如果位置与记忆中的不同，标记为有变化
                    if (!m_nodePositions.contains(it.key()) || 
                        m_nodePositions[it.key()] != currentPos)
                    {
                        hasChange = true;
                    }
                }
            }
            
            // 如果有变化，更新记忆并启动保存定时器
            if (hasChange)
            {
                m_nodePositions = currentPositions;
                if (m_layoutSaveTimer && m_database)
                {
                    m_layoutSaveTimer->stop();
                    m_layoutSaveTimer->start(); // 延迟200ms后保存，确保布局及时持久化
                }
            }
        });
        checkTimer->start(200); // 每200ms检查一次是否有节点位置变化
    }

    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout, 1);
}

void VisualizationWindow::updateGraph()
{
    // 先保存当前布局（如果有节点）
    if (!m_nodes.isEmpty())
    {
        QMap<long, QPointF> currentPositions;
        for (auto it = m_nodes.constBegin(); it != m_nodes.constEnd(); ++it)
        {
            if (it.value())
                currentPositions.insert(it.key(), it.value()->pos());
        }
        // 保存当前布局到数据库
        if (!currentPositions.isEmpty() && m_database)
        {
            m_database->saveLayout(currentPositions);
        }
    }

    // 从数据库重新加载当前表的布局
    m_nodePositions.clear();
    if (m_database)
    {
        if (m_database->loadLayout(m_nodePositions))
        {
            qDebug() << "更新图形时重新加载布局，节点数:" << m_nodePositions.size();
        }
    }

    clearGraph();
    createGraph();
    updateStatistics();
}

void VisualizationWindow::onShowHelp()
{
    QString text;
    text += "【图形可视化界面使用说明】\n";
    text += "1. 鼠标滚轮或触控板手势可以缩放视图，也可以使用“放大/缩小/重置缩放”按钮。\n";
    text += "2. 左键单击节点：选中/取消选中，可连续多选；点击空白处可清空所有选中。\n";
    text += "3. 左键拖动节点（不按 Ctrl/Command）：只移动当前节点；按住 Ctrl/Command 拖动可整体移动已选中的节点。\n";
    text += "4. 选中恰好两个节点时，右侧“路径计算”区域会自动填入 ID，按钮变为黄色，点击“计算并高亮”即可在图上显示最短路径。\n";
    text += "5. 使用“快速添加边”可以在当前图中加入新的节点关系，并同步保存到数据库。\n";
    text += "6. 当节点数量超过上限时，为保证性能，界面只显示文字提示，不进行图形化绘制。\n";
    text += "7. 布局持久化：你在图中拖动调整过的节点位置会自动保存到数据库，与当前数据表关联。下次重新打开该表的可视化界面时，会自动恢复上次的布局。\n";

    QMessageBox::information(this, "可视化界面使用说明", text);
}

void VisualizationWindow::onCalcInputChanged()
{
    if (!m_btnQuickCalculate)
        return;

    // 如果当前选中了超过两个节点，仍然禁止计算
    if (m_scene && m_scene->selectedItems().size() > 2)
    {
        m_btnQuickCalculate->setEnabled(false);
        m_btnQuickCalculate->setStyleSheet(QString());
        return;
    }

    bool ok1 = false, ok2 = false;
    m_calcStartEdit->text().toLong(&ok1);
    m_calcEndEdit->text().toLong(&ok2);

    if (ok1 && ok2)
    {
        // 可以计算：按钮启用并高亮为黄色
        m_btnQuickCalculate->setEnabled(true);
        m_btnQuickCalculate->setStyleSheet("background-color: #ffd54f; color: #333333; font-weight: bold;");
    }
    else
    {
        // 不能计算：按钮保持启用但恢复默认灰色样式
        m_btnQuickCalculate->setEnabled(true);
        m_btnQuickCalculate->setStyleSheet(QString());
    }
}

void VisualizationWindow::createGraph()
{
    if (!m_dijkstra || m_dijkstra->nodeCount() == 0)
    {
        // 如果没有数据，显示提示
        m_scene->clear();
        QGraphicsTextItem *textItem = m_scene->addText("暂无数据\n\n请先加载数据文件");
        textItem->setDefaultTextColor(Qt::gray);
        textItem->setFont(QFont("Arial", 14));
        QRectF textRect = textItem->boundingRect();
        textItem->setPos(-textRect.width() / 2, -textRect.height() / 2);
        return;
    }

    // 检查节点数量，如果太多就不显示图形
    int nodeCount = m_dijkstra->nodeCount();
    if (nodeCount > MAX_VISUALIZATION_NODES)
    {
        // 显示提示信息而不是图形
        m_scene->clear();
        QGraphicsTextItem *textItem = m_scene->addText(
            QString("节点数量过多 (%1 个节点)\n\n"
                   "为了性能考虑，当节点数超过 %2 个时\n"
                   "将不显示图形化视图。\n\n"
                   "请使用数据管理界面查看和编辑数据。")
            .arg(nodeCount)
            .arg(MAX_VISUALIZATION_NODES)
        );
        textItem->setDefaultTextColor(Qt::red);
        textItem->setFont(QFont("Arial", 14, QFont::Bold));
        QRectF textRect = textItem->boundingRect();
        textItem->setPos(-textRect.width() / 2, -textRect.height() / 2);
        m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        return;
    }

    // 创建节点
    QVector<long> nodeIDs = m_dijkstra->getAllNodeIDs();
    qDebug() << "Creating graph with" << nodeIDs.size() << "nodes";
    
    for (long id : nodeIDs)
    {
        QString label = m_dijkstra->getNodeLabel(id);
        GraphNode *node = new GraphNode(id, label);
        m_nodes[id] = node;
        m_scene->addItem(node);

        // 如果有记忆的布局位置，则恢复该节点的位置
        if (m_nodePositions.contains(id))
        {
            QPointF savedPos = m_nodePositions.value(id);
            node->setPos(savedPos);
            qDebug() << "恢复节点" << id << "位置到:" << savedPos;
        }
    }

    qDebug() << "Created" << m_nodes.size() << "nodes in scene";

    // 创建边
    for (long id : nodeIDs)
    {
        QMap<long, long> neighbors = m_dijkstra->getNodeNeighbors(id);
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            long neighborID = it.key();
            long distance = it.value();
            
            // 避免重复创建边（无向图）
            if (id < neighborID)
            {
                GraphNode *source = m_nodes[id];
                GraphNode *dest = m_nodes[neighborID];
                if (source && dest)
                {
                    GraphEdge *edge = new GraphEdge(source, dest, distance);
                    m_edges.append(edge);
                }
            }
        }
    }

    qDebug() << "Created" << m_edges.size() << "edges";

    // 如果没有保存过用户布局，则使用自动布局初始化一次
    if (m_nodePositions.isEmpty())
    {
        autoLayout();
    }

    // 确保视图正确显示
    QRectF itemsRect = m_scene->itemsBoundingRect();
    if (!itemsRect.isEmpty())
    {
        // 扩大场景矩形以确保所有内容可见
        qreal margin = 50;
        itemsRect.adjust(-margin, -margin, margin, margin);
        m_scene->setSceneRect(itemsRect);
        m_view->fitInView(itemsRect, Qt::KeepAspectRatio);
        qDebug() << "Fitted view to rect:" << itemsRect;
    }
    else
    {
        qDebug() << "Warning: itemsBoundingRect is empty!";
    }
}

void VisualizationWindow::clearGraph()
{
    // 清除所有图形项
    foreach (GraphEdge *edge, m_edges)
    {
        delete edge;
    }
    m_edges.clear();

    foreach (GraphNode *node, m_nodes)
    {
        delete node;
    }
    m_nodes.clear();

    m_scene->clear();
}

void VisualizationWindow::autoLayout()
{
    if (m_nodes.isEmpty())
    {
        qDebug() << "autoLayout: no nodes to layout";
        return;
    }

    int nodeCount = m_nodes.size();
    QVector<long> nodeIDs = m_nodes.keys().toVector();
    
    // 使用圆形布局
    qreal radius = qMax(100.0, qMin(400.0, (double)nodeCount * 15.0));
    qreal angleStep = 2 * M_PI / nodeCount;
    
    qDebug() << "Layouting" << nodeCount << "nodes with radius" << radius;
    
    for (int i = 0; i < nodeCount; i++)
    {
        long id = nodeIDs[i];
        GraphNode *node = m_nodes[id];
        if (node)
        {
            qreal angle = i * angleStep;
            qreal x = radius * cos(angle);
            qreal y = radius * sin(angle);
            node->setPos(x, y);
            qDebug() << "Node" << id << "at position" << x << y;
        }
    }
    
    // 更新场景矩形
    QRectF itemsRect = m_scene->itemsBoundingRect();
    if (!itemsRect.isEmpty())
    {
        qreal margin = 50;
        itemsRect.adjust(-margin, -margin, margin, margin);
        m_scene->setSceneRect(itemsRect);
    }
}

void VisualizationWindow::highlightPath(const QVector<long> &path)
{
    // 清除之前的高亮
    foreach (GraphNode *node, m_nodes)
    {
        node->setPathNode(false);
        node->setHighlighted(false);
        node->setVisited(false);
        node->setCurrent(false);
    }

    foreach (GraphEdge *edge, m_edges)
    {
        edge->setPathEdge(false);
        edge->setHighlighted(false);
    }

    m_pathNodes.clear();
    m_pathEdges.clear();

    if (path.isEmpty())
    {
        update();
        return;
    }

    // 高亮路径节点
    for (long id : path)
    {
        if (m_nodes.contains(id))
        {
            m_nodes[id]->setPathNode(true);
            m_pathNodes.insert(id);
        }
    }

    // 高亮路径边
    for (int i = 0; i < path.size() - 1; i++)
    {
        long id1 = path[i];
        long id2 = path[i + 1];
        
        QPair<long, long> edgePair1(id1, id2);
        QPair<long, long> edgePair2(id2, id1);
        m_pathEdges.insert(edgePair1);
        m_pathEdges.insert(edgePair2);

        foreach (GraphEdge *edge, m_edges)
        {
            if ((edge->sourceNode()->nodeID() == id1 && edge->destNode()->nodeID() == id2) ||
                (edge->sourceNode()->nodeID() == id2 && edge->destNode()->nodeID() == id1))
            {
                edge->setPathEdge(true);
                break;
            }
        }
    }

    update();
}

void VisualizationWindow::startAnimation(long startNode, long endNode)
{
    if (!m_dijkstra)
        return;

    m_animationTimer->stop();
    m_animationIndex = 0;
    m_visitedNodes.clear();
    m_pathNodes.clear();
    m_pathEdges.clear();
    m_animationProgress->setValue(0);
    m_animationStatus->setText("开始计算...");

    // 清除所有高亮
    foreach (GraphNode *node, m_nodes)
    {
        node->setPathNode(false);
        node->setHighlighted(false);
        node->setVisited(false);
        node->setCurrent(false);
    }

    foreach (GraphEdge *edge, m_edges)
    {
        edge->setPathEdge(false);
        edge->setHighlighted(false);
    }

    long distance = 0;
    QVector<long> path;

    // 动画回调 - 实时更新节点状态
    AnimationCallback animCallback = [this](int nodeIndex, long dist, bool finished) {
        QMetaObject::invokeMethod(this, [this, nodeIndex, dist, finished]() {
            if (finished)
            {
                onAnimationFinished();
            }
            else
            {
                long nodeID = m_dijkstra->nodeID(nodeIndex);
                m_visitedNodes.insert(nodeID);
                
                // 更新节点显示状态
                foreach (GraphNode *node, m_nodes)
                {
                    bool isCurrent = (node->nodeID() == nodeID);
                    bool isVisited = m_visitedNodes.contains(node->nodeID());
                    node->setCurrent(isCurrent);
                    node->setVisited(isVisited);
                }
                
                // 更新进度
                int progress = m_dijkstra->nodeCount() > 0 ? 
                              (m_visitedNodes.size() * 100 / m_dijkstra->nodeCount()) : 0;
                m_animationProgress->setValue(qMin(progress, 95));
                m_animationStatus->setText(QString("访问节点: %1 (距离: %2)").arg(nodeID).arg(dist));
                
                // 强制刷新视图
                m_view->viewport()->update();
            }
        }, Qt::QueuedConnection);
    };

    int result = m_dijkstra->getDistance(startNode, endNode, distance, path, animCallback);

    if (result > 0)
    {
        // 延迟显示路径
        QTimer::singleShot(300, this, [this, path, distance]() {
            highlightPath(path);
            m_animationStatus->setText(QString("计算完成！距离: %1").arg(distance));
            m_animationProgress->setValue(100);
        });
    }
    else if (result == -1)
    {
        m_animationStatus->setText("无路径可达");
        m_animationProgress->setValue(100);
    }
    else
    {
        m_animationStatus->setText("计算失败");
        m_animationProgress->setValue(0);
    }
}

void VisualizationWindow::onAnimationStep()
{
    // 动画步骤处理（如果需要更细粒度的控制）
}

void VisualizationWindow::onAnimationFinished()
{
    m_animationTimer->stop();
    m_animationStatus->setText("动画完成");
    m_animationProgress->setValue(100);
}

void VisualizationWindow::zoomIn()
{
    if (m_view)
        m_view->scale(1.2, 1.2);
}

void VisualizationWindow::zoomOut()
{
    if (m_view)
        m_view->scale(1.0 / 1.2, 1.0 / 1.2);
}

void VisualizationWindow::resetZoom()
{
    if (m_view)
        m_view->resetTransform();
}

void VisualizationWindow::onQuickCalculate()
{
    if (!m_dijkstra)
        return;
    bool ok1, ok2;
    long startId = m_calcStartEdit->text().toLong(&ok1);
    long endId = m_calcEndEdit->text().toLong(&ok2);
    if (!ok1 || !ok2)
    {
        QMessageBox::warning(this, "输入错误", "请输入有效的节点ID！");
        return;
    }

    QVector<long> path;
    long distance = 0;
    int result = m_dijkstra->getDistance(startId, endId, distance, path);
    if (result > 0)
    {
        highlightPath(path);

        // 统一计算结果展示
        showCalculationResult(startId, endId, distance, path);

        // 发送结果到主界面
        QString resultText;
        resultText += QString("起始节点: %1\n").arg(startId);
        resultText += QString("终止节点: %1\n").arg(endId);
        resultText += QString("最短距离: %1\n\n").arg(distance);
        resultText += "路径: ";
        for (int i = 0; i < path.size(); ++i)
        {
            resultText += QString::number(path[i]);
            if (i < path.size() - 1)
                resultText += " → ";
        }
        resultText += QString("\n\n路径节点数: %1").arg(path.size());
        emit calcResultReady(resultText);
    }
    else if (result == -1)
    {
        highlightPath(QVector<long>());
        QMessageBox::information(this, "结果", "两个节点之间无路径可达！");
    }
    else
    {
        QString err = m_dijkstra->errorDescription();
        if (err.isEmpty())
            err = "计算失败";
        QMessageBox::critical(this, "错误", err);
    }
}

void VisualizationWindow::onQuickAddEdge()
{
    bool ok1, ok2, ok3;
    long id1 = m_quickNode1Edit->text().toLong(&ok1);
    long id2 = m_quickNode2Edit->text().toLong(&ok2);
    long distance = m_quickDistanceEdit->text().toLong(&ok3);

    if (!ok1 || !ok2 || !ok3)
    {
        QMessageBox::warning(this, "输入错误", "请输入有效的节点ID和距离！");
        return;
    }
    if (distance <= 0)
    {
        QMessageBox::warning(this, "输入错误", "距离必须大于0！");
        return;
    }

    if (!m_dijkstra->addNodesDist(id1, id2, distance))
    {
        QMessageBox::critical(this, "错误", QString("添加失败:\n%1").arg(m_dijkstra->errorDescription()));
        return;
    }

    QString label1 = m_quickNode1LabelEdit->text().trimmed();
    QString label2 = m_quickNode2LabelEdit->text().trimmed();
    if (!label1.isEmpty())
        m_dijkstra->setNodeLabel(id1, label1);
    if (!label2.isEmpty())
        m_dijkstra->setNodeLabel(id2, label2);

    if (m_database)
    {
        m_database->addOrUpdateNode(id1, m_dijkstra->getNodeLabel(id1));
        m_database->addOrUpdateNode(id2, m_dijkstra->getNodeLabel(id2));
        m_database->addOrUpdateEdge(id1, id2, distance);
    }

    m_quickNode1Edit->clear();
    m_quickNode2Edit->clear();
    m_quickDistanceEdit->clear();
    m_quickNode1LabelEdit->clear();
    m_quickNode2LabelEdit->clear();

    updateGraph();
    updateStatistics();
    emit graphDataChanged();
    QMessageBox::information(this, "成功", "边已添加并保存！");
}

void VisualizationWindow::onNodeLabelChanged()
{
    bool ok;
    long nodeID = m_editNodeID->text().toLong(&ok);
    if (!ok)
    {
        QMessageBox::warning(this, "错误", "请输入有效的节点ID！");
        return;
    }

    QString label = m_editNodeLabel->text();
    if (label.isEmpty())
        label = QString::number(nodeID);
    m_dijkstra->setNodeLabel(nodeID, label);
    if (m_database)
        m_database->addOrUpdateNode(nodeID, label);

    // 只更新对应图元的标签和统计信息，不重置布局
    if (m_nodes.contains(nodeID) && m_nodes[nodeID])
    {
        m_nodes[nodeID]->setLabel(label);
    }
    updateStatistics();

    emit graphDataChanged();
    QMessageBox::information(this, "成功", "节点标签已更新并保存！");
}

void VisualizationWindow::onLayoutChanged()
{
    autoLayout();
}

void VisualizationWindow::updateStatistics()
{
    if (!m_dijkstra)
    {
        m_statsGroup->setTitle("数据统计");
        m_statsText->setStyleSheet(QString());
        m_statsText->setPlainText("无数据");
        return;
    }

    m_statsGroup->setTitle("数据统计");
    m_statsText->setStyleSheet(QString());
    Dijkstra::GraphStats stats = m_dijkstra->getGraphStats();
    
    QString statsText;
    statsText += QString("节点数量: %1\n").arg(stats.nodeCount);
    statsText += QString("边的数量: %1\n").arg(stats.edgeCount);
    statsText += QString("平均度数: %1\n").arg(stats.avgDegree, 0, 'f', 2);
    statsText += QString("最大度数: %1\n").arg(stats.maxDegree);
    statsText += QString("最小度数: %1\n").arg(stats.minDegree);
    statsText += QString("总距离: %1\n").arg(stats.totalDistance);
    
    if (stats.nodeCount > 0)
    {
        double density = (double)stats.edgeCount / (stats.nodeCount * (stats.nodeCount - 1) / 2.0);
        statsText += QString("\n图密度: %1\n").arg(density, 0, 'f', 4);
    }

    m_statsText->setPlainText(statsText);
}

QPointF VisualizationWindow::calculateNodePosition(int index, int total)
{
    // 圆形布局
    qreal radius = 200;
    qreal angle = 2 * M_PI * index / total;
    return QPointF(radius * cos(angle), radius * sin(angle));
}

void VisualizationWindow::showCalculationResult(long startId, long endId, long distance, const QVector<long> &path)
{
    if (!m_statsGroup || !m_statsText)
        return;

    m_statsGroup->setTitle("计算结果");

    QString text;
    text += QString("起始节点: %1\n").arg(startId);
    text += QString("终止节点: %1\n").arg(endId);
    text += QString("最短距离: %1\n\n").arg(distance);
    text += "路径: ";
    for (int i = 0; i < path.size(); ++i)
    {
        text += QString::number(path[i]);
        if (i < path.size() - 1)
            text += " → ";
    }
    text += QString("\n\n路径节点数: %1").arg(path.size());

    m_statsText->setPlainText(text);

    // 简单的闪烁动画提醒
    m_statsText->setStyleSheet("background-color: #FFF9C4; border: 2px solid #FF9800;");
    QTimer *timer = new QTimer(this);
    int *remaining = new int(8); // 闪烁次数
    connect(timer, &QTimer::timeout, this, [this, timer, remaining]() {
        bool on = ((*remaining) % 2) == 0;
        if (on)
            m_statsText->setStyleSheet("background-color: #FFF9C4; border: 2px solid #FF9800;");
        else
            m_statsText->setStyleSheet("background-color: white; border: 1px solid #CCCCCC;");

        (*remaining)--;
        if (*remaining <= 0)
        {
            timer->stop();
            timer->deleteLater();
            delete remaining;
            m_statsText->setStyleSheet(QString());
        }
    });
    timer->start(150);
}

void VisualizationWindow::onSelectionChanged()
{
    if (!m_scene)
        return;

    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    QVector<long> selectedIds;
    QVector<GraphNode*> selectedNodes;
    for (QGraphicsItem *item : selectedItems)
    {
        if (GraphNode *node = qgraphicsitem_cast<GraphNode*>(item))
        {
            selectedIds.append(node->nodeID());
            selectedNodes.append(node);
        }
    }

    // 节点标签设置区域：当且仅当选中一个节点时，自动填充并高亮“设置标签”按钮
    if (selectedNodes.size() == 1)
    {
        GraphNode *node = selectedNodes.first();
        if (m_editNodeID)
            m_editNodeID->setText(QString::number(node->nodeID()));
        if (m_editNodeLabel)
            m_editNodeLabel->setText(node->label());
        if (m_btnSetLabel)
        {
            m_btnSetLabel->setEnabled(true);
            m_btnSetLabel->setStyleSheet("background-color: #ffd54f; color: #333333; font-weight: bold;");
        }
    }
    else
    {
        if (m_editNodeID)
            m_editNodeID->clear();
        if (m_editNodeLabel)
            m_editNodeLabel->clear();
        if (m_btnSetLabel)
        {
            m_btnSetLabel->setEnabled(true);
            m_btnSetLabel->setStyleSheet(QString());
        }
    }

    if (selectedIds.size() == 2)
    {
        long id1 = selectedIds[0];
        long id2 = selectedIds[1];
        m_calcStartEdit->setText(QString::number(id1));
        m_calcEndEdit->setText(QString::number(id2));
        if (m_animationStatus)
            m_animationStatus->setText("已选中两个节点，可计算路径。");
    }
    else if (selectedIds.size() > 2)
    {
        // 多于两个选中时禁用计算
        m_btnQuickCalculate->setEnabled(false);
        m_btnQuickCalculate->setStyleSheet(QString());
        m_calcStartEdit->clear();
        m_calcEndEdit->clear();
        if (m_animationStatus)
            m_animationStatus->setText("已选择多个节点，无法计算路径（仅支持选中两个节点）。");
    }
    else
    {
        // 0 或 1 个选中，恢复默认按钮状态，可手动输入计算
        if (selectedIds.size() == 1)
        {
            m_calcStartEdit->setText(QString::number(selectedIds[0]));
            m_calcEndEdit->clear();
            if (m_animationStatus)
                m_animationStatus->setText("已选中一个节点，可手动输入另一个节点进行计算。");
        }
        else
        {
            // 0 个选中，清空输入，路径计算需手动输入
            m_calcStartEdit->clear();
            m_calcEndEdit->clear();
            if (m_animationStatus)
                m_animationStatus->setText("就绪");
        }

        // 0 或 1 个选中，按钮是否高亮由输入框内容决定
        onCalcInputChanged();
    }
    
    // 当正好两个节点选中时，根据输入内容（已自动填充）刷新按钮高亮
    if (selectedIds.size() == 2)
        onCalcInputChanged();
}

void VisualizationWindow::onSaveLayoutToDatabase()
{
    if (!m_database || !m_scene)
        return;

    // 收集当前所有节点的位置
    QMap<long, QPointF> positions;
    for (auto it = m_nodes.constBegin(); it != m_nodes.constEnd(); ++it)
    {
        if (it.value())
        {
            positions.insert(it.key(), it.value()->pos());
        }
    }

        // 如果有节点位置数据，保存到数据库
        if (!positions.isEmpty())
        {
            QString currentTable = m_database->currentTable();
            qDebug() << "准备保存布局到表:" << currentTable << "节点数:" << positions.size();
            
            if (m_database->saveLayout(positions))
            {
                // 更新内存中的布局记忆
                m_nodePositions = positions;
                qDebug() << "✓ 布局已成功保存到数据库，节点数:" << positions.size();
            }
            else
            {
                qDebug() << "✗ 保存布局失败:" << m_database->lastError();
            }
        }
        else
        {
            qDebug() << "没有节点位置数据，跳过保存";
        }
}

