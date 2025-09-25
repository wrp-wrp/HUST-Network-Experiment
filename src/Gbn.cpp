// Gbn.cpp : 定义GBN协议的控制台应用程序入口点。
//

#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "GbnRdtSender.h"
#include "GbnRdtReceiver.h"

int main(int argc, char* argv[])
{
    RdtSender *ps = new GbnRdtSender();
    RdtReceiver *pr = new GbnRdtReceiver();
    
    pns->setRunMode(0);  // VERBOSE模式
    // pns->setRunMode(1);  // 安静模式
    
    pns->init();
    pns->setRtdSender(ps);
    pns->setRtdReceiver(pr);
    
    // 设置输入输出文件路径（需要根据实际情况修改）
    pns->setInputFile("input.txt");
    pns->setOutputFile("output.txt");
    
    pns->start();
    
    delete ps;
    delete pr;
    delete pUtils;      // 指向唯一的工具类实例，只在main函数结束前delete
    delete pns;         // 指向唯一的模拟网络环境类实例，只在main函数结束前delete
    
    return 0;
}