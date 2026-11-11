# AI Waste Classification

## 프로젝트 개요
YOLOv8과 ConvNeXt Tiny 모델을 활용하여 카메라 영상에서 쓰레기를 탐지하고 분류하는 AI 프로젝트입니다.

객체 탐지 기반 분류와 ONNX 모델을 이용한 단독 분류, 두 가지 방식을 구현하였습니다.

## 개발 환경
- Language: Python
- AI Models: YOLOv8n, ConvNeXt Tiny
- Libraries: OpenCV, ONNX Runtime, Ultralytics, NumPy
- Model Format: ONNX
- Hardware: Jetson Orin Nano

## 주요 기능
- 카메라 영상 실시간 입력 및 쓰레기 분류
- YOLOv8 기반 객체 탐지 및 ConvNeXt Tiny 분류
- YOLO 없이 ConvNeXt Tiny 모델만 사용하는 분류 방식
- TensorRT, CUDA, CPU 실행 환경 지원
- 총 10종의 쓰레기 분류
- 분류 결과 및 예측 확률 화면 출력

## 분류 대상
battery, biological, cardboard, clothes, glass, metal, paper, plastic, shoes, trash

## Source Code
- [PredictCamera_classify.py](PredictCamera_classify.py) — YOLOv8 + ConvNeXt Tiny 기반 객체 탐지 및 분류
- [predictcamera_fast.py](predictcamera_fast.py) — ConvNeXt Tiny 단독 분류

## 모델 파일 안내
- `yolov8n.pt`: Ultralytics에서 제공하는 사전학습 모델 사용
- `trash_convnext_tiny.onnx`: 별도 준비가 필요한 쓰레기 분류 모델

※ 모델 가중치 파일은 저장소에 포함하지 않았습니다.

## 프로젝트를 통해 배운 점
YOLOv8 객체 탐지와 ConvNeXt Tiny 이미지 분류 모델을 연계하여 실시간 영상 처리 시스템을 구현하였습니다. 또한 ONNX Runtime을 활용한 모델 추론과 GPU 가속 환경 설정 방법을 학습하고, 객체 탐지 기반 방식과 단독 분류 방식의 차이를 이해할 수 있었습니다.
