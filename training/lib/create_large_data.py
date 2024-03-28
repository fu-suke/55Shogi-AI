import json

with open("./playout_data.json", "r") as f:
    data = json.load(f)


# 先手後手の対称性を考慮してデータを変換
def convert_data(arr):
    board = arr[:25]
    hand_black = arr[25:31]
    hand_white = arr[31:37]
    turn = arr[37]

    new_board = [piece ^ 16 if piece != 0 else 0 for piece in board[::-1]]
    new_hand = hand_white + hand_black
    turn ^= 1

    return new_board + new_hand + [turn]


# 左右の対称性を考慮してデータを変換
def convert_data2(arr):
    file1 = arr[:5]
    file2 = arr[5:10]
    file3 = arr[10:15]
    file4 = arr[15:20]
    file5 = arr[20:25]
    new_board = file5 + file4 + file3 + file2 + file1
    hand = arr[25:37]
    turn = [arr[37]]
    new_arr = new_board + hand + turn
    return new_arr


if __name__ == "__main__":

    n = len(data["X"])
    print(n)

    # データの変換
    # 元のリストの長さに基づいてループしないとデータが増え続けて無限ループになる
    for i, x in enumerate(data["X"][:n]):
        nex_x = convert_data(x)
        data["X"].append(nex_x)
        data["Y"].append(data["Y"][i])

        # 進捗表示
        if n >= 100 and i % (n // 100) == 0:
            print(f'{(i / n * 100):.2f} %')

    n = len(data["X"])
    for i, x in enumerate(data["X"][:n]):
        nex_x = convert_data2(x)
        data["X"].append(nex_x)
        data["Y"].append(data["Y"][i])

        if n >= 100 and i % (n // 100) == 0:
            print(f'{(i / n * 100):.2f} %')

    with open("./large_playout_data.json", "w") as f:
        json.dump(data, f)

    print("Total data size: ", len(data["X"]))
    print("Done!")
