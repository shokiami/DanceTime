import cv2
import time

cap = cv2.VideoCapture("blackpink.mp4")
start = time.time_ns()
prev = start
curr = 0
i = 0
fps = cap.get(5)
while(cap.isOpened()):
  now = time.time_ns()
  index = int(fps * (now - start) * 1e-9)
  while (curr < index - 1):
    cap.grab()
    curr += 1
  ret, frame = cap.read()
  curr += 1
  cv2.imshow("test", frame)
  cv2.waitKey(1)
  if now - prev > 1000000000:
    prev = now
    print(i)
    i = 0
  i += 1
