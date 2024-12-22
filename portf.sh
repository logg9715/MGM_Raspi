#!/bin/bash

# 라우터의 외부 IP 주소
PUBLIC_IP="175.123.202.85"

# 내부 IP 주소와 포트 번호 설정
declare -A PORT_MAPPINGS=(
    [20786]="192.168.0.142:80"
    [20920]="192.168.0.142:9200:UDP"
    [20800]="192.168.0.142:8000"
    [22183]="192.168.0.183:22"
    [2183]="192.168.0.183:9200"
    [20801]="192.168.0.85:8081"
    [22786]="192.168.0.142:22"
    [20300]="192.168.0.85:3000"
)

# miniupnpc 경로 확인
UPNPC=$(command -v upnpc)
if [ -z "$UPNPC" ]; then
    echo "miniupnpc가 설치되지 않았습니다. 설치 후 다시 시도하십시오."
    exit 1
fi

# 포트포워딩 설정
for EXTERNAL_PORT in "${!PORT_MAPPINGS[@]}"; do
    IFS=":" read -r INTERNAL_IP INTERNAL_PORT PROTOCOL <<< "${PORT_MAPPINGS[$EXTERNAL_PORT]}"
    if [ -z "$PROTOCOL" ]; then
        PROTOCOL="TCP"
    fi
    echo "Setting port forwarding: External $EXTERNAL_PORT -> Internal $INTERNAL_IP:$INTERNAL_PORT ($PROTOCOL)"
    $UPNPC -e "libminiupnpc" -a $INTERNAL_IP $INTERNAL_PORT $EXTERNAL_PORT $PROTOCOL
done

echo "포트포워딩 설정이 완료되었습니다."
