# Nhận diện và phân loại vật nuôi bằng hình ảnh sử dụng ESP32-CAM và YOLOv5

## 📌 Giới thiệu

Đề tài hướng đến việc xây dựng hệ thống giám sát và nhận diện vật nuôi (chó, mèo, gà...) tự động bằng cách sử dụng camera ESP32-CAM để chụp ảnh và YOLOv5 để phân loại. Kết quả nhận dạng được lưu trữ phục vụ cho mục đích quản lý.

## 🛠️ Công nghệ sử dụng

- ESP32-CAM (AI Thinker)
- Python (Flask)
- YOLOv5 (PyTorch)
- HTML/CSS cho giao diện web
- Git & GitHub để quản lý mã nguồn

## ⚙️ Chức năng

- ESP32-CAM chụp ảnh và gửi về server
- Server Flask nhận ảnh, phân tích bằng YOLOv5
- Hiển thị kết quả nhận dạng (loài vật nuôi)
- Lưu ảnh và thông tin vào thư mục `/results`
- Giao diện hiển thị ảnh và nhật ký nhận diện

## 🧩 Sơ đồ hệ thống

ESP32-CAM → (WiFi) → Flask Server (YOLOv5) → Lưu & hiển thị kết quả

## 📚 Các thư viện cần thiết

Cài đặt bằng pip:

pip install -r requirements.txt

## 🚀 Cách hoạt động

1. Kết nối ESP32-CAM và nạp file `esp32_cam_pet_detect.ino`
2. Chạy Flask server bằng lệnh:
python app.py
3. ESP32-CAM tự động gửi ảnh về địa chỉ Flask server
4. Ảnh được nhận dạng và kết quả hiển thị ngay trên web

---

## 📂 Cấu trúc thư mục

├── app.py # Flask server
├── esp32_cam_pet_detect.ino # Code cho ESP32-CAM
├── uploads/ # Ảnh upload từ ESP32-CAM
├── results/ # Kết quả nhận diện
├── README.md # File mô tả dự án
├── requirements.txt # Thư viện cần cài đặt
└── log.csv # Nhật ký nhận diện
