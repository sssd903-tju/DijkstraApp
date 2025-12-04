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
class QFileDialog;
QT_END_NAMESPACE

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

private:
    void setupUI();
    void updateStatus();

    Dijkstra *m_dijkstra;

    // UI组件
    QPushButton *m_btnLoadFile;
    QPushButton *m_btnAddNode;
    QPushButton *m_btnCalculate;
    QPushButton *m_btnClear;

    QLineEdit *m_editNode1;
    QLineEdit *m_editNode2;
    QLineEdit *m_editDistance;
    QLineEdit *m_editStartNode;
    QLineEdit *m_editEndNode;

    QTextEdit *m_textResult;
    QTextEdit *m_textNodes;

    QLabel *m_labelStatus;
    QLabel *m_labelFile;
};

#endif // MAINWINDOW_H

