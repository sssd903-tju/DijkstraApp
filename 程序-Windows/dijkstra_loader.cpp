#include "dijkstra_loader.h"
#include "dijkstra.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

// ==================== FileLoaderWorker 实现 ====================

FileLoaderWorker::FileLoaderWorker(Dijkstra *dijkstra, const QString &fileName)
    : m_dijkstra(dijkstra)
    , m_fileName(fileName)
{
}

void FileLoaderWorker::load()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit finished(false, QString("无法打开文件: %1").arg(m_fileName));
        return;
    }

    // 获取文件大小用于进度计算
    qint64 fileSize = file.size();
    qint64 bytesRead = 0;

    QTextStream in(&file);
    // Windows兼容：确保使用UTF-8编码读取文件
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    // Qt5: 使用setCodec
    in.setCodec("UTF-8");
#endif
    int lineNumber = 0;
    int lastProgressLine = 0;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        lineNumber++;
        bytesRead += line.toUtf8().size() + 1;

        if (line.isEmpty())
            continue;

        // 分割行
        QStringList parts;
        if (line.contains('\t'))
            parts = line.split('\t', Qt::SkipEmptyParts);
        else if (line.contains(','))
            parts = line.split(',', Qt::SkipEmptyParts);
        else if (line.contains(';'))
            parts = line.split(';', Qt::SkipEmptyParts);
        else
        {
            parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() < 3)
            {
                parts = line.split('\t', Qt::SkipEmptyParts);
            }
        }

        if (parts.size() < 3)
        {
            file.close();
            emit finished(false, QString("第 %1 行格式错误：需要至少3个字段").arg(lineNumber));
            return;
        }

        bool ok1, ok2, ok3;
        long id1 = parts[0].toLong(&ok1);
        long id2 = parts[1].toLong(&ok2);
        long dist = parts[2].toLong(&ok3);

        if (!ok1 || !ok2 || !ok3)
        {
            file.close();
            emit finished(false, QString("第 %1 行数据格式错误：无法解析数字").arg(lineNumber));
            return;
        }

        if (!m_dijkstra->addNodesDist(id1, id2, dist))
        {
            file.close();
            emit finished(false, m_dijkstra->errorDescription());
            return;
        }

        // 每1000行更新一次进度和行数
        if (lineNumber - lastProgressLine >= 1000)
        {
            lastProgressLine = lineNumber;
            float progressValue = fileSize > 0 ? (float)bytesRead / fileSize : 0.0f;
            emit progress(progressValue);
            emit lineProcessed(lineNumber);
        }
    }

    file.close();
    emit progress(1.0f);
    emit finished(true, "");
}

// ==================== DijkstraLoader 实现 ====================

DijkstraLoader::DijkstraLoader(QObject *parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_cancelled(false)
{
}

DijkstraLoader::~DijkstraLoader()
{
    cancel();
}

void DijkstraLoader::loadFile(Dijkstra *dijkstra, const QString &fileName)
{
    // 取消之前的加载
    cancel();

    m_cancelled = false;
    m_fileName = fileName;

    // 创建新线程
    m_thread = new QThread(this);
    m_worker = new FileLoaderWorker(dijkstra, fileName);

    m_worker->moveToThread(m_thread);

    // 连接信号
    connect(m_thread, &QThread::started, m_worker, &FileLoaderWorker::load);
    connect(m_worker, &FileLoaderWorker::finished, this, &DijkstraLoader::onWorkerFinished);
    connect(m_worker, &FileLoaderWorker::progress, this, &DijkstraLoader::progress);
    connect(m_worker, &FileLoaderWorker::lineProcessed, this, &DijkstraLoader::lineProcessed);

    // 启动线程
    m_thread->start();
}

void DijkstraLoader::cancel()
{
    if (m_thread && m_thread->isRunning())
    {
        m_cancelled = true;
        m_thread->quit();
        m_thread->wait(3000); // 等待最多3秒
        if (m_thread->isRunning())
        {
            m_thread->terminate();
            m_thread->wait();
        }
    }

    if (m_worker)
    {
        delete m_worker;
        m_worker = nullptr;
    }

    if (m_thread)
    {
        delete m_thread;
        m_thread = nullptr;
    }
}

void DijkstraLoader::onWorkerFinished(bool success, const QString &error)
{
    if (m_thread)
    {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }

    if (m_worker)
    {
        delete m_worker;
        m_worker = nullptr;
    }

    emit finished(success, error);
}

