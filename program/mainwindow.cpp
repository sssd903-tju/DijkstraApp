#include "mainwindow.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dijkstra(new Dijkstra())
{
    setupUI();
    updateStatus();
}

MainWindow::~MainWindow()
{
    delete m_dijkstra;
}

void MainWindow::setupUI()
{
    setWindowTitle("Dijkstra最短路径算法 - Qt版本");
    setMinimumSize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 状态栏
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_labelStatus = new QLabel("就绪", this);
    m_labelFile = new QLabel("未加载文件", this);
    statusLayout->addWidget(m_labelStatus);
    statusLayout->addStretch();
    statusLayout->addWidget(m_labelFile);
    mainLayout->addLayout(statusLayout);

    // 文件加载区域
    QGroupBox *fileGroup = new QGroupBox("文件操作", this);
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_btnLoadFile = new QPushButton("加载数据文件", this);
    m_btnClear = new QPushButton("清空数据", this);
    fileLayout->addWidget(m_btnLoadFile);
    fileLayout->addWidget(m_btnClear);
    fileLayout->addStretch();
    fileGroup->setLayout(fileLayout);
    mainLayout->addWidget(fileGroup);

    connect(m_btnLoadFile, &QPushButton::clicked, this, &MainWindow::onLoadFile);
    connect(m_btnClear, &QPushButton::clicked, this, &MainWindow::onClearData);

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
    m_btnAddNode = new QPushButton("添加", this);
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
    m_btnCalculate = new QPushButton("计算", this);
    calcLayout->addWidget(m_btnCalculate);
    calcLayout->addStretch();
    calcGroup->setLayout(calcLayout);
    mainLayout->addWidget(calcGroup);

    connect(m_btnCalculate, &QPushButton::clicked, this, &MainWindow::onCalculatePath);

    // 结果显示区域
    QGroupBox *resultGroup = new QGroupBox("计算结果", this);
    QVBoxLayout *resultLayout = new QVBoxLayout();
    m_textResult = new QTextEdit(this);
    m_textResult->setReadOnly(true);
    m_textResult->setMinimumHeight(150);
    resultLayout->addWidget(m_textResult);
    resultGroup->setLayout(resultLayout);
    mainLayout->addWidget(resultGroup);

    // 节点列表区域
    QGroupBox *nodesGroup = new QGroupBox("节点信息", this);
    QVBoxLayout *nodesLayout = new QVBoxLayout();
    m_textNodes = new QTextEdit(this);
    m_textNodes->setReadOnly(true);
    m_textNodes->setMinimumHeight(100);
    nodesLayout->addWidget(m_textNodes);
    nodesGroup->setLayout(nodesLayout);
    mainLayout->addWidget(nodesGroup);
}

void MainWindow::onLoadFile()
{
    // 获取程序所在目录的父目录，然后指向Dijkstra算法文件夹
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

    if (m_dijkstra->loadFileData(fileName))
    {
        QFileInfo fileInfo(fileName);
        m_labelFile->setText(QString("已加载: %1").arg(fileInfo.fileName()));
        m_labelStatus->setText(QString("成功加载 %1 个节点").arg(m_dijkstra->nodeCount()));
        updateStatus();
        QMessageBox::information(this, "成功", QString("成功加载文件！\n节点数量: %1").arg(m_dijkstra->nodeCount()));
    }
    else
    {
        QMessageBox::critical(this, "错误", QString("加载文件失败:\n%1").arg(m_dijkstra->errorDescription()));
    }
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

    long distance = 0;
    QVector<long> path;

    int result = m_dijkstra->getDistance(startId, endId, distance, path);

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
                resultText += " -> ";
        }
        resultText += QString("\n\n路径节点数: %1").arg(path.size());
        m_textResult->setPlainText(resultText);
        m_labelStatus->setText(QString("计算完成: 距离=%1, 节点数=%2").arg(distance).arg(path.size()));
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

void MainWindow::onClearData()
{
    int ret = QMessageBox::question(this, "确认", "确定要清空所有数据吗？",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        m_dijkstra->clear();
        m_textResult->clear();
        m_textNodes->clear();
        m_labelFile->setText("未加载文件");
        updateStatus();
        QMessageBox::information(this, "完成", "数据已清空！");
    }
}

void MainWindow::updateStatus()
{
    int nodeCount = m_dijkstra->nodeCount();
    if (nodeCount == 0)
    {
        m_labelStatus->setText("就绪 - 无数据");
        m_textNodes->setPlainText("暂无节点数据");
    }
    else
    {
        m_labelStatus->setText(QString("节点数量: %1").arg(nodeCount));
        QString nodesText;
        nodesText += QString("节点总数: %1\n\n").arg(nodeCount);
        nodesText += "节点列表:\n";
        for (int i = 1; i <= nodeCount && i <= 100; i++) // 最多显示100个节点
        {
            long id = m_dijkstra->nodeID(i);
            nodesText += QString("%1 ").arg(id);
            if (i % 10 == 0)
                nodesText += "\n";
        }
        if (nodeCount > 100)
            nodesText += QString("\n... (还有 %1 个节点未显示)").arg(nodeCount - 100);
        m_textNodes->setPlainText(nodesText);
    }
}

