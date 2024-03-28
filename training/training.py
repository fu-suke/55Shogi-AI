import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset
import json
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
from shogi_nn import ShogiNN
from datetime import datetime
from shogi_nn import INPUT_SIZE

lr = 0.0001
batch_size = 64
num_layers = 6
hidden_size = 1500
num_epochs = 30
reduction_factor = 1.0
dropout_rate = 0.0
init = "xavier"

cuda_available = torch.cuda.is_available()
if cuda_available:
    print("Cuda is available.")
else:
    print("Cuda is not available.")

cuda_available = False
with open('./data/large_playout_data.json', 'r') as file:
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

model = ShogiNN(num_layers=num_layers, hidden_size=hidden_size,
                reduction_factor=reduction_factor, dropout_rate=dropout_rate, initialize=init)
if cuda_available:
    model.to('cuda')

# 損失関数とオプティマイザーの設定
criterion = nn.MSELoss()
optimizer = optim.Adam(model.parameters(), lr=lr)

# データセットとデータローダーの設定
dataset = TensorDataset(X_train, y_train)
dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

# start = time.time()

model.train()
# 訓練ループ
for epoch in range(num_epochs):
    for inputs, targets in dataloader:
        if cuda_available:
            inputs = inputs.to('cuda')
            targets = targets.to('cuda')
        targets = targets.unsqueeze(1)
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        loss.backward()
        optimizer.step()

    model.eval()
    with torch.no_grad():  # 勾配計算を無効化
        if cuda_available:
            X_test = X_test.to('cuda')
            y_test = y_test.to('cuda')
        predictions = model(X_test)
        test_loss = criterion(predictions, y_test.unsqueeze(1)).item()
    print(f'Epoch: {(epoch+1):3d}/{num_epochs}, Loss: {loss.item():.5f}')
    print(f'Test Loss: {test_loss}')
    model.train()

# モデルの保存
t = datetime.now().strftime('%m%d_%H%M')
# torch.save(model.state_dict(), f'shogi_nn_model_{t}.pt')

# トレースして保存
model.eval()
example = torch.rand(1, INPUT_SIZE)
model.to('cpu')
traced_script_module = torch.jit.trace(model, example)
traced_script_module.save(f"./models/model4_{t}.pt")
