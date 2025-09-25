#include "Global.h"
#include "SrRdtReceiver.h"

SrRdtReceiver::SrRdtReceiver() : rcvBase(0)
{
    rcvBuffer.resize(WINDOW_SIZE);
    packetReceived.resize(WINDOW_SIZE, false);
}

SrRdtReceiver::~SrRdtReceiver()
{
}

bool SrRdtReceiver::isInWindow(int seqNum)
{
    return (seqNum >= rcvBase && seqNum < rcvBase + WINDOW_SIZE);
}

void SrRdtReceiver::receive(const Packet &packet)
{
    // 检查校验和
    int checkSum = pUtils->calculateCheckSum(packet);
    
    if (checkSum == packet.checksum) {
        // 校验和正确
        pUtils->printPacket("SR接收方收到报文", packet);
        
        // 发送ACK（无论是否在窗口内都发送ACK）
        sendAck(packet.seqnum);
        
        // 检查是否在接收窗口内
        if (isInWindow(packet.seqnum)) {
            int bufferIndex = packet.seqnum % WINDOW_SIZE;
            
            // 检查是否已经收到过这个包
            if (!packetReceived[bufferIndex]) {
                // 存储包到缓冲区
                rcvBuffer[bufferIndex] = packet;
                packetReceived[bufferIndex] = true;
                
                pUtils->printPacket("SR接收方缓存报文", packet);
                
                // 如果是窗口基包，尝试递交连续的包
                if (packet.seqnum == rcvBase) {
                    deliverOrderedPackets();
                }
            } else {
                pUtils->printPacket("SR接收方收到重复报文", packet);
            }
        } else if (packet.seqnum < rcvBase) {
            // 这是一个已经确认过的包，重复ACK
            pUtils->printPacket("SR接收方收到已确认的重复报文", packet);
        } else {
            // 超出窗口范围的包，丢弃但仍发送ACK
            pUtils->printPacket("SR接收方收到超出窗口的报文，丢弃", packet);
        }
    } else {
        // 校验和错误，丢弃包，不发送ACK
        pUtils->printPacket("SR接收方收到损坏的报文，丢弃", packet);
    }
}

void SrRdtReceiver::sendAck(int seqNum)
{
    // 创建ACK包
    Packet ackPkt;
    ackPkt.seqnum = -1;         // 忽略该字段
    ackPkt.acknum = seqNum;     // 确认序号
    ackPkt.checksum = 0;
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
        ackPkt.payload[i] = '.';
    }
    ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
    
    pUtils->printPacket("SR接收方发送确认报文", ackPkt);
    pns->sendToNetworkLayer(SENDER, ackPkt);
}

void SrRdtReceiver::deliverOrderedPackets()
{
    // 从rcvBase开始，连续递交已收到的包
    while (rcvBase < rcvBase + WINDOW_SIZE) {
        int bufferIndex = rcvBase % WINDOW_SIZE;
        
        if (packetReceived[bufferIndex]) {
            // 该包已收到，可以递交给应用层
            Message msg;
            memcpy(msg.data, rcvBuffer[bufferIndex].payload, sizeof(rcvBuffer[bufferIndex].payload));
            pns->delivertoAppLayer(RECEIVER, msg);
            
            pUtils->printPacket("SR接收方递交报文给应用层", rcvBuffer[bufferIndex]);
            
            // 重置状态
            packetReceived[bufferIndex] = false;
            
            // 滑动窗口
            rcvBase++;
        } else {
            // 遇到未收到的包，停止递交
            break;
        }
    }
}

void SrRdtReceiver::slideWindow()
{
    // 窗口滑动在deliverOrderedPackets()中已经实现
    // 这里可以添加额外的窗口管理逻辑
}