import torch.nn as nn
import torch.nn.init as init

INPUT_SIZE = 38
OUTPUT_SIZE = 1


class ShogiNN(nn.Module):
    def __init__(self, num_layers=3, hidden_size=200, reduction_factor=1.0, dropout_rate=0.3, initialize="he"):
        super(ShogiNN, self).__init__()
        self.num_layers = num_layers

        if initialize == "he":
            init_func = init.kaiming_normal_
        elif initialize == "xavier":
            init_func = init.xavier_normal_
        elif initialize == "rand_norm":
            init_func = init.normal_
        elif initialize == "rand_uni":
            init_func = init.uniform_
        else:
            raise ValueError("Invalid initialization method.")

        if num_layers == 1:
            self.layers = nn.ModuleList([
                nn.Linear(INPUT_SIZE, OUTPUT_SIZE),
                nn.Sigmoid()
            ])
            return

        # 最初の中間層
        layers = [
            nn.Linear(INPUT_SIZE, hidden_size),
            # nn.BatchNorm1d(hidden_size),
            nn.ReLU(),
            # nn.Dropout(dropout_rate)
        ]
        # 追加の中間層
        for _ in range(num_layers-2):
            next_hidden_size = int(hidden_size * reduction_factor)+1
            layers.append(nn.Linear(hidden_size, next_hidden_size))
            # layers.append(nn.BatchNorm1d(next_hidden_size))
            layers.append(nn.ReLU())
            # layers.append(nn.Dropout(dropout_rate))
            hidden_size = next_hidden_size

        # 出力層
        layers.append(nn.Linear(hidden_size, OUTPUT_SIZE))
        layers.append(nn.Sigmoid())

        self.layers = nn.ModuleList(layers)

        # 重みの初期化
        for layer in self.layers:
            if isinstance(layer, nn.Linear):
                init_func(layer.weight)

    def forward(self, x):
        for layer in self.layers:
            x = layer(x)
        return x
