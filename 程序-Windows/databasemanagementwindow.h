#ifndef DATABASEMANAGEMENTWINDOW_H
#define DATABASEMANAGEMENTWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>

class GraphDatabase;
class Dijkstra;

class DatabaseManagementWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DatabaseManagementWindow(GraphDatabase *db, Dijkstra *dijkstra, QWidget *parent = nullptr);
    ~DatabaseManagementWindow();

public slots:
    void refreshTableList();
    void onUseTable();
    void onCreateTable();
    void onDeleteTable();
    void onTableSelectionChanged();

signals:
    void tableSwitched(const QString &tableName);

private:
    void setupUI();
    void updateTableInfo();
    
    GraphDatabase *m_db;
    Dijkstra *m_dijkstra;
    
    // UI组件
    QTableWidget *m_tableList;
    QPushButton *m_btnUse;
    QPushButton *m_btnCreate;
    QPushButton *m_btnDelete;
    QPushButton *m_btnRefresh;
    QLineEdit *m_editTableName;
    QLineEdit *m_editDisplayName;
    QTextEdit *m_infoText;
    QLabel *m_statusLabel;
};

#endif // DATABASEMANAGEMENTWINDOW_H

