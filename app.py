from flask import Flask, request, jsonify, send_from_directory
from datetime import datetime
import torch
import cv2
import numpy as np
import os


# 1. KHỞI TẠO

app = Flask(__name__)

# ── Thử load mô hình tùy chỉnh (best.pt). Nếu chưa có → dùng yolov5s pretrained
MODEL_PATH = "best.pt"
try:
    model = torch.hub.load('ultralytics/yolov5', 'custom',
                           path=MODEL_PATH, force_reload=False)
    print(f"🎯 Loaded custom model: {MODEL_PATH}")
except Exception as e:
    print(f"⚠️  Không tìm thấy {MODEL_PATH} – dùng yolov5s mặc định.")
    model = torch.hub.load('ultralytics/yolov5', 'yolov5s', pretrained=True)

# (tuỳ chọn) chỉnh threshold
model.conf = 0.4          # confidence cut-off
model.iou  = 0.45         # NMS IoU threshold

# ── Thư mục lưu trữ
UPLOAD_FOLDER  = "uploads"
RESULT_FOLDER  = "results"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(RESULT_FOLDER, exist_ok=True)


# 2. ROUTE NHẬN ẢNH & NHẬN DIỆN

@app.route('/upload', methods=['POST'])
def upload_and_detect():
    # 2.1. Đọc dữ liệu ảnh
    if not request.data:
        return jsonify({"status": "fail", "msg": "No image data"}), 400

    nparr = np.frombuffer(request.data, np.uint8)
    img   = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    if img is None:
        return jsonify({"status": "fail", "msg": "Decode error"}), 400

    # 2.2. Lưu ảnh gốc
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    raw_name   = f"{ts}.jpg"
    raw_path   = os.path.join(UPLOAD_FOLDER, raw_name)
    cv2.imwrite(raw_path, img)
    print(f"📥  Ảnh gốc đã lưu: {raw_path}")

    # 2.3. Chạy YOLOv5
    results  = model(img)
    boxes_df = results.pandas().xyxy[0]
    labels   = boxes_df['name'].tolist()
    print(f"🧠  Phát hiện: {labels}")

    # 2.4. Sinh ảnh có bounding-box
    rendered = results.render()[0]          # numpy BGR
    res_name = f"{ts}_pred.jpg"
    res_path = os.path.join(RESULT_FOLDER, res_name)
    cv2.imwrite(res_path, rendered)
    print(f"💾  Ảnh kết quả đã lưu: {res_path}")

    # 2.5. Ghi log
    with open("log.csv", "a", encoding="utf-8") as f:
        f.write(f"{ts},{','.join(labels)}\n")

    # 2.6. Trả JSON (kèm link xem ảnh)
    return jsonify({
        "status"     : "ok",
        "labels"     : labels,
        "result_url" : f"/result/{res_name}"
    }), 200


# 3. ROUTE LẤY ẢNH KẾT QUẢ

@app.route('/result/<filename>')
def get_result(filename):
    # Cho phép mở ảnh ngay trên trình duyệt: http://IP:5000/result/20250604_143201_pred.jpg
    return send_from_directory(RESULT_FOLDER, filename)


# 4. CHẠY SERVER

if __name__ == '__main__':
    # host='0.0.0.0' để ESP32-CAM (hoặc máy khác) truy cập được
    app.run(host='0.0.0.0', port=5000, debug=False)
