import pandas as pd

data = pd.read_csv('updated_battle_log.csv').to_dict(orient='records')

mcts_first_win = 0
mcts_second_win = 0
improved_first_win = 0
improved_second_win = 0

mcts = "./engines/mcts"

mcts_win = 0
improved_win = 0


for i in range(0, len(data)):
    first = data[i]["First"]
    # print(first == mcts)
    # print(first)
    # print(mcts)
    # if i >= 3:
    #     break
    if mcts == data[i]["Winner"]:
        if first == mcts:
            mcts_first_win += 1
        else:
            mcts_second_win += 1
        mcts_win += 1
    else:
        if first == mcts:
            improved_second_win += 1
        else:
            improved_first_win += 1
        improved_win += 1

print(f"MCTS: {mcts_win}")
print(f"Improved: {improved_win}")

print("MCTS vs MCTS Improved")
print(f"First Win: {mcts_first_win}")
print(f"Second Win: {improved_second_win}")
print("MCTS Improved vs MCTS")
print(f"First Win: {improved_first_win}")
print(f"Second Win: {mcts_second_win}")