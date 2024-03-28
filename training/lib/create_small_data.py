import numpy as np
import random
import json

if __name__ == "__main__":

    with open("large_playout_data.json", "r") as file:
        data = json.load(file)

    print("Data size:", len(data["X"]))

    random.seed(42)
    np.random.seed(42)

    indices = np.random.choice(len(data["X"]), 15000, replace=False)
    X = np.array(data["X"])[indices]
    y = np.array(data["Y"])[indices]

    with open("mid_playout_data.json", "w") as file:
        json.dump({"X": X.tolist(), "Y": y.tolist()}, file)

    print("Data size:", len(X))
