import subprocess
import re
import os
import csv
from datetime import datetime
import time
import pandas as pd
import sys


def send_message(exe, message):
    # メッセージを送信
    print(f"GM: {message.strip()}")
    exe.stdin.write(message)
    exe.stdin.flush()

    # 標準出力からの返答を読み取る
    while True:
        output = exe.stdout.readline().strip()
        # print(f"{exe.args[:-4]}: {output}")
        if "ok" in output or "bestmove" in output:
            print(f"{exe.args[:-4]}: {output}")
            return output


def add_game_result(player1, player2, winner, player1_time, player2_time, total_moves):
    log_file = "battle_log.csv"
    file_exists = os.path.isfile(log_file)
    max_retries = 5  # 最大再試行回数
    wait_seconds = 3  # 再試行するまでの待機時間（秒）

    for attempt in range(max_retries):
        try:
            with open(log_file, 'a', newline='') as file:
                writer = csv.writer(file)
                if not file_exists:
                    writer.writerow(["Date", "Player 1", "Player 2", "Winner",
                                    "Player 1 Thinking Time", "Player 2 Thinking Time", "Total Moves"])
                    file_exists = True  # ファイルが存在するとして扱う

                now = datetime.now().strftime("%Y-%m-%d %H:%M")
                writer.writerow([now, player1, player2, winner,
                                player1_time, player2_time, total_moves])
            break  # 書き込みに成功したらループを抜ける
        except IOError as e:
            # IOErrorが発生した場合（ファイルが他のプログラムによって使用中など）
            if attempt < max_retries - 1:  # 最後の試行では待機しない
                time.sleep(wait_seconds)  # 指定した秒数だけ待機
            else:
                raise  # 最大試行回数に達した場合は例外を再発生させる

    # ログファイルが1000行を超えていたら終わり
    df = pd.read_csv(log_file)
    if len(df) >= 1000:
        sys.exit(0)


move_pattern = re.compile(r"bestmove\s([^\s]+)")


class ResignException(Exception):
    pass


class IllegalMoveException(Exception):
    pass


def battle(engine1_path, engine2_path, game_cnt):
    if game_cnt % 2 == 0:
        first = subprocess.Popen(engine1_path, stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
        second = subprocess.Popen(engine2_path, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
    else:
        first = subprocess.Popen(engine2_path, stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
        second = subprocess.Popen(engine1_path, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
    players = [first, second]
    side_to_move = 0

    assert (send_message(first, "isready\n") == "readyok")
    assert (send_message(second, "isready\n") == "readyok")

    print(f"Game {game_cnt+1} start !!")
    times = [0, 0]
    turn = 0
    try:
        while True:
            turn += 1
            start = datetime.now()
            best_move = send_message(players[side_to_move], "go\n")
            end = datetime.now()
            times[side_to_move] += (end - start).total_seconds()
            move = move_pattern.match(best_move).group(1)
            if move:
                if move == "resign":
                    # print(f"Player {side_to_move} resigned.")
                    raise ResignException(side_to_move)
                send_message(players[~side_to_move], f"position {move}\n")
                side_to_move = ~side_to_move
            else:
                raise IllegalMoveException()
    except ResignException as e:
        winner = players[~e.args[0]]
        print(f"player {e.args[0]} resigned.")
        add_game_result(first.args, second.args,
                        winner.args, times[0], times[1], turn)
    except IllegalMoveException:
        print("Illegal move !!")
        pass
    finally:
        # プロセスを終了
        for player in players:
            player.stdin.close()
            player.terminate()
            player.wait()


def main():
    engine1_path = "../engines/ab_2.exe"
    engine2_path = "../engines/ab_4.exe"
    for i in range(1000):
        battle(engine1_path, engine2_path, i)
