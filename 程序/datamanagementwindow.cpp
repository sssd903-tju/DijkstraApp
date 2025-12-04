#include "datamanagementwindow.h"
#include "dijkstra.h"
#include "graphdatabase.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <algorithm>
#include <climits>

DataManagementWindow::DataManagementWindow(Dijkstra *dijkstra, GraphDatabase *db, QWidget *parent)
    : QMainWindow(parent)
    , m_dijkstra(dijkstra)
    , m_db(db)
    , m_selectedNodeRow(-1)
    , m_selectedEdgeRow(-1)
    , m_nodePage(0)
    , m_edgePage(0)
    , m_nodeTotalPages(0)
    , m_edgeTotalPages(0)
{
    setupUI();
    refreshData();
}

DataManagementWindow::~DataManagementWindow()
{
}

void DataManagementWindow::setupUI()
{
    setWindowTitle("数据管理界面");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 左侧：节点管理
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QGroupBox *nodeGroup = new QGroupBox("节点管理", this);
    QVBoxLayout *nodeLayout = new QVBoxLayout();
    
    // 节点过滤和排序
    QHBoxLayout *nodeFilterLayout = new QHBoxLayout();
    nodeFilterLayout->addWidget(new QLabel("搜索:", this));
    m_nodeFilterEdit = new QLineEdit(this);
    m_nodeFilterEdit->setPlaceholderText("输入节点ID或标签...");
    nodeFilterLayout->addWidget(m_nodeFilterEdit);
    nodeFilterLayout->addWidget(new QLabel("排序:", this));
    m_nodeSortCombo = new QComboBox(this);
    m_nodeSortCombo->addItems(QStringList() << "按ID" << "按标签" << "按度数");
    nodeFilterLayout->addWidget(m_nodeSortCombo);
    nodeLayout->addLayout(nodeFilterLayout);
    
    connect(m_nodeFilterEdit, &QLineEdit::textChanged, this, &DataManagementWindow::onNodeFilterChanged);
    connect(m_nodeSortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DataManagementWindow::onNodeFilterChanged);
    
    // 节点表格
    m_nodeTable = new QTableWidget(this);
    m_nodeTable->setColumnCount(3);
    m_nodeTable->setHorizontalHeaderLabels(QStringList() << "节点ID" << "标签" << "度数");
    m_nodeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_nodeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_nodeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_nodeTable->horizontalHeader()->setStretchLastSection(true);
    m_nodeTable->setSortingEnabled(true);
    nodeLayout->addWidget(m_nodeTable);
    
    connect(m_nodeTable, &QTableWidget::itemSelectionChanged, this, &DataManagementWindow::onNodeTableSelectionChanged);
    connect(m_nodeTable, &QTableWidget::cellDoubleClicked, this, &DataManagementWindow::onNodeTableDoubleClicked);
    
    // 节点分页控件
    QHBoxLayout *nodePageLayout = new QHBoxLayout();
    m_btnNodePrevPage = new QPushButton("上一页", this);
    m_btnNodeNextPage = new QPushButton("下一页", this);
    m_nodePageLabel = new QLabel("第 0 / 0 页", this);
    nodePageLayout->addWidget(m_btnNodePrevPage);
    nodePageLayout->addWidget(m_btnNodeNextPage);
    nodePageLayout->addWidget(m_nodePageLabel);
    nodePageLayout->addStretch();
    nodeLayout->addLayout(nodePageLayout);

    connect(m_btnNodePrevPage, &QPushButton::clicked, this, &DataManagementWindow::onNodePrevPage);
    connect(m_btnNodeNextPage, &QPushButton::clicked, this, &DataManagementWindow::onNodeNextPage);

    // 节点操作按钮
    QHBoxLayout *nodeButtonLayout = new QHBoxLayout();
    m_btnAddNode = new QPushButton("添加节点", this);
    m_btnEditNode = new QPushButton("编辑节点", this);
    m_btnDeleteNode = new QPushButton("删除节点", this);
    nodeButtonLayout->addWidget(m_btnAddNode);
    nodeButtonLayout->addWidget(m_btnEditNode);
    nodeButtonLayout->addWidget(m_btnDeleteNode);
    nodeButtonLayout->addStretch();
    nodeLayout->addLayout(nodeButtonLayout);
    
    connect(m_btnAddNode, &QPushButton::clicked, this, &DataManagementWindow::onAddNode);
    connect(m_btnEditNode, &QPushButton::clicked, this, &DataManagementWindow::onEditNode);
    connect(m_btnDeleteNode, &QPushButton::clicked, this, &DataManagementWindow::onDeleteNode);
    
    nodeGroup->setLayout(nodeLayout);
    leftLayout->addWidget(nodeGroup);
    
    mainLayout->addLayout(leftLayout, 1);

    // 中间：边管理
    QVBoxLayout *middleLayout = new QVBoxLayout();
    
    QGroupBox *edgeGroup = new QGroupBox("边管理", this);
    QVBoxLayout *edgeLayout = new QVBoxLayout();
    
    // 边过滤和排序
    QHBoxLayout *edgeFilterLayout = new QHBoxLayout();
    edgeFilterLayout->addWidget(new QLabel("搜索:", this));
    m_edgeFilterEdit = new QLineEdit(this);
    m_edgeFilterEdit->setPlaceholderText("输入节点ID...");
    edgeFilterLayout->addWidget(m_edgeFilterEdit);
    edgeFilterLayout->addWidget(new QLabel("排序:", this));
    m_edgeSortCombo = new QComboBox(this);
    m_edgeSortCombo->addItems(QStringList() << "按节点1" << "按节点2" << "按距离");
    edgeFilterLayout->addWidget(m_edgeSortCombo);
    edgeLayout->addLayout(edgeFilterLayout);
    
    connect(m_edgeFilterEdit, &QLineEdit::textChanged, this, &DataManagementWindow::onEdgeFilterChanged);
    connect(m_edgeSortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DataManagementWindow::onEdgeFilterChanged);
    
    // 边表格
    m_edgeTable = new QTableWidget(this);
    m_edgeTable->setColumnCount(3);
    m_edgeTable->setHorizontalHeaderLabels(QStringList() << "节点1" << "节点2" << "距离");
    m_edgeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_edgeTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_edgeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_edgeTable->horizontalHeader()->setStretchLastSection(true);
    m_edgeTable->setSortingEnabled(true);
    edgeLayout->addWidget(m_edgeTable);
    
    connect(m_edgeTable, &QTableWidget::itemSelectionChanged, this, &DataManagementWindow::onEdgeTableSelectionChanged);
    connect(m_edgeTable, &QTableWidget::cellDoubleClicked, this, &DataManagementWindow::onEdgeTableDoubleClicked);

    // 边分页控件
    QHBoxLayout *edgePageLayout = new QHBoxLayout();
    m_btnEdgePrevPage = new QPushButton("上一页", this);
    m_btnEdgeNextPage = new QPushButton("下一页", this);
    m_edgePageLabel = new QLabel("第 0 / 0 页", this);
    edgePageLayout->addWidget(m_btnEdgePrevPage);
    edgePageLayout->addWidget(m_btnEdgeNextPage);
    edgePageLayout->addWidget(m_edgePageLabel);
    edgePageLayout->addStretch();
    edgeLayout->addLayout(edgePageLayout);

    connect(m_btnEdgePrevPage, &QPushButton::clicked, this, &DataManagementWindow::onEdgePrevPage);
    connect(m_btnEdgeNextPage, &QPushButton::clicked, this, &DataManagementWindow::onEdgeNextPage);
    
    // 边操作按钮
    QHBoxLayout *edgeButtonLayout = new QHBoxLayout();
    m_btnAddEdge = new QPushButton("添加边", this);
    m_btnEditEdge = new QPushButton("编辑边", this);
    m_btnDeleteEdge = new QPushButton("删除边", this);
    m_btnBatchDelete = new QPushButton("批量删除", this);
    edgeButtonLayout->addWidget(m_btnAddEdge);
    edgeButtonLayout->addWidget(m_btnEditEdge);
    edgeButtonLayout->addWidget(m_btnDeleteEdge);
    edgeButtonLayout->addWidget(m_btnBatchDelete);
    edgeButtonLayout->addStretch();
    edgeLayout->addLayout(edgeButtonLayout);
    
    connect(m_btnAddEdge, &QPushButton::clicked, this, &DataManagementWindow::onAddEdge);
    connect(m_btnEditEdge, &QPushButton::clicked, this, &DataManagementWindow::onEditEdge);
    connect(m_btnDeleteEdge, &QPushButton::clicked, this, &DataManagementWindow::onDeleteEdge);
    connect(m_btnBatchDelete, &QPushButton::clicked, this, &DataManagementWindow::onBatchDelete);
    
    edgeGroup->setLayout(edgeLayout);
    middleLayout->addWidget(edgeGroup);
    
    mainLayout->addLayout(middleLayout, 1);

    // 右侧：统计和操作
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    // 统计信息
    QGroupBox *statsGroup = new QGroupBox("统计信息", this);
    QVBoxLayout *statsLayout = new QVBoxLayout();
    m_statsText = new QTextEdit(this);
    m_statsText->setReadOnly(true);
    m_statsText->setMaximumHeight(200);
    statsLayout->addWidget(m_statsText);
    statsGroup->setLayout(statsLayout);
    rightLayout->addWidget(statsGroup);
    
    // 数据操作
    QGroupBox *dataGroup = new QGroupBox("数据操作", this);
    QVBoxLayout *dataLayout = new QVBoxLayout();
    m_btnRefresh = new QPushButton("刷新数据", this);
    m_btnExport = new QPushButton("导出到文本", this);
    m_btnImport = new QPushButton("从文本导入", this);
    m_btnLoadDb = new QPushButton("从数据库加载", this);
    m_btnSaveDb = new QPushButton("保存到数据库", this);
    dataLayout->addWidget(m_btnRefresh);
    dataLayout->addWidget(m_btnExport);
    dataLayout->addWidget(m_btnImport);
    dataLayout->addWidget(m_btnLoadDb);
    dataLayout->addWidget(m_btnSaveDb);
    dataLayout->addStretch();
    dataGroup->setLayout(dataLayout);
    rightLayout->addWidget(dataGroup);
    
    connect(m_btnRefresh, &QPushButton::clicked, this, &DataManagementWindow::refreshData);
    connect(m_btnExport, &QPushButton::clicked, this, &DataManagementWindow::onExportData);
    connect(m_btnImport, &QPushButton::clicked, this, &DataManagementWindow::onImportData);
    connect(m_btnLoadDb, &QPushButton::clicked, this, &DataManagementWindow::onLoadFromDatabase);
    connect(m_btnSaveDb, &QPushButton::clicked, this, &DataManagementWindow::onSaveToDatabase);
    
    // 粘贴导入
    QGroupBox *pasteGroup = new QGroupBox("粘贴边数据导入 (每行: id1 id2 dist)", this);
    QVBoxLayout *pasteLayout = new QVBoxLayout();
    m_pasteEdit = new QTextEdit(this);
    m_pasteEdit->setPlaceholderText("例如:\n101 102 6\n101 103 3\n...");
    m_btnPasteImport = new QPushButton("从粘贴文本导入", this);
    pasteLayout->addWidget(m_pasteEdit);
    pasteLayout->addWidget(m_btnPasteImport);
    pasteGroup->setLayout(pasteLayout);
    rightLayout->addWidget(pasteGroup);
    connect(m_btnPasteImport, &QPushButton::clicked, this, &DataManagementWindow::onPasteImport);
    
    // 状态栏
    m_statusLabel = new QLabel("就绪", this);
    rightLayout->addWidget(m_statusLabel);
    
    rightLayout->addStretch();
    mainLayout->addLayout(rightLayout, 0);
}

void DataManagementWindow::refreshData()
{
    // 简单节流，避免过于频繁刷新导致卡顿
    if (m_lastRefreshTime.isValid() &&
        m_lastRefreshTime.msecsTo(QTime::currentTime()) < 200)
    {
        return;
    }
    m_lastRefreshTime = QTime::currentTime();

    m_nodePage = 0;
    m_edgePage = 0;
    populateNodeTable();
    populateEdgeTable();
    updateStatistics();
    m_statusLabel->setText(QString("数据已刷新 - %1 个节点").arg(m_dijkstra->nodeCount()));
}

void DataManagementWindow::populateNodeTable()
{
    m_nodeTable->setRowCount(0);
    
    if (!m_dijkstra)
        return;

    QVector<long> nodeIDs = m_dijkstra->getAllNodeIDs();
    
    // 应用过滤和排序
    QString filter = m_nodeFilterEdit->text().toLower();
    int sortType = m_nodeSortCombo->currentIndex();
    
    // 构建节点数据列表
    struct NodeData {
        long id;
        QString label;
        int degree;
    };
    
    QVector<NodeData> nodeDataList;
    for (long id : nodeIDs)
    {
        QString label = m_dijkstra->getNodeLabel(id);
        QMap<long, long> neighbors = m_dijkstra->getNodeNeighbors(id);
        int degree = neighbors.size();
        
        // 过滤
        if (!filter.isEmpty())
        {
            if (!QString::number(id).contains(filter) && 
                !label.toLower().contains(filter))
                continue;
        }
        
        nodeDataList.append({id, label, degree});
    }
    
    // 排序
    if (sortType == 0) // 按ID
    {
        std::sort(nodeDataList.begin(), nodeDataList.end(), 
                  [](const NodeData &a, const NodeData &b) { return a.id < b.id; });
    }
    else if (sortType == 1) // 按标签
    {
        std::sort(nodeDataList.begin(), nodeDataList.end(), 
                  [](const NodeData &a, const NodeData &b) { return a.label < b.label; });
    }
    else // 按度数
    {
        std::sort(nodeDataList.begin(), nodeDataList.end(), 
                  [](const NodeData &a, const NodeData &b) { return a.degree > b.degree; });
    }
    
    // 分页计算
    int total = nodeDataList.size();
    m_nodeTotalPages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    if (m_nodeTotalPages == 0)
        m_nodePage = 0;
    else if (m_nodePage >= m_nodeTotalPages)
        m_nodePage = m_nodeTotalPages - 1;

    int start = m_nodePage * PAGE_SIZE;
    int end = qMin(start + PAGE_SIZE, total);
    int rowCount = qMax(0, end - start);
    m_nodeTable->setRowCount(rowCount);
    
    for (int i = 0; i < rowCount; i++)
    {
        const NodeData &data = nodeDataList[start + i];
        m_nodeTable->setItem(i, 0, new QTableWidgetItem(QString::number(data.id)));
        m_nodeTable->setItem(i, 1, new QTableWidgetItem(data.label));
        m_nodeTable->setItem(i, 2, new QTableWidgetItem(QString::number(data.degree)));
    }

    updateNodePageControls();
}

void DataManagementWindow::populateEdgeTable()
{
    m_edgeTable->setRowCount(0);
    
    if (!m_dijkstra)
        return;

    QVector<long> nodeIDs = m_dijkstra->getAllNodeIDs();
    QString filter = m_edgeFilterEdit->text();
    int sortType = m_edgeSortCombo->currentIndex();
    
    // 构建边数据列表
    struct EdgeData {
        long id1;
        long id2;
        long distance;
    };
    
    QSet<QPair<long, long>> addedEdges; // 避免重复
    QVector<EdgeData> edgeDataList;
    
    for (long id1 : nodeIDs)
    {
        QMap<long, long> neighbors = m_dijkstra->getNodeNeighbors(id1);
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            long id2 = it.key();
            long distance = it.value();
            
            // 避免重复（无向图）
            QPair<long, long> edgePair(qMin(id1, id2), qMax(id1, id2));
            if (addedEdges.contains(edgePair))
                continue;
            addedEdges.insert(edgePair);
            
            // 过滤
            if (!filter.isEmpty())
            {
                if (!QString::number(id1).contains(filter) && 
                    !QString::number(id2).contains(filter))
                    continue;
            }
            
            edgeDataList.append({id1, id2, distance});
        }
    }
    
    // 排序
    if (sortType == 0) // 按节点1
    {
        std::sort(edgeDataList.begin(), edgeDataList.end(), 
                  [](const EdgeData &a, const EdgeData &b) { return a.id1 < b.id1; });
    }
    else if (sortType == 1) // 按节点2
    {
        std::sort(edgeDataList.begin(), edgeDataList.end(), 
                  [](const EdgeData &a, const EdgeData &b) { return a.id2 < b.id2; });
    }
    else // 按距离
    {
        std::sort(edgeDataList.begin(), edgeDataList.end(), 
                  [](const EdgeData &a, const EdgeData &b) { return a.distance < b.distance; });
    }
    
    // 分页计算
    int total = edgeDataList.size();
    m_edgeTotalPages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    if (m_edgeTotalPages == 0)
        m_edgePage = 0;
    else if (m_edgePage >= m_edgeTotalPages)
        m_edgePage = m_edgeTotalPages - 1;

    int start = m_edgePage * PAGE_SIZE;
    int end = qMin(start + PAGE_SIZE, total);
    int rowCount = qMax(0, end - start);
    m_edgeTable->setRowCount(rowCount);
    
    for (int i = 0; i < rowCount; i++)
    {
        const EdgeData &data = edgeDataList[start + i];
        m_edgeTable->setItem(i, 0, new QTableWidgetItem(QString::number(data.id1)));
        m_edgeTable->setItem(i, 1, new QTableWidgetItem(QString::number(data.id2)));
        m_edgeTable->setItem(i, 2, new QTableWidgetItem(QString::number(data.distance)));
    }

    updateEdgePageControls();
}

void DataManagementWindow::updateStatistics()
{
    if (!m_dijkstra)
    {
        m_statsText->setPlainText("无数据");
        return;
    }

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

void DataManagementWindow::onNodeFilterChanged()
{
    m_nodePage = 0;
    populateNodeTable();
}

void DataManagementWindow::onEdgeFilterChanged()
{
    m_edgePage = 0;
    populateEdgeTable();
}

void DataManagementWindow::onNodePrevPage()
{
    if (m_nodePage > 0)
    {
        --m_nodePage;
        populateNodeTable();
    }
}

void DataManagementWindow::onNodeNextPage()
{
    if (m_nodePage + 1 < m_nodeTotalPages)
    {
        ++m_nodePage;
        populateNodeTable();
    }
}

void DataManagementWindow::onEdgePrevPage()
{
    if (m_edgePage > 0)
    {
        --m_edgePage;
        populateEdgeTable();
    }
}

void DataManagementWindow::onEdgeNextPage()
{
    if (m_edgePage + 1 < m_edgeTotalPages)
    {
        ++m_edgePage;
        populateEdgeTable();
    }
}

void DataManagementWindow::updateNodePageControls()
{
    int total = m_nodeTotalPages;
    int current = (total == 0) ? 0 : (m_nodePage + 1);
    m_nodePageLabel->setText(QString("第 %1 / %2 页").arg(current).arg(total));
    m_btnNodePrevPage->setEnabled(m_nodePage > 0);
    m_btnNodeNextPage->setEnabled(m_nodePage + 1 < m_nodeTotalPages);
}

void DataManagementWindow::updateEdgePageControls()
{
    int total = m_edgeTotalPages;
    int current = (total == 0) ? 0 : (m_edgePage + 1);
    m_edgePageLabel->setText(QString("第 %1 / %2 页").arg(current).arg(total));
    m_btnEdgePrevPage->setEnabled(m_edgePage > 0);
    m_btnEdgeNextPage->setEnabled(m_edgePage + 1 < m_edgeTotalPages);
}

void DataManagementWindow::onNodeTableSelectionChanged()
{
    QList<QTableWidgetItem*> selected = m_nodeTable->selectedItems();
    m_selectedNodeRow = selected.isEmpty() ? -1 : selected.first()->row();
    m_btnEditNode->setEnabled(m_selectedNodeRow >= 0);
    m_btnDeleteNode->setEnabled(m_selectedNodeRow >= 0);
}

void DataManagementWindow::onEdgeTableSelectionChanged()
{
    QList<QTableWidgetItem*> selected = m_edgeTable->selectedItems();
    m_selectedEdgeRow = selected.isEmpty() ? -1 : selected.first()->row();
    m_btnEditEdge->setEnabled(m_selectedEdgeRow >= 0);
    m_btnDeleteEdge->setEnabled(m_selectedEdgeRow >= 0);
    m_btnBatchDelete->setEnabled(!selected.isEmpty());
}

void DataManagementWindow::onNodeTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0)
        onEditNode();
}

void DataManagementWindow::onEdgeTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0)
        onEditEdge();
}

bool DataManagementWindow::validateNodeInput(long &id, QString &label)
{
    bool ok;
    id = QInputDialog::getInt(this, "输入节点ID", "节点ID:", id, 1, INT_MAX, 1, &ok);
    if (!ok) return false;
    
    label = QInputDialog::getText(this, "输入节点标签", "节点标签:", QLineEdit::Normal, label);
    return true;
}

bool DataManagementWindow::validateEdgeInput(long &id1, long &id2, long &distance)
{
    bool ok1, ok2, ok3;
    id1 = QInputDialog::getInt(this, "输入节点1 ID", "节点1 ID:", id1, 1, INT_MAX, 1, &ok1);
    if (!ok1) return false;
    
    id2 = QInputDialog::getInt(this, "输入节点2 ID", "节点2 ID:", id2, 1, INT_MAX, 1, &ok2);
    if (!ok2) return false;
    
    distance = QInputDialog::getInt(this, "输入距离", "距离:", distance, 1, INT_MAX, 1, &ok3);
    return ok3;
}

void DataManagementWindow::onAddNode()
{
    long id = 0;
    QString label;
    if (!validateNodeInput(id, label))
        return;
    
    // 检查节点是否已存在
    if (m_dijkstra->nodeIndex(id) > 0)
    {
        QMessageBox::warning(this, "错误", QString("节点 %1 已存在！").arg(id));
        return;
    }
    
    // 添加一个虚拟边来创建节点（或者需要扩展Dijkstra类支持直接添加节点）
    // 暂时通过添加自环边来创建节点
    if (m_dijkstra->addNodesDist(id, id, 0))
    {
        m_dijkstra->setNodeLabel(id, label);
        if (m_db)
        {
            m_db->addOrUpdateNode(id, label.isEmpty() ? QString::number(id) : label);
            m_db->addOrUpdateEdge(id, id, 0);
        }
        refreshData();
        QMessageBox::information(this, "成功", "节点添加成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", m_dijkstra->errorDescription());
    }
}

void DataManagementWindow::onEditNode()
{
    if (m_selectedNodeRow < 0)
        return;
    
    QTableWidgetItem *idItem = m_nodeTable->item(m_selectedNodeRow, 0);
    QTableWidgetItem *labelItem = m_nodeTable->item(m_selectedNodeRow, 1);
    
    if (!idItem || !labelItem)
        return;
    
    long id = idItem->text().toLong();
    QString oldLabel = labelItem->text();
    
    QString newLabel = QInputDialog::getText(this, "编辑节点标签", "新标签:", QLineEdit::Normal, oldLabel);
    if (newLabel.isEmpty())
        return;
    
    m_dijkstra->setNodeLabel(id, newLabel);
    if (m_db)
        m_db->addOrUpdateNode(id, newLabel);
    refreshData();
    QMessageBox::information(this, "成功", "节点标签已更新！");
}

void DataManagementWindow::onDeleteNode()
{
    if (m_selectedNodeRow < 0)
        return;
    
    QTableWidgetItem *idItem = m_nodeTable->item(m_selectedNodeRow, 0);
    if (!idItem)
        return;
    
    long id = idItem->text().toLong();
    
    int ret = QMessageBox::question(this, "确认删除", 
                                     QString("确定要删除节点 %1 及其所有连接的边吗？").arg(id),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;
    
    // 删除节点需要删除所有相关的边
    // 注意：当前Dijkstra类不支持直接删除节点，需要扩展功能
    QMessageBox::information(this, "提示", "删除节点功能需要扩展Dijkstra类支持");
}

void DataManagementWindow::onAddEdge()
{
    long id1 = 0, id2 = 0, distance = 0;
    if (!validateEdgeInput(id1, id2, distance))
        return;
    
    if (m_dijkstra->addNodesDist(id1, id2, distance))
    {
        if (m_db)
        {
            m_db->addOrUpdateNode(id1, m_dijkstra->getNodeLabel(id1));
            m_db->addOrUpdateNode(id2, m_dijkstra->getNodeLabel(id2));
            m_db->addOrUpdateEdge(id1, id2, distance);
        }
        refreshData();
        QMessageBox::information(this, "成功", "边添加成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", m_dijkstra->errorDescription());
    }
}

void DataManagementWindow::onEditEdge()
{
    if (m_selectedEdgeRow < 0)
        return;
    
    QTableWidgetItem *id1Item = m_edgeTable->item(m_selectedEdgeRow, 0);
    QTableWidgetItem *id2Item = m_edgeTable->item(m_selectedEdgeRow, 1);
    QTableWidgetItem *distItem = m_edgeTable->item(m_selectedEdgeRow, 2);
    
    if (!id1Item || !id2Item || !distItem)
        return;
    
    long oldDist = distItem->text().toLong();
    
    // 先删除旧边，再添加新边
    // 注意：需要扩展Dijkstra类支持删除边
    bool ok;
    long newDist = QInputDialog::getInt(this, "编辑边距离", "新距离:", oldDist, 1, INT_MAX, 1, &ok);
    Q_UNUSED(newDist);
    
    if (ok)
    {
        QMessageBox::information(this, "提示", "编辑边功能需要扩展Dijkstra类支持删除边");
    }
}

void DataManagementWindow::onDeleteEdge()
{
    if (m_selectedEdgeRow < 0)
        return;
    
    QTableWidgetItem *id1Item = m_edgeTable->item(m_selectedEdgeRow, 0);
    QTableWidgetItem *id2Item = m_edgeTable->item(m_selectedEdgeRow, 1);
    
    if (!id1Item || !id2Item)
        return;
    
    long id1 = id1Item->text().toLong();
    long id2 = id2Item->text().toLong();
    
    int ret = QMessageBox::question(this, "确认删除", 
                                     QString("确定要删除边 (%1, %2) 吗？").arg(id1).arg(id2),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;
    
    Q_UNUSED(id1);
    Q_UNUSED(id2);
    QMessageBox::information(this, "提示", "删除边功能需要扩展Dijkstra类支持");
}

void DataManagementWindow::onBatchDelete()
{
    QList<QTableWidgetItem*> selected = m_edgeTable->selectedItems();
    if (selected.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请先选择要删除的边！");
        return;
    }
    
    int count = selected.size() / 3; // 每行3列
    int ret = QMessageBox::question(this, "确认批量删除", 
                                     QString("确定要删除选中的 %1 条边吗？").arg(count),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;
    
    QMessageBox::information(this, "提示", "批量删除功能需要扩展Dijkstra类支持");
}

void DataManagementWindow::onExportData()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出数据", "", "文本文件 (*.txt);;所有文件 (*.*)");
    if (fileName.isEmpty())
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "错误", "无法创建文件！");
        return;
    }
    
    QTextStream out(&file);
    QVector<long> nodeIDs = m_dijkstra->getAllNodeIDs();
    QSet<QPair<long, long>> addedEdges;
    
    int count = 0;
    for (long id1 : nodeIDs)
    {
        QMap<long, long> neighbors = m_dijkstra->getNodeNeighbors(id1);
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            long id2 = it.key();
            long distance = it.value();
            
            QPair<long, long> edgePair(qMin(id1, id2), qMax(id1, id2));
            if (addedEdges.contains(edgePair))
                continue;
            addedEdges.insert(edgePair);
            
            out << id1 << "\t" << id2 << "\t" << distance << "\n";
            count++;
            
            if (count % 1000 == 0)
            {
                QApplication::processEvents();
            }
        }
    }
    
    file.close();
    QMessageBox::information(this, "成功", QString("成功导出 %1 条边数据！").arg(count));
}

void DataManagementWindow::onImportData()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入数据", "", "文本文件 (*.txt);;所有文件 (*.*)");
    if (fileName.isEmpty())
        return;
    
    QProgressDialog progress("正在导入数据...", "取消", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    
    std::function<void(float)> progressCallback = [&progress](float p) {
        progress.setValue((int)(p * 100));
        QApplication::processEvents();
    };
    
    if (m_dijkstra->loadFileData(fileName, progressCallback))
    {
        progress.close();
        if (m_db)
        {
            m_db->clear();
            m_db->saveGraph(m_dijkstra);
        }
        refreshData();
        QMessageBox::information(this, "成功", QString("成功导入数据！\n节点数量: %1").arg(m_dijkstra->nodeCount()));
    }
    else
    {
        progress.close();
        QMessageBox::critical(this, "错误", QString("导入失败:\n%1").arg(m_dijkstra->errorDescription()));
    }
}

void DataManagementWindow::onLoadFromDatabase()
{
    if (!m_db)
    {
        QMessageBox::warning(this, "提示", "当前未启用数据库。");
        return;
    }
    // 获取所有表格信息
    QList<GraphTableInfo> tables = m_db->getAllTableInfos();
    if (tables.isEmpty())
    {
        QMessageBox::information(this, "提示", "数据库中尚未创建任何数据表格。");
        return;
    }

    // 构建显示列表
    QStringList items;
    for (const GraphTableInfo &info : tables)
    {
        QString display = info.displayName.isEmpty() ? info.tableName : info.displayName;
        QString line = QString("%1  (表名: %2, 节点: %3, 边: %4)")
                           .arg(display)
                           .arg(info.tableName)
                           .arg(info.nodeCount)
                           .arg(info.edgeCount);
        items << line;
    }

    bool ok = false;
    QString selected = QInputDialog::getItem(
        this,
        "从数据库加载",
        "请选择要加载的数据表格：",
        items,
        0,
        false,
        &ok);

    if (!ok || selected.isEmpty())
        return;

    // 找到对应的表名
    int index = items.indexOf(selected);
    if (index < 0 || index >= tables.size())
        return;
    QString tableName = tables[index].tableName;

    // 切换当前表格并加载数据
    if (!m_db->setCurrentTable(tableName))
    {
        QMessageBox::critical(this, "错误", QString("切换到表格失败:\n%1").arg(m_db->lastError()));
        return;
    }
    if (!m_db->loadGraph(m_dijkstra, tableName))
    {
        QMessageBox::critical(this, "错误", QString("从表格加载失败:\n%1").arg(m_db->lastError()));
        return;
    }

    m_nodePage = 0;
    m_edgePage = 0;
    refreshData();
    QMessageBox::information(this, "成功", QString("已从表格 \"%1\" 加载图数据，节点数量: %2")
                                             .arg(tables[index].displayName.isEmpty() ? tableName : tables[index].displayName)
                                             .arg(m_dijkstra->nodeCount()));
}

void DataManagementWindow::onSaveToDatabase()
{
    if (!m_db)
    {
        QMessageBox::warning(this, "提示", "当前未启用数据库。");
        return;
    }
    // 选择要保存到的表格：先让用户选择“新建表格”还是“保存到已有表格”
    QList<GraphTableInfo> tables = m_db->getAllTableInfos();
    QString tableName;

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("保存到数据库");
    msgBox.setText("请选择保存方式：");
    QPushButton *btnNew = msgBox.addButton("新建表格", QMessageBox::ActionRole);
    QPushButton *btnExisting = msgBox.addButton("保存到已有表格", QMessageBox::ActionRole);
    Q_UNUSED(btnExisting); // 按钮已添加到消息框，不需要直接使用
    QPushButton *btnCancel = msgBox.addButton("取消", QMessageBox::RejectRole);
    msgBox.exec();

    if (msgBox.clickedButton() == btnCancel)
        return;

    bool chooseNew = (msgBox.clickedButton() == btnNew);

    if (chooseNew)
    {
        // 始终弹出对话框创建新表格
        QString newName = QInputDialog::getText(this, "创建新表格", "请输入新的表格名：");
        if (newName.trimmed().isEmpty())
            return;
        QString displayName = QInputDialog::getText(this, "创建新表格", "请输入显示名称（可选）：");
        if (!m_db->createNewTable(newName, displayName))
        {
            QMessageBox::critical(this, "错误", QString("创建表格失败:\n%1").arg(m_db->lastError()));
            return;
        }
        tableName = newName;
    }
    else
    {
        // 保存到已有表格
        if (tables.isEmpty())
        {
            QMessageBox::information(this, "提示", "当前数据库中还没有任何表格，请先选择“新建表格”。");
            return;
        }

        QStringList items;
        for (const GraphTableInfo &info : tables)
        {
            QString display = info.displayName.isEmpty() ? info.tableName : info.displayName;
            QString line = QString("%1  (表名: %2, 节点: %3, 边: %4)")
                               .arg(display)
                               .arg(info.tableName)
                               .arg(info.nodeCount)
                               .arg(info.edgeCount);
            items << line;
        }

        bool ok = false;
        QString selected = QInputDialog::getItem(
            this,
            "保存到已有表格",
            "请选择要保存到的数据表格：",
            items,
            0,
            false,
            &ok);
        if (!ok || selected.isEmpty())
            return;

        int index = items.indexOf(selected);
        if (index < 0 || index >= tables.size())
            return;
        tableName = tables[index].tableName;
    }

    if (!m_db->setCurrentTable(tableName))
    {
        QMessageBox::critical(this, "错误", QString("切换到表格失败:\n%1").arg(m_db->lastError()));
        return;
    }

    if (!m_db->saveGraph(m_dijkstra, tableName))
    {
        QMessageBox::critical(this, "错误", QString("保存到数据库失败:\n%1").arg(m_db->lastError()));
        return;
    }

    QMessageBox::information(this, "成功", QString("已将当前图保存到表格 \"%1\"。").arg(tableName));
}

void DataManagementWindow::onPasteImport()
{
    QString text = m_pasteEdit->toPlainText();
    if (text.trimmed().isEmpty())
    {
        QMessageBox::warning(this, "提示", "请先在文本框中粘贴数据。");
        return;
    }

    // 使用简单的换行符分割
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    // 处理可能包含\r的情况
    for (QString &line : lines)
    {
        line = line.trimmed();
        if (line.endsWith('\r'))
            line.chop(1);
    }
    int successCount = 0;
    int failCount = 0;
    for (const QString &lineRaw : lines)
    {
        QString line = lineRaw.trimmed();
        if (line.isEmpty())
            continue;

        QStringList parts;
        if (line.contains('\t'))
            parts = line.split('\t', Qt::SkipEmptyParts);
        else if (line.contains(','))
            parts = line.split(',', Qt::SkipEmptyParts);
        else if (line.contains(';'))
            parts = line.split(';', Qt::SkipEmptyParts);
        else
            // 使用空格分割（支持多个空格）
            parts = line.split(' ', Qt::SkipEmptyParts);
            // 如果分割后元素不足，尝试用制表符分割
            if (parts.size() < 3)
            {
                parts = line.split('\t', Qt::SkipEmptyParts);
            }

        if (parts.size() < 3)
        {
            failCount++;
            continue;
        }

        bool ok1, ok2, ok3;
        long id1 = parts[0].toLong(&ok1);
        long id2 = parts[1].toLong(&ok2);
        long dist = parts[2].toLong(&ok3);
        if (!ok1 || !ok2 || !ok3)
        {
            failCount++;
            continue;
        }

        if (m_dijkstra->addNodesDist(id1, id2, dist))
        {
            if (m_db)
            {
                m_db->addOrUpdateNode(id1, m_dijkstra->getNodeLabel(id1));
                m_db->addOrUpdateNode(id2, m_dijkstra->getNodeLabel(id2));
                m_db->addOrUpdateEdge(id1, id2, dist);
            }
            successCount++;
        }
        else
        {
            failCount++;
        }
    }

    refreshData();
    QMessageBox::information(this, "导入完成",
                             QString("成功导入 %1 行，失败 %2 行。").arg(successCount).arg(failCount));
}

