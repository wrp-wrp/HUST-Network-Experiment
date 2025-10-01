#include "Global.h"
#include "TCPRdtSender.h"
#include <iostream>

TCPRdtSender::TCPRdtSender() : base(0), nextSeqNum(0), waitingState(false), lastAck(-1), lastAckCnt(0) {
    sendBuffer.resize(WINDOW_SIZE);
}

TCPRdtSender::~TCPRdtSender() {
}

bool TCPRdtSender::getWaitingState() {
    return waitingState;
}

bool TCPRdtSender::isWindowFull() {
    // 窗口满的条件：下一个序号与基序号之差达到窗口大小
    return (nextSeqNum - base) >= WINDOW_SIZE;
}

bool TCPRdtSender::send(const Message &message) {
    if (isWindowFull()) {
        // 窗口已满，拒绝发送
        waitingState = true;
        return false;
    }
    sendPacket(message);
    return true;
}

void TCPRdtSender::sendPacket(const Message &message)
{
    // 创建数据包
    Packet packet;
    packet.seqnum = nextSeqNum;
    packet.acknum = -1; // 忽略该字段
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);

    // 存储到发送缓冲区
    int bufferIndex = nextSeqNum % WINDOW_SIZE;
    sendBuffer[bufferIndex] = packet;

    pUtils->printPacket("TCP发送方发送报文", packet);
    
    // 如果是窗口中的第一个包，启动定时器
    if (base == nextSeqNum) {
        pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);
    }

    // 发送
    pns->sendToNetworkLayer(RECEIVER, packet);
    // 更新下一个序号
    nextSeqNum++;
    // 检查窗口是否已满
    waitingState = isWindowFull();
}

void TCPRdtSender::receive(const Packet &ackPkt)
{
    // 检查校验和
    int checkSum = pUtils->calculateCheckSum(ackPkt);
    if (checkSum == ackPkt.checksum) {
        pUtils->printPacket("TCP发送方收到确认", ackPkt);
        
        // 处理重复ACK
        if (ackPkt.acknum == lastAck) {
            lastAckCnt++;
            if (lastAckCnt >= 3) {
                std::cout << "[TCP] 快速重传触发: 收到3个重复ACK, base=" << base << std::endl;
                pUtils->printPacket("TCP发送方收到3个重复ACK，快速重传", ackPkt);
                // 快速重传最早未确认的包
                resendPackets();
                // 重置计数器
                lastAckCnt = 0;
            }
        } else {
            lastAck = ackPkt.acknum;
            lastAckCnt = 0; // 收到新ACK，重复计数重置为0
        }

        // 累积确认：确认序号为n的ACK表示序号0到n的所有包都已正确接收
        if (ackPkt.acknum >= base) {
            // 停止当前定时器
            pns->stopTimer(SENDER, base);
            // 更新窗口基序号
            int oldBase = base;
            base = ackPkt.acknum + 1;
            // 输出滑动窗口内容
            std::cout << "[TCP] 滑动窗口: base=" << base << ", nextSeqNum=" << nextSeqNum << ", 窗口内容: ";
            if (base < nextSeqNum) {
                for (int i = base; i < nextSeqNum; ++i) {
                    int idx = i % WINDOW_SIZE;
                    std::cout << sendBuffer[idx].seqnum << " ";
                }
            } else {
                std::cout << "(空窗口)";
            }
            std::cout << std::endl;
            // 如果还有未确认的包，重新启动定时器
            if (base < nextSeqNum) {
                pns->startTimer(SENDER, Configuration::TIME_OUT, base);
            }
            // 窗口可能有空间了，更新等待状态
            waitingState = isWindowFull();
        }
    } else {
        pUtils->printPacket("TCP发送方收到损坏的确认包", ackPkt);
    }
}

void TCPRdtSender::timeoutHandler(int seqNum)
{
    pUtils->printPacket("TCP发送方定时器超时，重发最靠前的包", sendBuffer[seqNum % WINDOW_SIZE]);
    
    // 停止定时器
    pns->stopTimer(SENDER, seqNum);
    
    // 重发所有未确认的包
    resendPackets();
    
    // 重新启动定时器
    pns->startTimer(SENDER, Configuration::TIME_OUT, base);
}

void TCPRdtSender::resendPackets()
{
    // // 重发从base到nextSeqNum-1的所有包
    // for (int i = base; i < nextSeqNum; i++) {
    //     int bufferIndex = i % WINDOW_SIZE;
    //     pUtils->printPacket("TCP发送方重发报文", sendBuffer[bufferIndex]);
    //     pns->sendToNetworkLayer(RECEIVER, sendBuffer[bufferIndex]);
    // }
    // 重发 base
    
    int baseIndex = base % WINDOW_SIZE;
    pUtils->printPacket("TCP发送方重发报文", sendBuffer[baseIndex]);
    pns->sendToNetworkLayer(RECEIVER, sendBuffer[baseIndex]);
}