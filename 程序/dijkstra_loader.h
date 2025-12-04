#ifndef DIJKSTRA_LOADER_H
#define DIJKSTRA_LOADER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <functional>

class Dijkstra;

// 文件加载工作线程
class FileLoaderWorker : public QObject
{
    Q_OBJECT

public:
    FileLoaderWorker(Dijkstra *dijkstra, const QString &fileName);

public slots:
    void load();

signals:
    void progress(float percent);
    void finished(bool success, const QString &error);
    void lineProcessed(int lineCount);

private:
    Dijkstra *m_dijkstra;
    QString m_fileName;
};

// 文件加载器（管理线程）
class DijkstraLoader : public QObject
{
    Q_OBJECT

public:
    explicit DijkstraLoader(QObject *parent = nullptr);
    ~DijkstraLoader();

    void loadFile(Dijkstra *dijkstra, const QString &fileName);
    void cancel();
    QString getFileName() const { return m_fileName; }

signals:
    void progress(float percent);
    void finished(bool success, const QString &error);
    void lineProcessed(int lineCount);

private slots:
    void onWorkerFinished(bool success, const QString &error);

private:
    QThread *m_thread;
    FileLoaderWorker *m_worker;
    bool m_cancelled;
    QString m_fileName;
};

#endif // DIJKSTRA_LOADER_H

