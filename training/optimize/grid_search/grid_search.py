import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset
import json
from sklearn.model_selection import train_test_split
import time
from sklearn.preprocessing import MinMaxScaler
from shogi_nn import ShogiNN
from datetime import datetime
import csv
import os
import numpy as np

cuda_available = torch.cuda.is_available()
if cuda_available:
    print("Cuda is available.")
else:
    print("Cuda is not available.")


def load_data(path):

    # データの読み込み
    with open(path, 'r') as file:
        data = json.load(file)

    print("Succeed to load data.")

    # 訓練用データとテスト用データに分割
    X_train, X_test, y_train, y_test = train_test_split(
        data["X"], data["Y"], test_size=0.2, random_state=42)

    # 入力データを0~1に正規化
    scaler = MinMaxScaler()
    X_train = scaler.fit_transform(X_train)
    X_test = scaler.transform(X_test)

    print("Data size: ", len(data["X"]))

    X_train = torch.tensor(X_train, dtype=torch.float32)
    y_train = torch.tensor(y_train, dtype=torch.float32)
    X_test = torch.tensor(X_test, dtype=torch.float32)
    y_test = torch.tensor(y_test, dtype=torch.float32)

    return X_train, X_test, y_train, y_test


def train(
        X_train, X_test, y_train, y_test,
        lr,
        batch_size=32,
        num_layers=3,
        hidden_size=200,
        num_epochs=20,
        reduction_factor=1.0,
        dropout_rate=0.0,
        init="xavier"):

    model = ShogiNN(num_layers=num_layers,
                    hidden_size=hidden_size,
                    reduction_factor=reduction_factor,
                    dropout_rate=dropout_rate,
                    initialize=init)
    if cuda_available:
        model.to('cuda')

    # 損失関数とオプティマイザーの設定
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=lr)

    # データセットとデータローダーの設定
    dataset = TensorDataset(X_train, y_train)
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    # 時間計測開始
    # start = time.time()

    model.train()
    min_test_loss = 1000
    # 訓練ループ
    for epoch in range(num_epochs):
        for inputs, targets in dataloader:
            if cuda_available:
                inputs = inputs.to('cuda')
                targets = targets.to('cuda')
            targets = targets.unsqueeze(1)  # ターゲットのサイズを変更
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, targets)
            loss.backward()
            optimizer.step()

        # 10%ごとに損失を出力
        # if (epoch+1) % (num_epochs//10) == 0:
        # print(f'Epoch: {(epoch+1):3d}/{num_epochs}, Loss: {loss.item():.5f}')
        # テストデータに対するモデルの評価
        model.eval()
        with torch.no_grad():  # 勾配計算を無効化
            if cuda_available:
                X_test = X_test.to('cuda')
                y_test = y_test.to('cuda')
            predictions = model(X_test)
            test_loss = criterion(predictions, y_test.unsqueeze(1)).item()
            # print(f'Test Loss: {test_loss}')
            min_test_loss = min(min_test_loss, test_loss)
        model.train()
    return min_test_loss


def main():
    X_train, X_test, y_train, y_test = load_data(
        './data/small_playout_data.json')
    time_stamp = datetime.now().strftime("%Y%m%d%H%M%S")
    res_filename = f'./results/hyperparams_results_{time_stamp}.csv'

    fieldnames = [
        "lr",
        "batch_size",
        "num_layers",
        "hidden_size",
        "reduce_factor",
        "test_loss"
    ]

    # ファイルが存在しない場合は、ヘッダーを追加
    if not os.path.exists(res_filename):
        with open(res_filename, 'w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()

    # ハイパーパラメータの設定
    # min_lr = np.log10(0.0001)  # 最小学習率の対数
    # max_lr = np.log10(0.001)   # 最大学習率の対数
    # lr = np.logspace(min_lr, max_lr, num=5, base=10.0)
    lr = [0.0001, 0.0005]
    batch_size = [32, 64]
    num_layers = [6, 7, 8, 9, 10]  # 6, 7, 8, 9, 10が候補
    hidden_size = [1500, 2000, 2300]  # 1500, 2300も候補
    num_epochs = 20
    reduction_factor = [0.6, 0.8, 1.0]  # 0.6, 0.8が候補。1.2とかもありかも？
    # dropout_rates = [0.0]
    init = ["xavier"]

    for lr_ in lr:
        for batch_size_ in batch_size:
            for num_layers_ in num_layers:
                for hidden_size_ in hidden_size:
                    for reduction_factor_ in reduction_factor:
                        try:
                            test_loss = train(
                                X_train, X_test, y_train, y_test,
                                lr_,
                                batch_size_,
                                num_layers_,
                                hidden_size_,
                                num_epochs,
                                reduction_factor_,
                                # init=init
                            )
                            result = {"lr": lr_,
                                      "batch_size": batch_size_,
                                      "num_layers": num_layers_,
                                      "hidden_size": hidden_size_,
                                      "reduce_factor": reduction_factor_,
                                      "test_loss": test_loss
                                      }

                            print(result, "\n", test_loss)
                            with open(res_filename, 'a', newline='') as file:  # 'a' モードでファイルを開く
                                writer = csv.DictWriter(
                                    file, fieldnames=fieldnames)
                                writer.writerow(result)
                        except Exception as e:
                            continue


if __name__ == '__main__':
    main()
