# 测试套件使用指南

## 快速开始

### 1. 运行完整测试（推荐）

```bash
./test_suite.sh
```

这将运行所有7项测试，生成完整的测试报告。

### 2. 快速单协议测试

```bash
# 测试GBN协议
./quick_test.sh gbn

# 测试SR协议
./quick_test.sh sr

# 测试TCP协议
./quick_test.sh tcp
```

## 测试项目对照表

| 编号 | 测试内容 | 分值 | 验证方法 |
|------|---------|------|---------|
| 1-1 | GBN协议正确性 | 45分 | 运行10次，对比输出文件 |
| 1-2 | GBN滑动窗口移动 | 5分 | 捕获窗口移动日志 |
| 2-3 | SR协议正确性 | 25分 | 运行10次，对比输出文件 |
| 2-4 | SR滑动窗口移动 | 5分 | 捕获窗口移动日志 |
| 3-5 | TCP协议正确性 | 10分 | 运行10次，对比输出文件 |
| 3-6 | TCP滑动窗口移动 | 5分 | 捕获窗口移动日志 |
| 3-7 | TCP快速重传 | 5分 | 检测快速重传事件 |

## 测试结果说明

### 1. 协议正确性测试

**通过标准：** 10次运行的output.txt文件内容完全一致

**如何验证：**
```bash
# 查看某次测试的输出对比
cd test_results/<timestamp>/gbn_correctness/
diff output_1.txt output_2.txt  # 应该没有任何差异
```

### 2. 滑动窗口移动测试

**通过标准：** 检测到窗口移动输出，且内容合理

**如何验证：**
```bash
# 查看GBN窗口移动记录
cat test_results/<timestamp>/gbn_window/window_movements.txt

# 输出示例：
# [GBN] 滑动窗口: base=1, nextSeqNum=4, 窗口内容: 1 2 3
# [GBN] 滑动窗口: base=2, nextSeqNum=5, 窗口内容: 2 3 4
```

### 3. TCP快速重传测试

**通过标准：** 检测到快速重传事件或重复ACK

**如何验证：**
```bash
# 查看快速重传事件
cat test_results/<timestamp>/tcp_fast_retransmit/fast_retransmit_events.txt

# 输出示例：
# [TCP] 快速重传触发: 收到3个重复ACK, base=10
```

## 查看特定测试结果

### 查看最新测试报告
```bash
ls -t test_results/ | head -1  # 获取最新测试目录
cat test_results/$(ls -t test_results/ | head -1)/test_report.txt
```

### 查看GBN测试
```bash
LATEST=$(ls -t test_results/ | head -1)
echo "=== GBN正确性测试 ==="
cat test_results/$LATEST/gbn_correctness/result.txt
echo "=== GBN窗口移动 ==="
cat test_results/$LATEST/gbn_window/window_movements.txt | head -10
```

### 查看SR测试
```bash
LATEST=$(ls -t test_results/ | head -1)
echo "=== SR正确性测试 ==="
cat test_results/$LATEST/sr_correctness/result.txt
echo "=== SR窗口移动 ==="
cat test_results/$LATEST/sr_window/window_movements.txt | head -10
```

### 查看TCP测试
```bash
LATEST=$(ls -t test_results/ | head -1)
echo "=== TCP正确性测试 ==="
cat test_results/$LATEST/tcp_correctness/result.txt
echo "=== TCP窗口移动 ==="
cat test_results/$LATEST/tcp_window/window_movements.txt | head -10
echo "=== TCP快速重传 ==="
cat test_results/$LATEST/tcp_fast_retransmit/fast_retransmit_events.txt
```

## 实验报告提交

### 需要提交的内容

1. **测试报告**
   ```bash
   test_results/<timestamp>/test_report.txt
   ```

2. **窗口移动日志**（每个协议）
   ```bash
   test_results/<timestamp>/gbn_window/window_movements.txt
   test_results/<timestamp>/sr_window/window_movements.txt
   test_results/<timestamp>/tcp_window/window_movements.txt
   ```

3. **快速重传日志**（TCP）
   ```bash
   test_results/<timestamp>/tcp_fast_retransmit/fast_retransmit_events.txt
   ```

4. **输出文件对比**（证明一致性）
   ```bash
   # 可以提交其中几次的输出文件作为证明
   test_results/<timestamp>/gbn_correctness/output_*.txt
   test_results/<timestamp>/sr_correctness/output_*.txt
   test_results/<timestamp>/tcp_correctness/output_*.txt
   ```

### 打包测试结果

```bash
# 获取最新测试结果目录
LATEST=$(ls -t test_results/ | head -1)

# 打包测试结果
tar -czf test_results_submission.tar.gz test_results/$LATEST/

# 或者只打包必需文件
mkdir submission
cp test_results/$LATEST/test_report.txt submission/
cp test_results/$LATEST/*/window_movements.txt submission/
cp test_results/$LATEST/tcp_fast_retransmit/fast_retransmit_events.txt submission/
tar -czf test_results_submission.tar.gz submission/
```

## 故障排查

### 中文乱码问题

如果在终端看到中文乱码，这是正常的，测试脚本仍能正确工作。可以：

1. 使用支持UTF-8的终端
2. 或者修改locale设置：
   ```bash
   export LANG=zh_CN.UTF-8
   export LC_ALL=zh_CN.UTF-8
   ```

### 测试失败调试

1. **输出不一致**
   ```bash
   # 查看具体差异
   cd test_results/<timestamp>/gbn_correctness/
   diff output_1.txt output_2.txt
   ```

2. **未检测到窗口移动**
   ```bash
   # 检查完整日志
   cat test_results/<timestamp>/gbn_window/window_output.txt | less
   
   # 搜索窗口相关输出
   grep -n "base" test_results/<timestamp>/gbn_window/window_output.txt
   ```

3. **编译问题**
   ```bash
   cd build
   rm -rf *
   cmake ..
   make -j4
   cd ..
   ```

## 高级用法

### 修改测试次数

编辑 `test_suite.sh`，修改 `run_count` 变量：
```bash
# 在test_gbn_correctness函数中
local run_count=10  # 改为20或其他值
```

### 只运行特定测试

编辑 `test_suite.sh`，在 `main` 函数中注释掉不需要的测试：
```bash
main() {
    test_gbn_correctness
    # test_gbn_window      # 注释掉此行跳过该测试
    test_sr_correctness
    # ...
}
```

### 自定义输出目录

修改测试脚本开头的目录设置：
```bash
TEST_RESULT_DIR="my_custom_results"
```

## 评分标准对应

| 测试项目 | 实验要求 | 测试脚本验证方法 | 满分条件 |
|---------|---------|----------------|---------|
| 1-1 | 多次运行输出一致 | `diff`比较所有输出文件 | 10次输出完全相同 |
| 1-2 | 窗口移动正确 | 解析`[GBN]滑动窗口`输出 | 检测到窗口移动记录 |
| 2-3 | 多次运行输出一致 | `diff`比较所有输出文件 | 10次输出完全相同 |
| 2-4 | 窗口移动正确 | 解析`[SR]滑动`输出 | 检测到窗口移动记录 |
| 3-5 | 多次运行输出一致 | `diff`比较所有输出文件 | 10次输出完全相同 |
| 3-6 | 窗口移动正确 | 解析`[TCP]滑动窗口`输出 | 检测到窗口移动记录 |
| 3-7 | 快速重传正确 | 检测快速重传事件 | 检测到快速重传触发 |

## 示例：完整测试流程

```bash
# 1. 确保项目已编译
cd build && make && cd ..

# 2. 运行完整测试套件
./test_suite.sh

# 3. 查看测试报告
cat test_results/$(ls -t test_results/ | head -1)/test_report.txt

# 4. 打包提交
LATEST=$(ls -t test_results/ | head -1)
tar -czf submission.tar.gz test_results/$LATEST/

# 5. 验证打包文件
tar -tzf submission.tar.gz | head -20
```

完成！现在可以将 `submission.tar.gz` 提交作为实验报告的一部分。
