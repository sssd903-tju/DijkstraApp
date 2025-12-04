#include "databasemanagementwindow.h"
#include "graphdatabase.h"
#include "dijkstra.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>

DatabaseManagementWindow::DatabaseManagementWindow(GraphDatabase *db, Dijkstra *dijkstra, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
    , m_dijkstra(dijkstra)
{
    setupUI();
    refreshTableList();
}

DatabaseManagementWindow::~DatabaseManagementWindow()
{
}

void DatabaseManagementWindow::setupUI()
{
    setWindowTitle("数据库管理界面");
    setMinimumSize(800, 600);
    resize(1000, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 左侧：表格列表
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QGroupBox *listGroup = new QGroupBox("数据表格列表", this);
    QVBoxLayout *listLayout = new QVBoxLayout();
    
    m_tableList = new QTableWidget(this);
    m_tableList->setColumnCount(5);
    m_tableList->setHorizontalHeaderLabels(QStringList() << "表格名" << "显示名" << "创建时间" << "节点数" << "边数");
    m_tableList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableList->horizontalHeader()->setStretchLastSection(true);
    m_tableList->setSortingEnabled(true);
    listLayout->addWidget(m_tableList);
    
    connect(m_tableList, &QTableWidget::itemSelectionChanged, this, &DatabaseManagementWindow::onTableSelectionChanged);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_btnUse = new QPushButton("使用此表格", this);
    m_btnCreate = new QPushButton("创建新表格", this);
    m_btnDelete = new QPushButton("删除表格", this);
    m_btnRefresh = new QPushButton("刷新", this);
    buttonLayout->addWidget(m_btnUse);
    buttonLayout->addWidget(m_btnCreate);
    buttonLayout->addWidget(m_btnDelete);
    buttonLayout->addWidget(m_btnRefresh);
    buttonLayout->addStretch();
    listLayout->addLayout(buttonLayout);
    
    connect(m_btnUse, &QPushButton::clicked, this, &DatabaseManagementWindow::onUseTable);
    connect(m_btnCreate, &QPushButton::clicked, this, &DatabaseManagementWindow::onCreateTable);
    connect(m_btnDelete, &QPushButton::clicked, this, &DatabaseManagementWindow::onDeleteTable);
    connect(m_btnRefresh, &QPushButton::clicked, this, &DatabaseManagementWindow::refreshTableList);
    
    listGroup->setLayout(listLayout);
    leftLayout->addWidget(listGroup);
    
    mainLayout->addLayout(leftLayout, 2);

    // 右侧：信息和控制
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    // 创建新表格
    QGroupBox *createGroup = new QGroupBox("创建新表格", this);
    QGridLayout *createLayout = new QGridLayout();
    createLayout->addWidget(new QLabel("表格名:", this), 0, 0);
    m_editTableName = new QLineEdit(this);
    m_editTableName->setPlaceholderText("输入表格名（英文、数字、下划线）");
    createLayout->addWidget(m_editTableName, 0, 1);
    createLayout->addWidget(new QLabel("显示名:", this), 1, 0);
    m_editDisplayName = new QLineEdit(this);
    m_editDisplayName->setPlaceholderText("输入显示名称（可选）");
    createLayout->addWidget(m_editDisplayName, 1, 1);
    createGroup->setLayout(createLayout);
    rightLayout->addWidget(createGroup);
    
    // 表格信息
    QGroupBox *infoGroup = new QGroupBox("当前表格信息", this);
    QVBoxLayout *infoLayout = new QVBoxLayout();
    m_infoText = new QTextEdit(this);
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(200);
    infoLayout->addWidget(m_infoText);
    infoGroup->setLayout(infoLayout);
    rightLayout->addWidget(infoGroup);
    
    // 状态
    m_statusLabel = new QLabel("就绪", this);
    rightLayout->addWidget(m_statusLabel);
    
    rightLayout->addStretch();
    mainLayout->addLayout(rightLayout, 1);
}

void DatabaseManagementWindow::refreshTableList()
{
    m_tableList->setRowCount(0);
    
    if (!m_db)
        return;

    QList<GraphTableInfo> tables = m_db->getAllTableInfos();
    m_tableList->setRowCount(tables.size());
    
    for (int i = 0; i < tables.size(); i++)
    {
        const GraphTableInfo &info = tables[i];
        m_tableList->setItem(i, 0, new QTableWidgetItem(info.tableName));
        m_tableList->setItem(i, 1, new QTableWidgetItem(info.displayName.isEmpty() ? info.tableName : info.displayName));
        m_tableList->setItem(i, 2, new QTableWidgetItem(info.createTime.toString("yyyy-MM-dd hh:mm:ss")));
        m_tableList->setItem(i, 3, new QTableWidgetItem(QString::number(info.nodeCount)));
        m_tableList->setItem(i, 4, new QTableWidgetItem(QString::number(info.edgeCount)));
    }
    
    updateTableInfo();
}

void DatabaseManagementWindow::onTableSelectionChanged()
{
    QList<QTableWidgetItem*> selected = m_tableList->selectedItems();
    bool hasSelection = !selected.isEmpty();
    m_btnUse->setEnabled(hasSelection);
    m_btnDelete->setEnabled(hasSelection);
    updateTableInfo();
}

void DatabaseManagementWindow::updateTableInfo()
{
    QList<QTableWidgetItem*> selected = m_tableList->selectedItems();
    if (selected.isEmpty())
    {
        QString currentTable = m_db->currentTable();
        if (currentTable.isEmpty())
        {
            m_infoText->setPlainText("未选择表格");
        }
        else
        {
            m_infoText->setPlainText(QString("当前使用表格: %1").arg(currentTable));
        }
        return;
    }
    
    int row = selected.first()->row();
    QString tableName = m_tableList->item(row, 0)->text();
    QString displayName = m_tableList->item(row, 1)->text();
    QString createTime = m_tableList->item(row, 2)->text();
    QString nodeCount = m_tableList->item(row, 3)->text();
    QString edgeCount = m_tableList->item(row, 4)->text();
    
    QString info;
    info += QString("表格名: %1\n").arg(tableName);
    info += QString("显示名: %1\n").arg(displayName);
    info += QString("创建时间: %1\n").arg(createTime);
    info += QString("节点数: %1\n").arg(nodeCount);
    info += QString("边数: %1\n").arg(edgeCount);
    
    QString currentTable = m_db->currentTable();
    if (tableName == currentTable)
    {
        info += "\n✓ 当前正在使用此表格";
    }
    
    m_infoText->setPlainText(info);
}

void DatabaseManagementWindow::onUseTable()
{
    QList<QTableWidgetItem*> selected = m_tableList->selectedItems();
    if (selected.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请先选择要使用的表格！");
        return;
    }
    
    int row = selected.first()->row();
    QString tableName = m_tableList->item(row, 0)->text();
    
    if (m_db->setCurrentTable(tableName))
    {
        // 加载数据到Dijkstra
        if (m_dijkstra && m_db->loadGraph(m_dijkstra, tableName))
        {
            m_statusLabel->setText(QString("已切换到表格: %1").arg(tableName));
            emit tableSwitched(tableName);
            QMessageBox::information(this, "成功", QString("已切换到表格: %1\n数据已加载到内存").arg(tableName));
        }
        else
        {
            QMessageBox::warning(this, "警告", QString("切换到表格 %1 成功，但加载数据失败:\n%2").arg(tableName).arg(m_db->lastError()));
        }
    }
    else
    {
        QMessageBox::critical(this, "错误", QString("切换表格失败:\n%1").arg(m_db->lastError()));
    }
}

void DatabaseManagementWindow::onCreateTable()
{
    QString tableName = m_editTableName->text().trimmed();
    if (tableName.isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "请输入表格名！");
        return;
    }
    
    QString displayName = m_editDisplayName->text().trimmed();
    if (displayName.isEmpty())
        displayName = tableName;
    
    if (m_db->createNewTable(tableName, displayName))
    {
        m_editTableName->clear();
        m_editDisplayName->clear();
        refreshTableList();
        m_statusLabel->setText(QString("成功创建表格: %1").arg(tableName));
        QMessageBox::information(this, "成功", QString("表格 %1 创建成功！").arg(tableName));
    }
    else
    {
        QMessageBox::critical(this, "错误", QString("创建表格失败:\n%1").arg(m_db->lastError()));
    }
}

void DatabaseManagementWindow::onDeleteTable()
{
    QList<QTableWidgetItem*> selected = m_tableList->selectedItems();
    if (selected.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请先选择要删除的表格！");
        return;
    }
    
    int row = selected.first()->row();
    QString tableName = m_tableList->item(row, 0)->text();
    QString displayName = m_tableList->item(row, 1)->text();
    
    int ret = QMessageBox::question(this, "确认删除", 
                                     QString("确定要删除表格 \"%1\" 吗？\n此操作不可恢复！").arg(displayName),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;
    
    if (m_db->deleteTable(tableName))
    {
        refreshTableList();
        m_statusLabel->setText(QString("已删除表格: %1").arg(tableName));
        QMessageBox::information(this, "成功", "表格已删除！");
    }
    else
    {
        QMessageBox::critical(this, "错误", QString("删除表格失败:\n%1").arg(m_db->lastError()));
    }
}

