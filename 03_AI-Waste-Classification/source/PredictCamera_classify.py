from ultralytics import YOLO
import cv2
import numpy as np
import os

# ONNX Runtime 사용 여부 확인
use_onnxruntime = True
try:
    import onnxruntime as ort
except Exception:
    use_onnxruntime = False

# 쓰레기 분류 클래스 10종
CLASS_NAMES = [
    "battery",
    "biological",
    "cardboard",
    "clothes",
    "glass",
    "metal",
    "paper",
    "plastic",
    "shoes",
    "trash"
]

# 모델 경로 및 기본 설정
ONNX_PATH = "/mnt/data/trash_convnext_tiny.onnx"
CONF_THRESH_YOLO = 0.25
CLASS_INPUT_SIZE = 224

ort_session = None
onnx_input_name = None

# ONNX 분류 모델 불러오기
def load_onnx(onnx_path=ONNX_PATH):
    global ort_session, onnx_input_name
    if not use_onnxruntime or not os.path.exists(onnx_path):
        return False
    ort_session = ort.InferenceSession(onnx_path, providers=['CUDAExecutionProvider', 'CPUExecutionProvider'])
    onnx_input_name = ort_session.get_inputs()[0].name
    return True

# 입력 이미지를 분류 모델 형식에 맞게 전처리
def preprocess_for_classifier(crop_bgr, size=CLASS_INPUT_SIZE):
    img = cv2.cvtColor(crop_bgr, cv2.COLOR_BGR2RGB).astype(np.float32) / 255.0
    img = cv2.resize(img, (size, size), interpolation=cv2.INTER_AREA)
    mean = np.array([0.485, 0.456, 0.406], dtype=np.float32)
    std = np.array([0.229, 0.224, 0.225], dtype=np.float32)
    img = (img - mean) / std
    img = np.transpose(img, (2, 0, 1)).astype(np.float32)
    return np.expand_dims(img, axis=0)

# ONNX 모델로 쓰레기 종류 및 확률 예측
def classify_onnx(crop_bgr):
    if ort_session is None:
        return None, 0.0
    x = preprocess_for_classifier(crop_bgr)
    outputs = ort_session.run(None, {onnx_input_name: x})
    logits = outputs[0]
    exp = np.exp(logits - np.max(logits, axis=1, keepdims=True))
    probs = exp / np.sum(exp, axis=1, keepdims=True)
    top_idx = int(np.argmax(probs, axis=1)[0])
    return CLASS_NAMES[top_idx], float(probs[0, top_idx])

# 탐지된 객체에 박스와 분류 결과 표시
def draw_label(frame, box, label, prob):
    x1, y1, x2, y2 = [int(v) for v in box]
    cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 0, 0), 2)
    text = f"{label} {prob*100:.1f}%"
    (tw, th), _ = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.7, 2)
    cv2.rectangle(frame, (x1, y2+5), (x1+tw+4, y2+5+th+4), (255, 0, 0), -1)
    cv2.putText(frame, text, (x1+2, y2+5+th+2), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

# 카메라 영상에서 객체 탐지 및 쓰레기 분류 실행
def show_camera(use_yolo=True, camera_id="/dev/video0"):
    yolo_model = YOLO('yolov8n.pt') if use_yolo else None
    if use_onnxruntime:
        load_onnx()

    # 카메라 연결
    window_title = "Trash Classifier"
    cap = cv2.VideoCapture(camera_id, cv2.CAP_V4L2)
    if not cap.isOpened():
        try:
            cap = cv2.VideoCapture(int(camera_id))
        except Exception:
            pass

    if not cap.isOpened():
        return

    cv2.namedWindow(window_title, cv2.WINDOW_AUTOSIZE)
    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            # YOLOv8을 이용한 객체 탐지
            if yolo_model is not None:
                try:
                    results = yolo_model.predict(frame, imgsz=640, verbose=False)
                    result = results[0]
                    if hasattr(result, 'boxes') and len(result.boxes) > 0:
                        for j, abox in enumerate(result.boxes.xyxy):
                            conf = float(result.boxes.conf[j])
                            if conf < CONF_THRESH_YOLO:
                                continue

                            # 탐지된 객체 영역 추출
                            x1, y1, x2, y2 = map(int, abox)
                            h, w = frame.shape[:2]
                            crop = frame[max(0, y1):min(h-1, y2), max(0, x1):min(w-1, x2)]
                            if crop.size == 0:
                                continue

                            # 객체 분류 후 결과 표시
                            cls, prob = classify_onnx(crop) if ort_session is not None else ("onnx_not_loaded", 0.0)
                            draw_label(frame, (x1, y1, x2, y2), cls, prob)
                    else:
                        # 객체 미탐지 시 전체 화면 분류
                        if ort_session is not None:
                            cls, prob = classify_onnx(frame)
                            cv2.putText(frame, f"{cls} {prob*100:.1f}%", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 0), 2)
                except Exception:
                    if ort_session is not None:
                        cls, prob = classify_onnx(frame)
                        cv2.putText(frame, f"{cls} {prob*100:.1f}%", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 0), 2)
            else:
                # YOLO 미사용 시 전체 화면 분류
                if ort_session is not None:
                    cls, prob = classify_onnx(frame)
                    cv2.putText(frame, f"{cls} {prob*100:.1f}%", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 0), 2)

            # 실시간 결과 출력 및 종료 키 설정
            cv2.imshow(window_title, frame)
            if (cv2.waitKey(1) & 0xFF) in (27, ord('q')):
                break
    finally:
        # 카메라 및 창 종료
        cap.release()
        cv2.destroyAllWindows()

# 프로그램 실행
if __name__ == "__main__":
    show_camera(use_yolo=True, camera_id="/dev/video0")
