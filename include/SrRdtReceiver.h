#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H

#include "RdtReceiver.h"
#include <vector>
#include <map>

class SrRdtReceiver : public RdtReceiver
{
private:
    static const int WINDOW_SIZE = 4;           // 接收窗口大小
    int rcvBase;                               // 接收窗口基序号
    std::vector<Packet> rcvBuffer;             // 接收缓冲区
    std::vector<bool> packetReceived;          // 记录每个位置是否已收到包
    std::map<int, Packet> outOfOrderPackets;   // 存储乱序包

public:
    SrRdtReceiver();
    virtual ~SrRdtReceiver();

    void receive(const Packet &packet) override;

private:
    void sendAck(int seqNum);                  // 发送ACK确认包
    bool isInWindow(int seqNum);               // 检查序号是否在接收窗口内
    void deliverOrderedPackets();             // 按序递交包给应用层
    void slideWindow();                        // 滑动接收窗口
};

#endif