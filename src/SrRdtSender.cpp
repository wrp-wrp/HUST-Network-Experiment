#include "Global.h"
#include "SrRdtSender.h"
#include <iostream>

SrRdtSender::SrRdtSender() : base(0), nextSeqNum(0), waitingState(false) {
    sendBuffer.resize(WINDOW_SIZE);
    ackReceived.resize(WINDOW_SIZE, false);
}

SrRdtSender::~SrRdtSender() {
}

bool SrRdtSender::getWaitingState() {
    return waitingState;
}

bool SrRdtSender::isWindowFull() {
    return (nextSeqNum - base) >= WINDOW_SIZE;
}

bool SrRdtSender::isInWindow(int seqNum) {
    return (seqNum >= base && seqNum < base + WINDOW_SIZE);
}

bool SrRdtSender::send(const Message &message)
{
    if (isWindowFull()) {
        // 窗口已满，拒绝发送
        waitingState = true;
        return false;
    }

    sendPacket(message);
    return true;
}

void SrRdtSender::sendPacket(const Message &message)
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
    ackReceived[bufferIndex] = false;

    pUtils->printPacket("SR发送方发送报文", packet);
    
    // 为每个包启动独立的定时器
    pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);
    timerActive[nextSeqNum] = true;

    // 发送数据包
    pns->sendToNetworkLayer(RECEIVER, packet);
    
    // 更新下一个序号
    nextSeqNum++;
    
    // 检查窗口是否已满
    waitingState = isWindowFull();
}

void SrRdtSender::receive(const Packet &ackPkt)
{
    // 检查校验和
    int checkSum = pUtils->calculateCheckSum(ackPkt);
    
    if (checkSum == ackPkt.checksum) {
        pUtils->printPacket("SR发送方收到确认", ackPkt);
        
        // 检查ACK是否在当前窗口内
        if (isInWindow(ackPkt.acknum)) {
            int bufferIndex = ackPkt.acknum % WINDOW_SIZE;
            
            // 标记该包已收到ACK
            ackReceived[bufferIndex] = true;
            
            // 停止该包的定时器
            if (timerActive[ackPkt.acknum]) {
                pns->stopTimer(SENDER, ackPkt.acknum);
                timerActive[ackPkt.acknum] = false;
            }
            
            // 如果是窗口基包，尝试滑动窗口
            if (ackPkt.acknum == base) {
                // 先输出滑动前的窗口内容
                std::cout << "[SR] 滑动前窗口: base=" << base << ", nextSeqNum=" << nextSeqNum << ", 窗口内容: ";
                for (int i = base; i < nextSeqNum; ++i) {
                    int idx = i % WINDOW_SIZE;
                    std::cout << sendBuffer[idx].seqnum << "(" << (ackReceived[idx] ? "ACK" : "NAK") << ") ";
                }
                std::cout << std::endl;
                
                slideWindow();
                
                // 输出滑动后的窗口内容
                std::cout << "[SR] 滑动后窗口: base=" << base << ", nextSeqNum=" << nextSeqNum << ", 窗口内容: ";
                if (base < nextSeqNum) {
                    for (int i = base; i < nextSeqNum; ++i) {
                        int idx = i % WINDOW_SIZE;
                        std::cout << sendBuffer[idx].seqnum << "(" << (ackReceived[idx] ? "ACK" : "NAK") << ") ";
                    }
                } else {
                    std::cout << "(空窗口)";
                }
                std::cout << std::endl;
            }
            
            // 窗口可能有空间了，更新等待状态
            waitingState = isWindowFull();
        } else {
            // ACK不在窗口内，可能是重复ACK，忽略
            pUtils->printPacket("SR发送方收到窗口外的ACK，忽略", ackPkt);
        }
    } else {
        pUtils->printPacket("SR发送方收到损坏的确认包", ackPkt);
    }
}

void SrRdtSender::timeoutHandler(int seqNum)
{
    // 检查是否在当前窗口内
    if (isInWindow(seqNum)) {
        int bufferIndex = seqNum % WINDOW_SIZE;
        
        // 只重发超时的包（选择重传的核心特性）
        if (!ackReceived[bufferIndex]) {
            pUtils->printPacket("SR发送方定时器超时，重发单个包", sendBuffer[bufferIndex]);
            
            // 重发该包
            pns->sendToNetworkLayer(RECEIVER, sendBuffer[bufferIndex]);
            
            // 重新启动该包的定时器
            pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
            timerActive[seqNum] = true;
        }
    }
}

void SrRdtSender::slideWindow()
{
    // 从base开始，连续滑动窗口直到遇到未确认的包
    while (base < nextSeqNum) {
        int bufferIndex = base % WINDOW_SIZE;
        
        if (ackReceived[bufferIndex]) {
            // 该包已确认，可以滑动
            ackReceived[bufferIndex] = false; // 重置状态供下次使用
            base++;
        } else {
            // 遇到未确认的包，停止滑动
            break;
        }
    }
    
    pUtils->printPacket("SR发送方窗口滑动", Packet());
}