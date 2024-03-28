import json
import torch.optim as optim
import torch.nn as nn
import torch
from torch.utils.data import DataLoader, TensorDataset
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
import optuna
import joblib
from shogi_nn import ShogiNN
from datetime import datetime


with open('./data/processed-playout_data.json', 'r') as file:
    data = json.load(file)
print("Data size: ", len(data["X"]))

X_train, X_test, y_train, y_test = train_test_split(
    data["X"], data["Y"], test_size=0.2, random_state=42)

# 入力データを0~1に正規化
scaler = MinMaxScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

X_train = torch.tensor(X_train, dtype=torch.float32)
y_train = torch.tensor(y_train, dtype=torch.float32)
X_test = torch.tensor(X_test, dtype=torch.float32)
y_test = torch.tensor(y_test, dtype=torch.float32)


def objective(trial):
    lr = trial.suggest_float('lr', 1e-5, 1e-3, log=True)
    # lr = 0.0009
    num_epochs = 20
    num_layers = trial.suggest_int('num_layers', 2, 7)
    hidden_size = trial.suggest_int('hidden_size', 50, 400, step=50)
    # reduction_factor = trial.suggest_float('reduction_factor', 0.5, 0.9)
    reduction_factor = 1.0
    batch_size = 64
    # dropout_rate = trial.suggest_float('dropout_rate', 0.2, 0.5)
    dropout_rate = 0.2

    model = ShogiNN(num_layers=num_layers, hidden_size=hidden_size,
                    reduction_factor=reduction_factor, dropout_rate=dropout_rate)

    # 損失関数とオプティマイザーの設定
    criterion = nn.MSELoss()
    # optimizer = optim.Adagrad(model.parameters(), lr=lr,
    #                           weight_decay=1e-4)
    optimizer = optim.Adam(model.parameters(), lr=lr)
    # データセットとデータローダーの設定
    dataset = TensorDataset(X_train, y_train)
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    # 学習率のスケジューラーの設定
    # scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=50, gamma=0.5)

    model.train()

    # 訓練ループ
    for epoch in range(1, num_epochs+1):
        for inputs, targets in dataloader:
            targets = targets.unsqueeze(1)
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = criterion(outputs, targets)
            loss.backward()
            optimizer.step()

        # 進捗を報告
        model.eval()
        intermediate_score = score(model, criterion)
        trial.report(intermediate_score, epoch)

        # 早期終了の判定
        if trial.should_prune():
            raise optuna.exceptions.TrialPruned()

        model.train()

    return score(model, criterion)


def score(model, criterion):
    # テストデータに対するモデルの評価
    with torch.no_grad():  # 勾配計算を無効化
        predictions = model(X_test)
        test_loss = criterion(predictions, y_test.unsqueeze(1))

    return test_loss.item()


def main():
    try:
        # 最適化の実行
        # study = optuna.create_study(direction='minimize')
        study = optuna.create_study(
            direction='minimize', sampler=optuna.samplers.RandomSampler())
        study.optimize(objective, n_trials=None)  # 無限にtrialを続ける

        # 結果の表示
        print('Best trial:', study.best_trial.params)

    # キーボード入力があった場合は途中保存して終了する
    except KeyboardInterrupt:
        print("\nKeyboardInterrupt caught, saving study...")
        t = datetime.now().strftime('%m%d_%H%M')
        joblib.dump(study, f'./studies/study_{t}.pkl')
        print(f"Study saved as './studies/study_{t}.pkl'")


if __name__ == '__main__':
    main()
