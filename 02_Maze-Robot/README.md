# Maze Escape Robot

## 프로젝트 개요
Arduino Mega 2560과 초음파 센서를 활용하여 미로 내부의 장애물을 감지하고 자율적으로 주행하는 로봇을 제작한 프로젝트입니다.

## 개발 환경
- Language: C++
- Board: Arduino Mega 2560
- IDE: Arduino IDE
- Hardware: DC Motor, Ultrasonic Sensor, RC Receiver

## 주요 기능
- 5개의 초음파 센서를 이용한 전방 및 측면 거리 측정
- 장애물 감지 및 회피 주행
- 좌우 벽과의 거리를 이용한 주행 위치 보정
- PWM을 이용한 모터 속도 제어
- 버튼을 통한 자율주행 / RC 수동조종 모드 전환

## 프로젝트를 통해 배운 점
초음파 센서 데이터를 활용하여 주변 환경을 판단하고, 모터의 속도와 방향을 제어하는 방법을 학습했습니다. 또한 자율주행과 수동조종 기능을 하나의 시스템으로 구현하면서 임베디드 제어 프로그램의 구조를 이해할 수 있었습니다.

## Source Code
- [maze_robot.ino](source/maze_robot.ino)
