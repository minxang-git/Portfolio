import cv2
import numpy as np
import onnxruntime as ort

# -----------------------------
# ONNX 모델 설정
# -----------------------------
ONNX_PATH = "trash_convnext_tiny.onnx"
CLASS_INPUT_SIZE = 224     # 모델 입력 크기 (224 고정)
CLASS_NAMES = [
    "battery", "biological", "cardboard", "clothes", "glass",
    "metal", "paper", "plastic", "shoes", "trash"
]

# -----------------------------
# ONNX GPU providers 자동 설정
# -----------------------------
available = ort.get_available_providers()
print("Available providers:", available)

# Jetson에서는 아래 순서가 일반적
providers = []
if "TensorrtExecutionProvider" in available:
    providers.append("TensorrtExecutionProvider")
if "CUDAExecutionProvider" in available:
    providers.append("CUDAExecutionProvider")
providers.append("CPUExecutionProvider")  # fallback

ort_session = ort.InferenceSession(ONNX_PATH, providers=providers)
input_name = ort_session.get_inputs()[0].name

# -----------------------------
# 전처리
# -----------------------------
def preprocess(img_bgr):
    img = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB).astype(np.float32) / 255.0
    img = cv2.resize(img, (CLASS_INPUT_SIZE, CLASS_INPUT_SIZE))
    img = (img - [0.485, 0.456, 0.406]) / [0.229, 0.224, 0.225]
    img = img.transpose(2, 0, 1)[np.newaxis, :].astype(np.float32)
    return img

# -----------------------------
# 분류 함수
# -----------------------------
def classify(img):
    x = preprocess(img)
    out = ort_session.run(None, {input_name: x})[0]   # shape (1,10)
    exp = np.exp(out - np.max(out))
    prob = exp / np.sum(exp)
    idx = int(np.argmax(prob))
    return CLASS_NAMES[idx], float(prob[0][idx])

# -----------------------------
# 메인 카메라 실행
# -----------------------------
def show_camera(camera_id=0):
    cap = cv2.VideoCapture(camera_id)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    cap.set(cv2.CAP_PROP_FPS, 30)

    if not cap.isOpened():
        print("카메라 열기 실패")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        cls, p = classify(frame)

        cv2.putText(frame, f"{cls} {p*100:.1f}%",
                    (20, 40), cv2.FONT_HERSHEY_SIMPLEX,
                    1, (0, 255, 0), 2)

        cv2.imshow("Fast Trash Classifier (No YOLO)", frame)

        if cv2.waitKey(1) & 0xFF in [27, ord("q")]:
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    show_camera()

