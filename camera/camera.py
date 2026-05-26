# camera3.py보다 이게 개수형인듯 

import io
import logging
import socketserver
from http import server
from threading import Condition, Thread
from picamera2 import Picamera2
from picamera2.encoders import MJPEGEncoder
from picamera2.outputs import FileOutput

import time

# 아이디와 비밀번호
USERNAME = "admin"
PASSWORD = "1234"

# 모든 요청을 허용하는 check_auth 함수
def check_auth(headers):
    return True

PAGE = """\
<html>
<head>
<title>camera test</title>
</head>
<body>
<h1>camera test</h1>
<img src="stream.mjpg" width="640" height="480" />
</body>
</html>
"""

class StreamingOutput(io.BufferedIOBase):
    def __init__(self):
        self.frame = None
        self.condition = Condition()

    def write(self, buf):
        with self.condition:
            self.frame = buf
            self.condition.notify_all()

class CORSRequestHandler(server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Authorization')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200, "ok")
        self.end_headers()

class StreamingHandler(CORSRequestHandler):
    def do_GET(self):
        if not check_auth(self.headers):
            self.send_response(401)
            self.send_header('WWW-Authenticate', 'Basic realm=\"Authentication required\"')
            self.end_headers()
            self.wfile.write('Authentication failed'.encode('utf-8'))
            return

        if self.path == '/':
            self.send_response(301)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif self.path == '/index.html':
            content = PAGE.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    with output.condition:
                        output.condition.wait()
                        frame = output.frame
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning(
                    'Removed streaming client %s: %s',
                    self.client_address, str(e))
        elif self.path == '/screenshot':
            self.send_response(200)
            self.send_header('Content-Type', 'image/jpeg')
            self.send_header('Content-Disposition', 'attachment; filename="screenshot.jpg"')
            self.end_headers()
            
            # 사진 촬영
            stream = io.BytesIO()
            picam2.capture_file(stream, format='jpeg')
            stream.seek(0)
            self.wfile.write(stream.read())
        else:
            self.send_error(404)
            self.end_headers()

class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

def start_camera():
    global picam2, output
    try:
        picam2 = Picamera2()
        picam2.configure(picam2.create_video_configuration(main={"size": (640, 480)}))
        output = StreamingOutput()
        picam2.start_recording(MJPEGEncoder(), FileOutput(output))
    except Exception as e:
        logging.error(f"Error starting camera: {e}")
        time.sleep(5)  # 카메라 초기화 실패 시 5초 대기 후 재시도
        start_camera()

def stop_camera():
    global picam2
    if picam2 is not None:
        picam2.stop_recording()
        picam2.close()  # 리소스 해제
        picam2 = None

def monitor_camera():
    while True:
        try:
            time.sleep(60)  # 60초마다 상태 확인
            if picam2 is None or not picam2.started:  # is_recording 대신 started로 상태 확인
                stop_camera()
                start_camera()
        except Exception as e:
            logging.error(f"Error monitoring camera: {e}")
            stop_camera()
            time.sleep(5)  # 오류 발생 시 5초 대기 후 재시도
            start_camera()

start_camera()

monitor_thread = Thread(target=monitor_camera, daemon=True)
monitor_thread.start()

try:
    address = ('', 8000)
    server = StreamingServer(address, StreamingHandler)
    server.serve_forever()
finally:
    stop_camera()
