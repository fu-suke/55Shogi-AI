import matplotlib.pyplot as plt

# ファイルからスコアを読み込む
scores = []
with open("random_playout_results.txt", "r") as file:
    for line in file:
        scores.append(float(line.strip()))

# ヒストグラムを作成
plt.hist(scores, bins=10)  # ビンの数は必要に応じて調整
plt.title("Playout Score Distribution")
plt.xlabel("Score")
plt.ylabel("Frequency")
plt.show()
