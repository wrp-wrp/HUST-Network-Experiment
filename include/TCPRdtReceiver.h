#ifndef TCP_RDT_RECEIVER_H
#define TCP_RDT_RECEIVER_H

#include "RdtReceiver.h"

class TCPRdtReceiver : public RdtReceiver
{
private:
    int expectedSeqNum;                        // 期待接收的序号
    Packet lastAckPkt;                        // 上次发送的ACK包

public:
    TCPRdtReceiver();
    virtual ~TCPRdtReceiver();

    void receive(const Packet &packet) override;

private:
    void sendAck(int seqNum);                 // 发送ACK确认包
};

#endif