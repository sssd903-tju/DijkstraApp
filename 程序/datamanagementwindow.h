#ifndef DATAMANAGEMENTWINDOW_H
#define DATAMANAGEMENTWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTime>

class Dijkstra;
class GraphDatabase;

class DataManagementWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataManagementWindow(Dijkstra *dijkstra, GraphDatabase *db, QWidget *parent = nullptr);
    ~DataManagementWindow();

public slots:
    void refreshData();
    void onNodeFilterChanged();
    void onEdgeFilterChanged();
    void onAddNode();
    void onEditNode();
    void onDeleteNode();
    void onAddEdge();
    void onEditEdge();
    void onDeleteEdge();
    void onExportData();
    void onImportData();
    void onBatchDelete();
    void onLoadFromDatabase();
    void onSaveToDatabase();
    void onPasteImport();

private slots:
    void onNodeTableSelectionChanged();
    void onEdgeTableSelectionChanged();
    void onNodeTableDoubleClicked(int row, int column);
    void onEdgeTableDoubleClicked(int row, int column);
    void onNodePrevPage();
    void onNodeNextPage();
    void onEdgePrevPage();
    void onEdgeNextPage();

private:
    void setupUI();
    void populateNodeTable();
    void populateEdgeTable();
    void updateStatistics();
    void updateNodePageControls();
    void updateEdgePageControls();
    void applyNodeFilter();
    void applyEdgeFilter();
    bool validateNodeInput(long &id, QString &label);
    bool validateEdgeInput(long &id1, long &id2, long &distance);

    Dijkstra *m_dijkstra;
    GraphDatabase *m_db;

    // UI组件
    QTableWidget *m_nodeTable;
    QTableWidget *m_edgeTable;
    
    QLineEdit *m_nodeFilterEdit;
    QLineEdit *m_edgeFilterEdit;
    QComboBox *m_nodeSortCombo;
    QComboBox *m_edgeSortCombo;
    
    QPushButton *m_btnAddNode;
    QPushButton *m_btnEditNode;
    QPushButton *m_btnDeleteNode;
    QPushButton *m_btnAddEdge;
    QPushButton *m_btnEditEdge;
    QPushButton *m_btnDeleteEdge;
    QPushButton *m_btnBatchDelete;
    QPushButton *m_btnExport;
    QPushButton *m_btnImport;
    QPushButton *m_btnRefresh;
    QPushButton *m_btnLoadDb;
    QPushButton *m_btnSaveDb;
    QPushButton *m_btnPasteImport;
    QTextEdit *m_pasteEdit;
    
    QTextEdit *m_statsText;
    QLabel *m_statusLabel;
    
    // 当前选中的行
    int m_selectedNodeRow;
    int m_selectedEdgeRow;

    // 分页相关
    int m_nodePage;
    int m_edgePage;
    int m_nodeTotalPages;
    int m_edgeTotalPages;
    static const int PAGE_SIZE = 1000;

    QPushButton *m_btnNodePrevPage;
    QPushButton *m_btnNodeNextPage;
    QLabel *m_nodePageLabel;

    QPushButton *m_btnEdgePrevPage;
    QPushButton *m_btnEdgeNextPage;
    QLabel *m_edgePageLabel;

    // 刷新节流
    QTime m_lastRefreshTime;
};

#endif // DATAMANAGEMENTWINDOW_H

