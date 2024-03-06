import cv2
import cvzone
import time
import math
from ultralytics import YOLO
import serial

# Constants (adjust as needed)
PLANT_INDEX = 58
PERSON_INDEX = 0

# Mapping of indices to labels
LABELS = {
    PERSON_INDEX: "person",
    PLANT_INDEX: "chilly plant"
}

# Initialize serial communication with Arduino
try:
    arduino = serial.Serial('/dev/cu.usbmodem101', 115200, timeout=0.1)
    print("Serial connection established with Arduino")
    time.sleep(2)
except serial.SerialException as e:
    print(f"Error opening serial connection: {e}")
    exit()

# Initialize the camera
cap = cv2.VideoCapture(0 + cv2.CAP_AVFOUNDATION)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 360)

# Load YOLO model
model = YOLO("../Yolo-Weights/yolov8n.pt")
print("YOLO model loaded successfully")

prev_frame_time = 0
prev_command = None

try:
    while True:
        new_frame_time = time.time()
        success, img = cap.read()

        if success:
            results = model(img, stream=True)
            fps = 1 / (new_frame_time - prev_frame_time)
            prev_frame_time = new_frame_time

            plant_detected = False

            for r in results:
                boxes = r.boxes
                for box in boxes:
                    cls = int(box.cls[0])  # Get class index

                    if cls in [PERSON_INDEX, PLANT_INDEX]:
                        if cls == PLANT_INDEX and cls != PERSON_INDEX:
                            plant_detected = True

                        x1, y1, x2, y2 = map(int, box.xyxy[0])
                        w, h = x2 - x1, y2 - y1
                        cvzone.cornerRect(img, (x1, y1, w, h), colorC=(0, 175, 175), colorR=(175, 175, 0))
                        conf = math.ceil(box.conf[0] * 100) / 100

                        # Get the label from the dictionary
                        label = LABELS.get(cls, f"Unknown Class: {cls}")

                        cvzone.putTextRect(img, f'{label} {round(conf, 2)}', (x1, max(35, y1)), scale=1, thickness=1)

            # Send signal to Arduino
            command = '1' if plant_detected else '0'
            # arduino.write(command.encode())
            # print(command)
            if command != prev_command:
                for bit in command:
                    arduino.write((bit + '\n').encode())
                    time.sleep(0.1)  # Adjust the delay as needed
                prev_command = command
                print(command)


            cvzone.putTextRect(img, f'FPS: {round(fps, 2)}', (20, 40), scale=1, thickness=1)
            cv2.imshow("Image", img)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except Exception as e:
    print(f"An error occurred: {e}")

finally:
    cap.release()
    cv2.destroyAllWindows()
    arduino.close()
    print("Serial connection closed")