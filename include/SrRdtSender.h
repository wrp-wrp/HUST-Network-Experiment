#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H

#include "RdtSender.h"
#include <vector>
#include <map>

class SrRdtSender : public RdtSender
{
private:
    static const int WINDOW_SIZE = 4;           // 发送窗口大小
    int base;                                   // 窗口基序号
    int nextSeqNum;                            // 下一个要发送的序号
    std::vector<Packet> sendBuffer;            // 发送缓冲区
    std::vector<bool> ackReceived;             // 记录每个包是否已收到ACK
    std::map<int, bool> timerActive;           // 记录每个包的定时器状态
    bool waitingState;                         // 是否处于等待状态（窗口满）

public:
    SrRdtSender();
    virtual ~SrRdtSender();

    bool getWaitingState() override;
    bool send(const Message &message) override;
    void receive(const Packet &ackPkt) override;
    void timeoutHandler(int seqNum) override;

private:
    bool isWindowFull();                       // 检查窗口是否已满
    void sendPacket(const Message &message);   // 发送数据包
    bool isInWindow(int seqNum);              // 检查序号是否在窗口内
    void slideWindow();                        // 滑动窗口
};

#endif