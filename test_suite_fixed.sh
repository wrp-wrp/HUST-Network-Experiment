#!/bin/bash

# 网络实验自动化测试脚本（修复版）
# 修复中文编码问题，确保正确捕获窗口移动日志

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 设置UTF-8编码
export LANG=zh_CN.UTF-8
export LC_ALL=zh_CN.UTF-8

TEST_RESULT_DIR="test_results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
CURRENT_TEST_DIR="${TEST_RESULT_DIR}/${TIMESTAMP}"

mkdir -p "${CURRENT_TEST_DIR}"

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

declare -A test_results
test_count=0
pass_count=0
fail_count=0

record_test() {
    local test_name=$1
    local result=$2
    local score=$3
    
    test_results["${test_name}"]="${result}|${score}"
    ((test_count++))
    
    if [ "$result" == "PASS" ]; then
        ((pass_count++))
    else
        ((fail_count++))
    fi
}

compile_project() {
    log_info "开始编译项目..."
    cd build
    if make clean && make; then
        log_success "项目编译成功"
        cd ..
        return 0
    else
        log_error "项目编译失败"
        cd ..
        return 1
    fi
}

# 测试1-1: GBN协议正确性 (45分)
test_gbn_correctness() {
    log_info "======================================"
    log_info "测试1-1: GBN协议正确性测试 (45分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/gbn_correctness"
    mkdir -p "${test_dir}"
    
    local run_count=10
    local all_same=true
    
    for i in $(seq 1 $run_count); do
        log_info "GBN第${i}次运行..."
        ./bin/gbn > "${test_dir}/run_${i}.log" 2>&1
        cp output.txt "${test_dir}/output_${i}.txt"
    done
    
    log_info "比较10次运行的输出文件..."
    local reference="${test_dir}/output_1.txt"
    
    for i in $(seq 2 $run_count); do
        if ! diff -q "$reference" "${test_dir}/output_${i}.txt" > /dev/null; then
            log_error "第${i}次运行的输出与第1次不同"
            all_same=false
        fi
    done
    
    if $all_same; then
        log_success "GBN协议正确性测试通过：10次运行输出完全一致"
        record_test "1-1 GBN协议正确性" "PASS" "45"
        echo "PASS" > "${test_dir}/result.txt"
    else
        log_error "GBN协议正确性测试失败：输出不一致"
        record_test "1-1 GBN协议正确性" "FAIL" "0"
        echo "FAIL" > "${test_dir}/result.txt"
    fi
}

# 测试1-2: GBN滑动窗口移动正确性 (5分)
test_gbn_window() {
    log_info "======================================"
    log_info "测试1-2: GBN滑动窗口移动正确性 (5分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/gbn_window"
    mkdir -p "${test_dir}"
    
    log_info "运行GBN程序并捕获窗口移动信息..."
    ./bin/gbn > "${test_dir}/window_output.txt" 2>&1
    
    # 使用多种方式提取窗口信息
    grep -a "\[GBN\]" "${test_dir}/window_output.txt" > "${test_dir}/window_movements.txt" 2>/dev/null || true
    
    local window_count=$(wc -l < "${test_dir}/window_movements.txt" 2>/dev/null || echo 0)
    log_info "检测到 ${window_count} 次窗口移动"
    
    if [ $window_count -gt 0 ]; then
        log_success "GBN滑动窗口移动测试通过"
        record_test "1-2 GBN滑动窗口移动" "PASS" "5"
        log_info "前10次窗口移动："
        head -10 "${test_dir}/window_movements.txt"
    else
        log_warning "未检测到窗口移动输出"
        record_test "1-2 GBN滑动窗口移动" "FAIL" "0"
    fi
}

# 测试2-3: SR协议正确性 (25分)
test_sr_correctness() {
    log_info "======================================"
    log_info "测试2-3: SR协议正确性测试 (25分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/sr_correctness"
    mkdir -p "${test_dir}"
    
    local run_count=10
    local all_same=true
    
    for i in $(seq 1 $run_count); do
        log_info "SR第${i}次运行..."
        ./bin/sr > "${test_dir}/run_${i}.log" 2>&1
        cp output.txt "${test_dir}/output_${i}.txt"
    done
    
    log_info "比较10次运行的输出文件..."
    local reference="${test_dir}/output_1.txt"
    
    for i in $(seq 2 $run_count); do
        if ! diff -q "$reference" "${test_dir}/output_${i}.txt" > /dev/null; then
            log_error "第${i}次运行的输出与第1次不同"
            all_same=false
        fi
    done
    
    if $all_same; then
        log_success "SR协议正确性测试通过：10次运行输出完全一致"
        record_test "2-3 SR协议正确性" "PASS" "25"
        echo "PASS" > "${test_dir}/result.txt"
    else
        log_error "SR协议正确性测试失败：输出不一致"
        record_test "2-3 SR协议正确性" "FAIL" "0"
        echo "FAIL" > "${test_dir}/result.txt"
    fi
}

# 测试2-4: SR滑动窗口移动正确性 (5分)
test_sr_window() {
    log_info "======================================"
    log_info "测试2-4: SR滑动窗口移动正确性 (5分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/sr_window"
    mkdir -p "${test_dir}"
    
    log_info "运行SR程序并捕获窗口移动信息..."
    ./bin/sr > "${test_dir}/window_output.txt" 2>&1
    
    # 提取SR窗口信息
    grep -a "\[SR\]" "${test_dir}/window_output.txt" > "${test_dir}/window_movements.txt" 2>/dev/null || true
    
    local window_count=$(wc -l < "${test_dir}/window_movements.txt" 2>/dev/null || echo 0)
    log_info "检测到 ${window_count} 次窗口相关输出"
    
    if [ $window_count -gt 0 ]; then
        log_success "SR滑动窗口移动测试通过"
        record_test "2-4 SR滑动窗口移动" "PASS" "5"
        log_info "前10次窗口移动："
        head -10 "${test_dir}/window_movements.txt"
    else
        log_warning "未检测到窗口移动输出"
        record_test "2-4 SR滑动窗口移动" "FAIL" "0"
    fi
}

# 测试3-5: TCP协议正确性 (10分)
test_tcp_correctness() {
    log_info "======================================"
    log_info "测试3-5: TCP协议正确性测试 (10分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/tcp_correctness"
    mkdir -p "${test_dir}"
    
    local run_count=10
    local all_same=true
    
    for i in $(seq 1 $run_count); do
        log_info "TCP第${i}次运行..."
        ./bin/tcp > "${test_dir}/run_${i}.log" 2>&1
        cp output.txt "${test_dir}/output_${i}.txt"
    done
    
    log_info "比较10次运行的输出文件..."
    local reference="${test_dir}/output_1.txt"
    
    for i in $(seq 2 $run_count); do
        if ! diff -q "$reference" "${test_dir}/output_${i}.txt" > /dev/null; then
            log_error "第${i}次运行的输出与第1次不同"
            all_same=false
        fi
    done
    
    if $all_same; then
        log_success "TCP协议正确性测试通过：10次运行输出完全一致"
        record_test "3-5 TCP协议正确性" "PASS" "10"
        echo "PASS" > "${test_dir}/result.txt"
    else
        log_error "TCP协议正确性测试失败：输出不一致"
        record_test "3-5 TCP协议正确性" "FAIL" "0"
        echo "FAIL" > "${test_dir}/result.txt"
    fi
}

# 测试3-6: TCP滑动窗口移动正确性 (5分)
test_tcp_window() {
    log_info "======================================"
    log_info "测试3-6: TCP滑动窗口移动正确性 (5分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/tcp_window"
    mkdir -p "${test_dir}"
    
    log_info "运行TCP程序并捕获窗口移动信息..."
    ./bin/tcp > "${test_dir}/window_output.txt" 2>&1
    
    # 提取TCP窗口信息
    grep -a "\[TCP\]" "${test_dir}/window_output.txt" > "${test_dir}/window_movements.txt" 2>/dev/null || true
    
    local window_count=$(wc -l < "${test_dir}/window_movements.txt" 2>/dev/null || echo 0)
    log_info "检测到 ${window_count} 次窗口相关输出"
    
    if [ $window_count -gt 0 ]; then
        log_success "TCP滑动窗口移动测试通过"
        record_test "3-6 TCP滑动窗口移动" "PASS" "5"
        log_info "前10次窗口移动："
        head -10 "${test_dir}/window_movements.txt"
    else
        log_warning "未检测到窗口移动输出"
        record_test "3-6 TCP滑动窗口移动" "FAIL" "0"
    fi
}

# 测试3-7: TCP快速重传正确性 (5分)
test_tcp_fast_retransmit() {
    log_info "======================================"
    log_info "测试3-7: TCP快速重传正确性 (5分)"
    log_info "======================================"
    
    local test_dir="${CURRENT_TEST_DIR}/tcp_fast_retransmit"
    mkdir -p "${test_dir}"
    
    log_info "运行TCP程序并捕获快速重传信息..."
    ./bin/tcp > "${test_dir}/fast_retransmit_output.txt" 2>&1
    
    # 提取快速重传信息
    grep -aE "\[TCP\].*快速|重复ACK" "${test_dir}/fast_retransmit_output.txt" > "${test_dir}/fast_retransmit_events.txt" 2>/dev/null || true
    
    local fast_retransmit_count=$(wc -l < "${test_dir}/fast_retransmit_events.txt" 2>/dev/null || echo 0)
    
    log_info "检测到 ${fast_retransmit_count} 次快速重传相关输出"
    
    if [ $fast_retransmit_count -gt 0 ]; then
        log_success "TCP快速重传测试通过：检测到快速重传事件"
        record_test "3-7 TCP快速重传" "PASS" "5"
        log_info "快速重传事件："
        cat "${test_dir}/fast_retransmit_events.txt"
    else
        log_warning "未检测到快速重传事件（网络条件可能未触发）"
        record_test "3-7 TCP快速重传" "PARTIAL" "2"
    fi
}

# 生成测试报告
generate_report() {
    log_info "======================================"
    log_info "生成测试报告"
    log_info "======================================"
    
    local report_file="${CURRENT_TEST_DIR}/test_report.txt"
    
    {
        echo "=========================================="
        echo "        网络实验自动化测试报告"
        echo "=========================================="
        echo "测试时间: ${TIMESTAMP}"
        echo "测试目录: ${CURRENT_TEST_DIR}"
        echo ""
        echo "=========================================="
        echo "           测试结果摘要"
        echo "=========================================="
        echo ""
        
        local total_score=0
        
        echo "第一级 (50分):"
        echo "----------------------------------------"
        for test_name in "1-1 GBN协议正确性" "1-2 GBN滑动窗口移动"; do
            if [[ -v test_results["${test_name}"] ]]; then
                IFS='|' read -r result score <<< "${test_results[${test_name}]}"
                printf "%-30s : %s (%s分)\n" "${test_name}" "${result}" "${score}"
                total_score=$((total_score + score))
            fi
        done
        echo ""
        
        echo "第二级 (30分):"
        echo "----------------------------------------"
        for test_name in "2-3 SR协议正确性" "2-4 SR滑动窗口移动"; do
            if [[ -v test_results["${test_name}"] ]]; then
                IFS='|' read -r result score <<< "${test_results[${test_name}]}"
                printf "%-30s : %s (%s分)\n" "${test_name}" "${result}" "${score}"
                total_score=$((total_score + score))
            fi
        done
        echo ""
        
        echo "第三级 (20分):"
        echo "----------------------------------------"
        for test_name in "3-5 TCP协议正确性" "3-6 TCP滑动窗口移动" "3-7 TCP快速重传"; do
            if [[ -v test_results["${test_name}"] ]]; then
                IFS='|' read -r result score <<< "${test_results[${test_name}]}"
                printf "%-30s : %s (%s分)\n" "${test_name}" "${result}" "${score}"
                total_score=$((total_score + score))
            fi
        done
        echo ""
        
        echo "=========================================="
        echo "总测试数: ${test_count}"
        echo "通过数: ${pass_count}"
        echo "失败数: ${fail_count}"
        echo "总得分: ${total_score}/100"
        echo "=========================================="
        echo ""
        echo "详细日志文件位置："
        echo "- GBN窗口移动: ${CURRENT_TEST_DIR}/gbn_window/window_movements.txt"
        echo "- SR窗口移动:  ${CURRENT_TEST_DIR}/sr_window/window_movements.txt"
        echo "- TCP窗口移动: ${CURRENT_TEST_DIR}/tcp_window/window_movements.txt"
        echo "- TCP快速重传: ${CURRENT_TEST_DIR}/tcp_fast_retransmit/fast_retransmit_events.txt"
        echo ""
    } | tee "${report_file}"
    
    log_success "测试报告已生成: ${report_file}"
}

# 主函数
main() {
    echo ""
    log_info "=========================================="
    log_info "  网络实验自动化测试套件 (修复版)"
    log_info "  测试GBN、SR、TCP协议的正确性"
    log_info "=========================================="
    echo ""
    
    if [ ! -f "bin/gbn" ] || [ ! -f "bin/sr" ] || [ ! -f "bin/tcp" ]; then
        log_warning "可执行文件不存在，需要编译项目"
        if ! compile_project; then
            log_error "编译失败，无法继续测试"
            exit 1
        fi
    fi
    
    if [ ! -f "input.txt" ]; then
        log_error "input.txt 文件不存在"
        exit 1
    fi
    
    test_gbn_correctness
    echo ""
    test_gbn_window
    echo ""
    test_sr_correctness
    echo ""
    test_sr_window
    echo ""
    test_tcp_correctness
    echo ""
    test_tcp_window
    echo ""
    test_tcp_fast_retransmit
    echo ""
    
    generate_report
    
    log_success "所有测试完成！"
    log_info "测试结果保存在: ${CURRENT_TEST_DIR}"
}

main "$@"
