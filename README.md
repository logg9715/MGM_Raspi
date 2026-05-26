# MGM_Raspi
MGM 프로젝트 스마트팜 코드입니다. [MGM-백엔드](https://github.com/logg9715/MGM), [MGM-프론트](https://github.com/logg9715/MGM_React_View)와 연계됩니다. 

## 준비
1. [Wiring pi](https://github.com/GrazerComputerClub/WiringPi.git) 설치
2. miniupnpc 설치, portf.sh의 공인ip 수정

## 실행
- 카메라 : cameranew.py
- 스마트팜 : /udp/devided/bin

## 통신설계
![image](https://github.com/user-attachments/assets/5f161526-46d5-476e-9632-dd3c04d0098b)
![image](https://github.com/user-attachments/assets/f2cdd81a-6d6a-4f7a-a89b-313a6a721b63)


## raspi 내부 통신
![image](https://github.com/user-attachments/assets/6e152822-5b21-4273-af89-fcc117b0ab7f)

## GPIO 회로도
![image](https://github.com/user-attachments/assets/c282b5c7-e8bb-4a60-be30-bd4cd02c15c0)
-------



# MGM_Raspi

스마트팜 IoT 시스템의 **라즈베리파이 서버**    
센서 데이터 수집, 액추에이터 제어, 영상 스트리밍을 담당    
    
> 졸업작품으로 개발한 스마트팜 프로젝트의 일부입니다.
> [MGM (백엔드)](https://github.com/logg9715/MGM) · [MGM_React_View (프론트엔드)](https://github.com/logg9715/MGM_React_View) 와 연동됩니다.

---

## 시스템 구성

```
[프론트엔드] <-> [백엔드] <-> [라즈베리파이 서버 (이 레포)] <-> [센서 / 액추에이터 / 카메라]
```

이 레포는 라즈베리파이에서 동작하며 아래 내용을 담당

- 온습도(SHT30) · 조도 센서 데이터 수집
- 액추에이터(회전 · 물 분사 · LED) 제어
- 카메라 영상 스트리밍
- 백엔드와 UDP 통신

---

## 기술 스택

| 구성 요소 | 언어 | 설명 |
| --- | --- | --- |
| 메인 서버 | C | 멀티스레드 UDP 서버, 센서/액추에이터 제어 |
| 카메라 | Python | 영상 스트리밍 |
| 네트워크 설정 | Bash | UPnP 포트포워딩 자동화 |

> 실시간 제어가 중요한 서버는 **C**로, 라이브러리 활용이 유리한 영상 처리는 **Python**으로 구현

---

## 프로젝트 구조

```
MGM_Raspi/
├── smartfarm/          # 메인 C 서버
│   ├── src/            # main, udp_server, actuators, light_sensor, sht30_sensor, logger
│   ├── include/        # 헤더 파일
│   └── Makefile
├── camera/             # 카메라 영상 스트리밍 (Python)
├── scripts/            # 포트포워딩 자동화 (Bash)
└── experiments/        # 초기 부품별 테스트 + 리팩토링 전 버전
```

---

## 주요 구현

### 멀티스레드 UDP 서버 (C)

`pthread`로 3개 스레드를 동시에 운영

- **UDP 통신 스레드** — 클라이언트 요청 수신 및 응답
- **온습도 센서 스레드** — SHT30으로 온도·습도 주기적 수집
- **조도 센서 스레드** — 광량 측정 및 로깅

공유 데이터(센서 값, 종료 플래그)는 `mutex`로 동기화하여 스레드 간 경합을 방지함.

### 태그 기반 프로토콜

UDP 메시지의 첫 바이트를 태그로 사용자 요청 구분

| 태그 | 동작 |
| --- | --- |
| `0` (SENSOR) | 현재 센서 값(온도, 습도, 조도) 응답 |
| `1` (ACTUATOR) | 액추에이터 제어 (회전 / 물 분사 / LED) |

### 모듈화 설계

기능별로 파일을 분리하고 헤더로 인터페이스를 정의

- `udp_server` — UDP 소켓 통신
- `sht30_sensor` · `light_sensor` — 센서 데이터 수집
- `actuators` — 액추에이터 제어
- `logger` — 데이터 로깅

---

## 개발 과정 (리팩토링)

초기에는 **단일 파일**(`udpServer.c`)로 구현했으나, 유지보수성과 가독성을 위해 **기능별 모듈로 분리**하는 리팩토링 진행

- Before: 모든 로직이 한 파일에 (현재 `experiments/legacyUDPSERVER/`에 보존)
- After: `src/` · `include/` 구조로 관심사 분리

이 과정에서 모듈 간 의존성을 헤더로 정리     
빌드를 Makefile로 자동화   

---

## 요구사항

- **Raspberry Pi** (GPIO 사용)
- **WiringPi** — GPIO 제어 라이브러리
- **miniupnpc** — 포트포워딩 (`scripts/` 사용 시)

> (중요) WiringPi에 의존하므로 라즈베리파이 환경에서만 빌드·실행됨

---

## 빌드 및 실행

```bash
# 스마트팜 서버
cd smartfarm
make
./bin/smartfarm

# 카메라 스트리밍
python3 camera/camera.py

# (선택) 포트포워딩 설정
bash scripts/portf.sh
```

---

## 시스템 설계

### 통신 설계
![image](https://github.com/user-attachments/assets/5f161526-46d5-476e-9632-dd3c04d0098b)
![image](https://github.com/user-attachments/assets/f2cdd81a-6d6a-4f7a-a89b-313a6a721b63)

### 라즈베리파이 내부 통신
![image](https://github.com/user-attachments/assets/6e152822-5b21-4273-af89-fcc117b0ab7f)

### GPIO 회로도
![image](https://github.com/user-attachments/assets/c282b5c7-e8bb-4a60-be30-bd4cd02c15c0)
