import pandas as pd
from matplotlib import pyplot as plt

df = pd.read_csv("test/right_hand_comp.csv")
frame = range(len(df))
player_x = df["player_x"]
player_y = df["player_y"]
avatar_x = df["avatar_x"]
avatar_y = df["avatar_y"]

plt.title("Right Hand Comparison")
plt.plot(frame, player_x, label="player x")
plt.plot(frame, player_y, label="player y")
plt.plot(frame, avatar_x, label="avatar x")
plt.plot(frame, avatar_y, label="avatar y")
plt.legend()
plt.show()
