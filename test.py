import cv2
import time

cap = cv2.VideoCapture("blackpink.mp4")
start = time.time_ns()
now = start
curr = 0
fps = cap.get(5)
while(cap.isOpened()):
  now = time.time_ns()
  index = int(fps * (now - start) * 1e-9)
  for i in range(index - curr - 1):
    cap.grab()
  ret, frame = cap.read()
  curr = index
  cv2.imshow("test", frame)
  cv2.waitKey(1)
  print(int(1e9 / (time.time_ns() - now)))
