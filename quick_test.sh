#!/bin/bash

# 快速单协议测试脚本

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

show_help() {
    echo "用法: ./quick_test.sh [协议名]"
    echo ""
    echo "协议名:"
    echo "  gbn     - 测试 Go-Back-N 协议"
    echo "  sr      - 测试 Selective Repeat 协议"
    echo "  tcp     - 测试 TCP 协议"
    echo "  all     - 运行完整测试套件"
    echo ""
    echo "示例:"
    echo "  ./quick_test.sh gbn    # 单独测试GBN"
    echo "  ./quick_test.sh all    # 运行所有测试"
    echo ""
}

test_protocol() {
    local protocol=$1
    local output_dir="test_results/quick_test_${protocol}_$(date +%Y%m%d_%H%M%S)"
    
    mkdir -p "$output_dir"
    
    echo -e "${BLUE}[INFO]${NC} 测试 ${protocol} 协议..."
    echo -e "${BLUE}[INFO]${NC} 结果将保存到: $output_dir"
    echo ""
    
    # 运行5次测试
    for i in {1..5}; do
        echo -e "${BLUE}[INFO]${NC} 第 $i 次运行..."
        ./bin/${protocol} > "$output_dir/run_${i}.log" 2>&1
        cp output.txt "$output_dir/output_${i}.txt"
    done
    
    # 比较结果
    echo -e "${BLUE}[INFO]${NC} 比较输出文件..."
    local all_same=true
    
    for i in {2..5}; do
        if ! diff -q "$output_dir/output_1.txt" "$output_dir/output_${i}.txt" > /dev/null; then
            echo -e "${RED}[ERROR]${NC} 第 $i 次运行的输出与第1次不同"
            all_same=false
        fi
    done
    
    if $all_same; then
        echo -e "${GREEN}[SUCCESS]${NC} ${protocol} 协议测试通过！所有运行结果一致"
    else
        echo -e "${RED}[FAIL]${NC} ${protocol} 协议测试失败！输出不一致"
    fi
    
    # 检查窗口移动
    echo ""
    echo -e "${BLUE}[INFO]${NC} 分析窗口移动..."
    
    case $protocol in
        gbn)
            local window_count=$(grep -c "\[GBN\] 滑动窗口" "$output_dir/run_1.log")
            grep "\[GBN\] 滑动窗口" "$output_dir/run_1.log" > "$output_dir/window_movements.txt"
            ;;
        sr)
            local window_count=$(grep -c "\[SR\] 滑动" "$output_dir/run_1.log")
            grep "\[SR\] 滑动" "$output_dir/run_1.log" > "$output_dir/window_movements.txt"
            ;;
        tcp)
            local window_count=$(grep -c "\[TCP\] 滑动窗口" "$output_dir/run_1.log")
            grep "\[TCP\] 滑动窗口" "$output_dir/run_1.log" > "$output_dir/window_movements.txt"
            
            # TCP额外检查快速重传
            local fast_retrans=$(grep -c "快速重传" "$output_dir/run_1.log")
            echo -e "${BLUE}[INFO]${NC} 检测到 $fast_retrans 次快速重传"
            grep -E "(快速重传|重复ACK)" "$output_dir/run_1.log" > "$output_dir/fast_retransmit.txt"
            ;;
    esac
    
    echo -e "${BLUE}[INFO]${NC} 检测到 $window_count 次窗口移动"
    
    if [ $window_count -gt 0 ]; then
        echo -e "${GREEN}[SUCCESS]${NC} 窗口移动记录已保存"
        echo ""
        echo "前5次窗口移动:"
        head -5 "$output_dir/window_movements.txt"
    fi
    
    echo ""
    echo -e "${GREEN}[INFO]${NC} 测试完成！详细结果请查看: $output_dir"
    echo ""
}

# 主程序
case "${1:-help}" in
    gbn|sr|tcp)
        if [ ! -f "bin/$1" ]; then
            echo -e "${RED}[ERROR]${NC} 可执行文件 bin/$1 不存在"
            echo "请先编译项目: cd build && make && cd .."
            exit 1
        fi
        test_protocol "$1"
        ;;
    all)
        ./test_suite.sh
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo -e "${RED}[ERROR]${NC} 未知的协议: $1"
        echo ""
        show_help
        exit 1
        ;;
esac
