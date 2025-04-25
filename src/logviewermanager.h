#ifndef LOGVIEWERMANAGER_H
#define LOGVIEWERMANAGER_H

#include <QObject>
#include <QString>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>
#include <QThread>

#include <assert.h>
#include <string>

#include "FS.h"
#include "Log.h"

class LogViewerManager;

namespace i2pd {
namespace qt {
namespace logviewer {

class Worker : public QObject
{
    Q_OBJECT
private:
    LogViewerManager &logViewerManager;
public:
    Worker(LogViewerManager &parameter1):logViewerManager(parameter1){}
private:
    QString pollAndShootATimerForInfiniteRetries();

public slots:
    void doWork1() {
        QString read=pollAndShootATimerForInfiniteRetries();
        emit resultReady(read);
    }

signals:
    void resultReady(QString read);
};

class Controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
    LogViewerManager& logViewerManager;
    int timerId;
public:
    Controller(LogViewerManager &parameter1);
    ~Controller() {
        if (timerId != 0) killTimer(timerId);
        workerThread.quit();
        workerThread.wait();
    }

signals:
    void operate1();

protected:
    void timerEvent(QTimerEvent */*event*/) {
        emit operate1();
    }
};

}
}
}

class LogViewerManager : public QObject
{
    Q_OBJECT
private:
    std::shared_ptr<std::iostream> logStream;
    QPlainTextEdit* logTextEdit;
    i2pd::qt::logviewer::Controller* controllerForBgThread;
public:
    explicit LogViewerManager(std::shared_ptr<std::iostream> logStream_,
                              QPlainTextEdit* logTextEdit_,
                              QObject *parent);
    virtual ~LogViewerManager(){}
    const i2pd::qt::logviewer::Controller& getControllerForBgThread() {
        assert(controllerForBgThread!=nullptr);
        return *controllerForBgThread;
    }
    const QPlainTextEdit* getLogTextEdit(){ return logTextEdit; }
    const std::shared_ptr<std::iostream> getLogStream(){ return logStream; }

public slots:
    void appendPlainText_atGuiThread(QString plainText) {
        if(plainText.length()==0)return;
        assert(logTextEdit != nullptr);
        int scrollPosVert =logTextEdit->verticalScrollBar()->value();
        int scrollPosHoriz=logTextEdit->horizontalScrollBar()->value();
        int scrollPosVertMax =logTextEdit->verticalScrollBar()->maximum();
        const int MAX_LINES=10*1024;
        logTextEdit->setMaximumBlockCount(MAX_LINES);
        logTextEdit->moveCursor(QTextCursor::End);
        logTextEdit->insertPlainText(plainText);
        if (scrollPosVert == scrollPosVertMax) {
            scrollPosVert =logTextEdit->verticalScrollBar()->maximum();
            scrollPosHoriz=logTextEdit->horizontalScrollBar()->minimum();
        }
        logTextEdit->verticalScrollBar()->setValue(scrollPosVert);
        logTextEdit->horizontalScrollBar()->setValue(scrollPosHoriz);
    }
};

#endif // LOGVIEWERMANAGER_H
