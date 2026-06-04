#!/bin/bash
RPI_USER="jjhword"                    # 라즈베리파이 사용자 계정
RPI_IP="100.94.125.69"                # 라즈베리파이 IP 주소
TARGET_DIR="/home/jjhword/rpi_project" # 라즈베리파이 내부에 저장될 폴더 경로

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================================"
echo "[1단계] 수정된 트리 구조 기준 통합 컴파일 엔진 가동..."
echo "========================================================"
make

# 빌드가 성공했는지 리턴코드 체크 ($? 가 0이어야 성공)
if [ $? -ne 0 ]; then
    echo "[Error] 컴파일 빌드 오류 발생. 원격 전송을 전면 중단합니다."
    exit 1
fi
echo "빌드 완료 및 바이너리 bin 집결 성공!"

echo -e "\n========================================================"
echo "[2단계] Raspberry Pi 원격 타겟 보드로 최종 배포 진행 (SCP)..."
echo "========================================================"

# 타겟 디렉토리 원격 자동 생성 가드
ssh "${RPI_USER}@${RPI_IP}" "mkdir -p ${TARGET_DIR}"

if [ -f "./bin/server" ] && [ -f "./bin/libdevice.so" ]; then
    echo "파일 슛팅: bin/server & bin/libdevice.so -> Raspberry Pi"
    
    scp ./bin/server ./bin/libdevice.so "${RPI_USER}@${RPI_IP}:${TARGET_DIR}/"
    
    if [ $? -eq 0 ]; then
        echo "[Deploy Success] 정석 구조 기반 라즈베리파이 최종 배포 완료!"
        echo "실행 명령어: ssh ${RPI_USER}@${RPI_IP} 'cd ${TARGET_DIR} && ./server'"
    else
        echo "[Error] SCP 원격 전송 망에 네트워크 트래블이 발생했습니다."
        exit 1
    fi
else
    echo "[Error] bin 폴더 내에 전송 파일(server/libdevice.so)이 누락되었습니다."
    exit 1
fi