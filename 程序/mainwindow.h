#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "dijkstra.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QLineEdit;
class QTextEdit;
class QLabel;
class QProgressBar;
class QGroupBox;
QT_END_NAMESPACE

class VisualizationWindow;
class DataManagementWindow;
class DatabaseManagementWindow;
class DijkstraLoader;
class GraphDatabase;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadFile();
    void onAddNode();
    void onCalculatePath();
    void onClearData();
    void onOpenVisualization();
    void onOpenDataManagement();
    void onOpenDatabaseManagement();
    void onShowHelp();
    void onFileLoadProgress(float progress);
    void onFileLoadFinished(bool success, const QString &error);
    void onFileLoadLineProcessed(int lineCount);
    void onTableSwitched(const QString &tableName);

private:
    void setupUI();
    void updateStatus();
    void ensureDatabaseLoaded();
    void syncEdgeToDatabase(long id1, long id2, long distance);

    Dijkstra *m_dijkstra;
    VisualizationWindow *m_visualizationWindow;
    DataManagementWindow *m_dataManagementWindow;
    DatabaseManagementWindow *m_databaseManagementWindow;
    DijkstraLoader *m_fileLoader;
    GraphDatabase *m_graphDb;

    // UI组件
    QPushButton *m_btnLoadFile;
    QPushButton *m_btnAddNode;
    QPushButton *m_btnCalculate;
    QPushButton *m_btnClear;
    QPushButton *m_btnVisualization;
    QPushButton *m_btnDataManagement;
    QPushButton *m_btnDatabaseManagement;
    QPushButton *m_btnHelp;

    QLineEdit *m_editNode1;
    QLineEdit *m_editNode2;
    QLineEdit *m_editDistance;
    QLineEdit *m_editStartNode;
    QLineEdit *m_editEndNode;

    QTextEdit *m_textResult;

    QLabel *m_labelStatus;
    QLabel *m_labelFile;
    QProgressBar *m_progressBar;
    QString m_loadedFileName;
};

#endif // MAINWINDOW_H

