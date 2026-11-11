# AI Waste Classification

## 프로젝트 개요
YOLOv8 객체 탐지 모델과 ConvNeXt Tiny 이미지 분류 모델을 활용하여 카메라 영상에서 쓰레기를 탐지하고 종류를 분류하는 AI 프로젝트입니다.

## 개발 환경
- Language: Python
- AI Models: YOLOv8n, ConvNeXt Tiny
- Libraries: OpenCV, ONNX Runtime, Ultralytics, NumPy
- Model Format: ONNX

## 주요 기능
- 카메라 영상 실시간 입력
- YOLOv8을 이용한 객체 탐지
- ConvNeXt Tiny 모델을 이용한 이미지 분류
- ONNX Runtime 기반 모델 추론
- 탐지 영역 및 분류 결과 화면 표시
- CUDA 및 CPU 실행 환경 지원

## 분류 대상
battery, biological, cardboard, clothes, glass, metal, paper, plastic, shoes, trash

## 프로젝트를 통해 배운 점
객체 탐지와 이미지 분류 모델을 연계하여 하나의 영상 처리 시스템을 구성하는 방법을 학습했습니다. 또한 ONNX 모델 추론과 OpenCV를 활용한 실시간 카메라 영상 처리 과정을 경험했습니다.

## Source Code
- [PredictCamera_classify.py](source/PredictCamera_classify.py)

## 실행 시 필요한 파일
- ConvNeXt Tiny ONNX 모델 파일 (별도 준비 필요)
- YOLOv8n 모델 가중치
