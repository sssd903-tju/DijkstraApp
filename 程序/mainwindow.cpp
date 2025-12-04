#include "mainwindow.h"
#include "visualizationwindow.h"
#include "datamanagementwindow.h"
#include "databasemanagementwindow.h"
#include "dijkstra_loader.h"
#include "graphdatabase.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QDir>
#include <QProgressBar>
#include <QStyle>
#include <QStandardPaths>
#include <QDebug>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dijkstra(new Dijkstra())
    , m_visualizationWindow(nullptr)
    , m_dataManagementWindow(nullptr)
    , m_databaseManagementWindow(nullptr)
    , m_fileLoader(new DijkstraLoader(this))
    , m_graphDb(new GraphDatabase(this))
    , m_loadedFileName(QString())
{
    setupUI();
    updateStatus();
    
    // 连接文件加载器信号
    connect(m_fileLoader, &DijkstraLoader::progress, this, &MainWindow::onFileLoadProgress);
    connect(m_fileLoader, &DijkstraLoader::finished, this, &MainWindow::onFileLoadFinished);
    connect(m_fileLoader, &DijkstraLoader::lineProcessed, this, &MainWindow::onFileLoadLineProcessed);
    ensureDatabaseLoaded();
}

MainWindow::~MainWindow()
{
    delete m_dijkstra;
    if (m_visualizationWindow)
        delete m_visualizationWindow;
    if (m_dataManagementWindow)
        delete m_dataManagementWindow;
    if (m_databaseManagementWindow)
        delete m_databaseManagementWindow;
    if (m_graphDb)
        delete m_graphDb;
}

void MainWindow::setupUI()
{
    setWindowTitle("Dijkstra最短路径算法");
    setMinimumSize(700, 500);
    resize(800, 600);

    // 统一灰色按钮样式
    setStyleSheet(
        "QMainWindow { background-color: #f5f5f5; }"
        "QGroupBox { font-weight: bold; border: 2px solid #cccccc; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QPushButton { background-color: #e0e0e0; color: #333333; border: 1px solid #bdbdbd; padding: 8px 16px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #d5d5d5; }"
        "QPushButton:pressed { background-color: #bdbdbd; }"
        "QLineEdit { padding: 6px; border: 1px solid #ddd; border-radius: 3px; }"
        "QTextEdit { border: 1px solid #ddd; border-radius: 3px; background-color: white; }"
    );

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 顶部状态栏 + 使用说明按钮
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_labelStatus = new QLabel("就绪", this);
    m_labelStatus->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    m_labelFile = new QLabel("未加载文件", this);
    m_labelFile->setStyleSheet("font-size: 12px; color: #666;");
    statusLayout->addWidget(m_labelStatus);
    statusLayout->addStretch();
    // 顺序：状态文字 → 空白 → 文件名 → 使用说明按钮（最右侧）
    statusLayout->addWidget(m_labelFile);
    m_btnHelp = new QPushButton("❓ 使用说明", this);
    statusLayout->addWidget(m_btnHelp);
    mainLayout->addLayout(statusLayout);

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    mainLayout->addWidget(m_progressBar);

    // 文件操作区域
    QGroupBox *fileGroup = new QGroupBox("文件 / 功能", this);
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_btnLoadFile = new QPushButton("📁 加载数据文件", this);
    m_btnClear = new QPushButton("🗑️ 清空数据", this);
    m_btnVisualization = new QPushButton("📊 打开可视化界面", this);
    m_btnDataManagement = new QPushButton("📋 打开数据管理", this);
    m_btnDatabaseManagement = new QPushButton("🗄️ 数据库管理", this);
    fileLayout->addWidget(m_btnLoadFile);
    fileLayout->addWidget(m_btnClear);
    fileLayout->addWidget(m_btnVisualization);
    fileLayout->addWidget(m_btnDataManagement);
    fileLayout->addWidget(m_btnDatabaseManagement);
    fileLayout->addStretch();
    fileGroup->setLayout(fileLayout);
    mainLayout->addWidget(fileGroup);

    connect(m_btnLoadFile, &QPushButton::clicked, this, &MainWindow::onLoadFile);
    connect(m_btnClear, &QPushButton::clicked, this, &MainWindow::onClearData);
    connect(m_btnVisualization, &QPushButton::clicked, this, &MainWindow::onOpenVisualization);
    connect(m_btnDataManagement, &QPushButton::clicked, this, &MainWindow::onOpenDataManagement);
    connect(m_btnDatabaseManagement, &QPushButton::clicked, this, &MainWindow::onOpenDatabaseManagement);

    // 手动添加节点区域
    QGroupBox *addGroup = new QGroupBox("手动添加节点关系", this);
    QGridLayout *addLayout = new QGridLayout();
    addLayout->addWidget(new QLabel("节点1 ID:", this), 0, 0);
    m_editNode1 = new QLineEdit(this);
    addLayout->addWidget(m_editNode1, 0, 1);
    addLayout->addWidget(new QLabel("节点2 ID:", this), 0, 2);
    m_editNode2 = new QLineEdit(this);
    addLayout->addWidget(m_editNode2, 0, 3);
    addLayout->addWidget(new QLabel("距离:", this), 0, 4);
    m_editDistance = new QLineEdit(this);
    addLayout->addWidget(m_editDistance, 0, 5);
    m_btnAddNode = new QPushButton("➕ 添加", this);
    addLayout->addWidget(m_btnAddNode, 0, 6);
    addGroup->setLayout(addLayout);
    mainLayout->addWidget(addGroup);

    connect(m_btnAddNode, &QPushButton::clicked, this, &MainWindow::onAddNode);

    // 计算最短路径区域
    QGroupBox *calcGroup = new QGroupBox("计算最短路径", this);
    QHBoxLayout *calcLayout = new QHBoxLayout();
    calcLayout->addWidget(new QLabel("起始节点:", this));
    m_editStartNode = new QLineEdit(this);
    calcLayout->addWidget(m_editStartNode);
    calcLayout->addWidget(new QLabel("终止节点:", this));
    m_editEndNode = new QLineEdit(this);
    calcLayout->addWidget(m_editEndNode);
    m_btnCalculate = new QPushButton("🔍 计算", this);
    calcLayout->addWidget(m_btnCalculate);
    calcLayout->addStretch();
    calcGroup->setLayout(calcLayout);
    mainLayout->addWidget(calcGroup);

    connect(m_btnCalculate, &QPushButton::clicked, this, &MainWindow::onCalculatePath);
    connect(m_btnHelp, &QPushButton::clicked, this, &MainWindow::onShowHelp);

    // 结果显示区域
    QGroupBox *resultGroup = new QGroupBox("计算结果", this);
    QVBoxLayout *resultLayout = new QVBoxLayout();
    m_textResult = new QTextEdit(this);
    m_textResult->setReadOnly(true);
    m_textResult->setMinimumHeight(200);
    resultLayout->addWidget(m_textResult);
    resultGroup->setLayout(resultLayout);
    mainLayout->addWidget(resultGroup);
}

void MainWindow::onLoadFile()
{
    QString defaultPath = QApplication::applicationDirPath();
    QDir dir(defaultPath);
    dir.cdUp();
    QString dijkstraPath = dir.absoluteFilePath("Dijkstra算法");
    
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择数据文件",
        dijkstraPath,
        "文本文件 (*.txt);;所有文件 (*.*)");

    if (fileName.isEmpty())
        return;

    // 清空现有数据
    m_dijkstra->clear();

    QFileInfo info(fileName);
    m_loadedFileName = info.fileName();
    m_labelFile->setText(QString("正在加载: %1").arg(m_loadedFileName));
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_labelStatus->setText("正在加载文件...");
    
    // 设置忙碌光标并使用多线程加载
    QApplication::setOverrideCursor(Qt::BusyCursor);
    m_fileLoader->loadFile(m_dijkstra, fileName);
}

void MainWindow::onAddNode()
{
    bool ok1, ok2, ok3;
    long id1 = m_editNode1->text().toLong(&ok1);
    long id2 = m_editNode2->text().toLong(&ok2);
    long dist = m_editDistance->text().toLong(&ok3);

    if (!ok1 || !ok2 || !ok3)
    {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字！");
        return;
    }

    if (dist <= 0)
    {
        QMessageBox::warning(this, "输入错误", "距离必须大于0！");
        return;
    }

    if (m_dijkstra->addNodesDist(id1, id2, dist))
    {
        m_editNode1->clear();
        m_editNode2->clear();
        m_editDistance->clear();
        updateStatus();
        syncEdgeToDatabase(id1, id2, dist);
        
        // 更新可视化窗口
        if (m_visualizationWindow)
        {
            m_visualizationWindow->updateGraph();
        }
        if (m_dataManagementWindow)
            m_dataManagementWindow->refreshData();
        
        QMessageBox::information(this, "成功", "节点关系添加成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", QString("添加失败:\n%1").arg(m_dijkstra->errorDescription()));
    }
}

void MainWindow::onCalculatePath()
{
    bool ok1, ok2;
    long startId = m_editStartNode->text().toLong(&ok1);
    long endId = m_editEndNode->text().toLong(&ok2);

    if (!ok1 || !ok2)
    {
        QMessageBox::warning(this, "输入错误", "请输入有效的节点ID！");
        return;
    }

    QApplication::setOverrideCursor(Qt::BusyCursor);

    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);

    long distance = 0;
    QVector<long> path;

    // 如果可视化窗口打开，让它处理动画；否则只更新进度条
    bool useVisualization = (m_visualizationWindow && m_visualizationWindow->isVisible());
    
    AnimationCallback animCallback;
    if (useVisualization)
    {
        // 可视化窗口会显示动画，这里同时更新进度条
        animCallback = [this](int /*nodeIndex*/, long /*dist*/, bool finished) {
            QMetaObject::invokeMethod(this, [this, finished]() {
                if (finished)
                {
                    m_progressBar->setValue(100);
                }
                else
                {
                    int current = m_progressBar->value();
                    if (current < 90)
                        m_progressBar->setValue(current + 1);
                }
                QApplication::processEvents();
            }, Qt::QueuedConnection);
        };
    }
    else
    {
        // 只有进度条
        animCallback = [this](int /*nodeIndex*/, long /*dist*/, bool finished) {
            QMetaObject::invokeMethod(this, [this, finished]() {
                if (finished)
                {
                    m_progressBar->setValue(100);
                }
                else
                {
                    int current = m_progressBar->value();
                    if (current < 90)
                        m_progressBar->setValue(current + 1);
                }
                QApplication::processEvents();
            }, Qt::QueuedConnection);
        };
    }

    int result = m_dijkstra->getDistance(startId, endId, distance, path, animCallback);
    
    // 如果可视化窗口打开，显示路径高亮（动画已在计算过程中显示）
    if (useVisualization && result > 0)
    {
        m_visualizationWindow->highlightPath(path);
    }

    m_progressBar->setVisible(false);
    QApplication::restoreOverrideCursor();

    if (result > 0)
    {
        QString resultText;
        resultText += QString("起始节点: %1\n").arg(startId);
        resultText += QString("终止节点: %1\n").arg(endId);
        resultText += QString("最短距离: %1\n\n").arg(distance);
        resultText += "路径: ";
        for (int i = 0; i < path.size(); i++)
        {
            resultText += QString::number(path[i]);
            if (i < path.size() - 1)
                resultText += " → ";
        }
        resultText += QString("\n\n路径节点数: %1").arg(path.size());
        m_textResult->setPlainText(resultText);
        m_labelStatus->setText(QString("计算完成: 距离=%1, 节点数=%2").arg(distance).arg(path.size()));

        // 更新可视化窗口显示路径
        if (m_visualizationWindow && m_visualizationWindow->isVisible())
        {
            m_visualizationWindow->highlightPath(path);
        }
    }
    else if (result == -1)
    {
        m_textResult->setPlainText(QString("起始节点: %1\n终止节点: %2\n\n无路径可达！").arg(startId).arg(endId));
        m_labelStatus->setText("无路径可达");
        QMessageBox::information(this, "结果", "两个节点之间无路径可达！");
    }
    else
    {
        m_textResult->setPlainText(QString("计算失败:\n%1").arg(m_dijkstra->errorDescription()));
        m_labelStatus->setText("计算失败");
        QMessageBox::critical(this, "错误", QString("计算失败:\n%1").arg(m_dijkstra->errorDescription()));
    }
}

void MainWindow::onShowHelp()
{
    QString text;
    text += "【主界面】\n";
    text += "1. 使用“加载数据文件”选择老师提供的数据文本，加载过程有进度条和忙碌鼠标。\n";
    text += "2. 数据会自动保存到本地 SQLite 数据库，并可在“数据管理/数据库管理”中选择表格。\n";
    text += "3. 在主界面输入起点和终点 ID，点击“计算”可运行 Dijkstra 最短路径，并在结果区显示。\n";
    text += "4. “清空数据”只清除内存中的图和界面，不会删除数据库中的表。\n\n";

    text += "【数据管理界面】\n";
    text += "1. 通过“从数据库加载”选择一张表，将节点/边加载到内存和各界面。\n";
    text += "2. 节点/边表支持分页浏览（约 1000 条/页），可上一页/下一页切换。\n";
    text += "3. 修改或粘贴数据后，可通过“保存到数据库”选择表格保存，或新建表保存。\n\n";

    text += "【可视化界面】\n";
    text += "1. 打开“可视化界面”可查看图形，支持鼠标滚轮/触控板缩放，以及放大/缩小/重置按钮。\n";
    text += "2. 左键单击节点：选中/取消选中，可连续多选；点击空白处可清空所有选中。\n";
    text += "3. 左键拖动节点（不按 Ctrl/Command）：只移动当前节点；按住 Ctrl/Command 拖动可整体移动已选中的节点。\n";
    text += "4. 选中恰好两个节点时，右侧“路径计算”区域会自动填入 ID，按钮变为黄色，点击即可计算并高亮最短路径。\n";
    text += "5. 大规模数据（节点数>500）时，不进行图形化绘制，只显示提示，以保证速度。\n";
    text += "6. 布局持久化：你在可视化界面中拖动调整过的节点位置会自动保存到数据库，与当前数据表关联。下次重新打开该表的可视化界面时，会自动恢复上次的布局。\n";

    QMessageBox::information(this, "使用说明", text);
}

void MainWindow::onClearData()
{
    int ret = QMessageBox::question(this, "确认", "确定要清空所有数据吗？",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        m_dijkstra->clear();
        m_textResult->clear();
        m_labelFile->setText("未加载文件");
        m_loadedFileName.clear();
        if (m_graphDb)
            m_graphDb->clear();
        updateStatus();
        
        // 更新可视化窗口
        if (m_visualizationWindow)
        {
            m_visualizationWindow->updateGraph();
        }
        if (m_dataManagementWindow)
            m_dataManagementWindow->refreshData();
        
        QMessageBox::information(this, "完成", "数据已清空！");
    }
}

void MainWindow::onOpenVisualization()
{
    if (!m_visualizationWindow)
    {
        m_visualizationWindow = new VisualizationWindow(m_dijkstra, m_graphDb, this);
        connect(m_visualizationWindow, &VisualizationWindow::graphDataChanged, this, [this]() {
            updateStatus();
            if (m_dataManagementWindow)
                m_dataManagementWindow->refreshData();
        });
        // 可视化计算结果同步到主界面
        connect(m_visualizationWindow, &VisualizationWindow::calcResultReady,
                this, [this](const QString &text) {
                    if (m_textResult)
                        m_textResult->setPlainText(text);
                });
    }
    m_visualizationWindow->show();
    m_visualizationWindow->raise();
    m_visualizationWindow->activateWindow();
}

void MainWindow::onOpenDataManagement()
{
    if (!m_dataManagementWindow)
    {
        m_dataManagementWindow = new DataManagementWindow(m_dijkstra, m_graphDb, this);
        m_dataManagementWindow->refreshData();
    }
    m_dataManagementWindow->show();
    m_dataManagementWindow->raise();
    m_dataManagementWindow->activateWindow();
}

void MainWindow::onOpenDatabaseManagement()
{
    if (!m_databaseManagementWindow)
    {
        m_databaseManagementWindow = new DatabaseManagementWindow(m_graphDb, m_dijkstra, this);
        connect(m_databaseManagementWindow, &DatabaseManagementWindow::tableSwitched, 
                this, &MainWindow::onTableSwitched);
    }
    m_databaseManagementWindow->show();
    m_databaseManagementWindow->raise();
    m_databaseManagementWindow->activateWindow();
}

void MainWindow::onTableSwitched(const QString &tableName)
{
    // 表格切换后，刷新数据管理窗口和可视化窗口
    if (m_dataManagementWindow)
        m_dataManagementWindow->refreshData();
    if (m_visualizationWindow)
        m_visualizationWindow->updateGraph();
    updateStatus();
    m_labelStatus->setText(QString("已切换到表格: %1").arg(tableName));
}

void MainWindow::onFileLoadProgress(float progress)
{
    m_progressBar->setValue((int)(progress * 100));
    QApplication::processEvents();
}

void MainWindow::onFileLoadFinished(bool success, const QString &error)
{
    m_progressBar->setVisible(false);
    QApplication::restoreOverrideCursor();
    
    if (success)
    {
        m_labelStatus->setText("文件加载成功");
        updateStatus();
        
        // 自动创建新表格并保存数据
        if (m_graphDb)
        {
            QFileInfo info(m_loadedFileName);
            QString baseName = info.baseName(); // 不带扩展名的文件名
            if (baseName.isEmpty())
                baseName = QString("table_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            
            // 创建新表格
            QString tableName = baseName;
            QString displayName = QString("%1 (%2)").arg(baseName).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
            
            if (m_graphDb->createNewTable(tableName, displayName))
            {
                // 设置为当前表格
                m_graphDb->setCurrentTable(tableName);
                
                // 保存数据到新表格
                if (m_graphDb->saveGraph(m_dijkstra, tableName))
                {
                    m_labelStatus->setText(QString("文件加载成功，已保存到表格: %1").arg(displayName));
                    
                    // 刷新数据库管理窗口
                    if (m_databaseManagementWindow)
                        m_databaseManagementWindow->refreshTableList();
                }
                else
                {
                    m_labelStatus->setText(QString("文件加载成功，但保存失败: %1").arg(m_graphDb->lastError()));
                }
            }
            else
            {
                // 如果创建失败（可能已存在），尝试使用现有表格
                if (m_graphDb->setCurrentTable(tableName))
                {
                    if (m_graphDb->saveGraph(m_dijkstra, tableName))
                    {
                        m_labelStatus->setText(QString("文件加载成功，已更新表格: %1").arg(displayName));
                    }
                }
                else
                {
                    m_labelStatus->setText(QString("文件加载成功，但创建表格失败: %1").arg(m_graphDb->lastError()));
                }
            }
        }
        
        // 更新可视化窗口
        if (m_visualizationWindow)
        {
            m_visualizationWindow->updateGraph();
        }
        if (m_dataManagementWindow)
            m_dataManagementWindow->refreshData();
        
        QMessageBox::information(this, "成功", QString("成功加载文件！\n节点数量: %1").arg(m_dijkstra->nodeCount()));
    }
    else
    {
        m_labelStatus->setText("加载失败");
        QMessageBox::critical(this, "错误", QString("加载文件失败:\n%1").arg(error));
    }
}

void MainWindow::onFileLoadLineProcessed(int lineCount)
{
    m_labelStatus->setText(QString("正在加载... 已处理 %1 行").arg(lineCount));
}

void MainWindow::updateStatus()
{
    int nodeCount = m_dijkstra->nodeCount();
    if (nodeCount == 0)
    {
        m_labelStatus->setText("就绪 - 无数据");
    }
    else
    {
        m_labelStatus->setText(QString("节点数量: %1").arg(nodeCount));
    }
}

void MainWindow::ensureDatabaseLoaded()
{
    if (!m_graphDb)
        return;

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty())
        dataDir = QDir::homePath() + "/.dijkstra_app";
    QDir dir(dataDir);
    if (!dir.exists())
        dir.mkpath(".");
    QString dbPath = dir.filePath("graph.db");

    if (!m_graphDb->initialize(dbPath))
    {
        qWarning() << "数据库初始化失败:" << m_graphDb->lastError();
        return;
    }

    if (m_graphDb->loadGraph(m_dijkstra) && m_dijkstra->nodeCount() > 0)
    {
        m_loadedFileName = tr("数据库存档");
        m_labelFile->setText(QString("已加载: %1").arg(m_loadedFileName));
        m_labelStatus->setText(QString("已从数据库载入 %1 个节点").arg(m_dijkstra->nodeCount()));
        updateStatus();
    }
}

void MainWindow::syncEdgeToDatabase(long id1, long id2, long distance)
{
    if (!m_graphDb)
        return;
    
    // 确保有当前表格，如果没有则创建一个默认表格
    QString currentTable = m_graphDb->currentTable();
    if (currentTable.isEmpty())
    {
        QString defaultTable = "default_table";
        if (m_graphDb->createNewTable(defaultTable, "默认表格"))
        {
            m_graphDb->setCurrentTable(defaultTable);
            currentTable = defaultTable;
        }
    }
    
    // 确保节点存在
    QString label1 = m_dijkstra->getNodeLabel(id1);
    if (label1.isEmpty())
        label1 = QString::number(id1);
    QString label2 = m_dijkstra->getNodeLabel(id2);
    if (label2.isEmpty())
        label2 = QString::number(id2);
    
    // 更新到当前表格
    m_graphDb->addOrUpdateNode(id1, label1, currentTable);
    m_graphDb->addOrUpdateNode(id2, label2, currentTable);
    m_graphDb->addOrUpdateEdge(id1, id2, distance, currentTable);
}

