#include "Global.h"
#include "TCPRdtReceiver.h"

TCPRdtReceiver::TCPRdtReceiver() : expectedSeqNum(0) {
    // 初始化上次发送的ACK包
    lastAckPkt.seqnum = -1;     // 忽略该字段
    lastAckPkt.acknum = -1;     // 初始确认号为-1
    lastAckPkt.checksum = 0;
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
        lastAckPkt.payload[i] = '.';
    }
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

TCPRdtReceiver::~TCPRdtReceiver() {
}

void TCPRdtReceiver::receive(const Packet &packet) {
    // 检查校验和
    int checkSum = pUtils->calculateCheckSum(packet);
    
    if (checkSum == packet.checksum) {
        // 校验和正确
        if (packet.seqnum == expectedSeqNum) {
            // 收到期待的包
            pUtils->printPacket("TCP接收方正确收到期待的报文", packet);
            Message msg;
            memcpy(msg.data, packet.payload, sizeof(packet.payload));
            pns->delivertoAppLayer(RECEIVER, msg);
            
            // 发送ACK确认
            sendAck(packet.seqnum);
            
            // 更新期待的序号
            expectedSeqNum++;
        } else {
            // 收到的不是期待的包
            if (packet.seqnum < expectedSeqNum) {
                // 这是一个重复包，发送对"最高按序接收报文"的累计ACK（expectedSeqNum-1）
                pUtils->printPacket("TCP接收方收到重复报文", packet);
                if (expectedSeqNum > 0) {
                    sendAck(expectedSeqNum - 1);
                } else {
                    // 仍未按序接收过任何报文，重发上次ACK（ack=-1）
                    pUtils->printPacket("TCP接收方重发上次的确认报文", lastAckPkt);
                    pns->sendToNetworkLayer(SENDER, lastAckPkt);
                }
            } else {
                // 这是一个超前的包，丢弃并发送最近正确接收包的ACK
                pUtils->printPacket("TCP接收方收到超前报文，丢弃", packet);
                if (expectedSeqNum > 0) {
                    sendAck(expectedSeqNum - 1);
                } else {
                    // 如果还没有正确接收过任何包，发送上次的ACK
                    pUtils->printPacket("TCP接收方重发上次的确认报文", lastAckPkt);
                    pns->sendToNetworkLayer(SENDER, lastAckPkt);
                }
            }
        }
    } else {
        // 校验和错误
        pUtils->printPacket("TCP接收方收到损坏的报文", packet);
        // 发送上次的ACK
        pUtils->printPacket("TCP接收方重发上次的确认报文", lastAckPkt);
        pns->sendToNetworkLayer(SENDER, lastAckPkt);
    }
}

void TCPRdtReceiver::sendAck(int seqNum)
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
    
    // 更新上次发送的ACK
    lastAckPkt = ackPkt;
    
    pUtils->printPacket("TCP接收方发送确认报文", ackPkt);
    pns->sendToNetworkLayer(SENDER, ackPkt);
}