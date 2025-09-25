#ifndef TCP_RDT_SENDER_H
#define TCP_RDT_SENDER_H

#include "RdtSender.h"
#include <vector>

class TCPRdtSender : public RdtSender
{
private:
    static const int WINDOW_SIZE = 4;           // 窗口大小
    int base;                                   // 窗口基序号
    int nextSeqNum;                            // 下一个要发送的序号
    std::vector<Packet> sendBuffer;            // 发送缓冲区，存储已发送但未确认的数据包
    bool waitingState;                         // 是否处于等待状态
    int lastAck;
    int lastAckCnt;

public:
    TCPRdtSender();
    virtual ~TCPRdtSender();

    bool getWaitingState() override;
    bool send(const Message &message) override;
    void receive(const Packet &ackPkt) override;
    void timeoutHandler(int seqNum) override;

private:
    bool isWindowFull();                       // 检查窗口是否已满
    void sendPacket(const Message &message);   // 发送数据包
    void resendPackets();                      // 重发所有未确认的数据包
};

#endif