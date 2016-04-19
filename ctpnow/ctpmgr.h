#ifndef CTPMGR_H
#define CTPMGR_H

#include "ringbuffer.h"
#include <QMap>
#include <QObject>
#include <QQueue>
#include <QStringList>
#include <QTimer>
#include <functional>

class MdSm;
class TdSm;
class Logger;
class Profile;

struct CtpCmd {
    std::function<int(int, QString)> fn;
    quint32 delayTick;
    QString robotId;
    quint32 expires;
};

// 1.完成登录/自动登录/确认账单/订阅合约=
// 2.订阅什么合约由gateway统一确定，策略不管这事=
// 3.在queryInstrument之前，重新初始化后界面/重新初始化内存，1秒后开始查询就可以了，这样就不会有问题了=
//   onGotInstruments后开始刷新界面，queryInstrument+login都延迟一秒，用途之一就是这个=
class CtpMgr : public QObject {
    Q_OBJECT
public:
    explicit CtpMgr(QObject* parent = 0);
    void init();
    void shutdown();

    // 可跨线程调用=
    bool running();

    void insertContract(QString id, void* contract);
    void* getContract(QString id);
    void freeContracts();

    void initRingBuffer(int itemLen, QStringList ids);
    void freeRingBuffer();
    RingBuffer* getRingBuffer(QString id);
    void* getLatestTick(QString id);
    void* getPreLatestTick(QString id);

signals:
    void gotInstruments(QStringList ids);
    void gotTick(void* curTick, void* preTick);
    void gotAccount(double balance, double available, double margin, double closeProfit, double positionProfit);
    void tradeWillBegin();

public slots:
    void showVersion();
    void start(QString password);
    void stop();
    void queryAccount();
    void runCmd(CtpCmd* cmd);

private slots:
    void onGotInstruments(QStringList ids);
    void onMdSmStateChanged(int state);
    void onTdSmStateChanged(int state);
    void onRunCmdInterval();

private:
    Profile* profile();
    Logger* logger();
    bool initMdSm();
    void startMdSm();
    bool initTdSm();
    void startTdSm();
    void tryStartSubscrible();
    void resetCmds();

private:
    MdSm* mdsm_ = nullptr;
    bool mdsm_logined_ = false;
    TdSm* tdsm_ = nullptr;
    bool tdsm_logined_ = false;
    bool autoLoginTd_ = true;
    bool autoLoginMd_ = true;
    QString password_;

    QMap<QString, void*> contracts_;
    QMap<QString, RingBuffer*> rbs_;
    const int ringBufferLen_ = 256;

    QQueue<CtpCmd*> cmds_;
    int reqId_ = 0;
    QTimer* cmdRunnerTimer_ = nullptr;
};

#endif // CTPMGR_H
