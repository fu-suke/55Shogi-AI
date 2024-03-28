import pandas as pd

data = pd.read_csv("./results/hyperparams_results_20240113075537.csv")

print(data.sort_values("test_loss").head(30))
