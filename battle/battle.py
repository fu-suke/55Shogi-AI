import subprocess
import re
import os
import csv
from datetime import datetime


def send_message(exe, message):
    # print(f"GM: {message.strip()}")
    exe.stdin.write(message)
    exe.stdin.flush()

    while True:
        output = exe.stdout.readline().strip()
        # print(f"{exe.args[:-4]}: {output}")
        if "ok" in output or "bestmove" in output:
            # print(f"{exe.args[:-4]}: {output}")
            return output


def add_game_result(player1, player2, winner, player1_time, player2_time, total_moves):
    log_file = "battle_log.csv"
    file_exists = os.path.isfile(log_file)

    with open(log_file, 'a', newline='') as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow(["Date", "Player1", "Player2", "Winner",
                            "Player1ThinkingTime", "Player2 ThinkingTime", "TotalMoves"])

        now = datetime.now().strftime("%Y-%m-%d %H:%M")
        writer.writerow([now, player1, player2, winner,
                        player1_time, player2_time, total_moves])


move_pattern = re.compile(r"bestmove\s([^\s]+)")


class ResignException(Exception):
    pass


class IllegalMoveException(Exception):
    pass


def battle(engine1_path, engine2_path):
    if not hasattr(battle, "cnt"):
        battle.cnt = 1
    else:
        battle.cnt += 1

    if battle.cnt % 2 == 0:
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

    print(
        f"Game {battle.cnt}: {first.args[:-4]} vs {second.args[:-4]} start!!")
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
                    print(f"{players[side_to_move].args[:-4]} resigned.")
                    raise ResignException(side_to_move)
                send_message(players[~side_to_move], f"position {move}\n")
                side_to_move ^= 1
            else:
                raise IllegalMoveException()
    except ResignException as e:
        winner = 1 ^ e.args[0]
        add_game_result(first.args[:-4], second.args[:-4],
                        winner, times[0], times[1], turn)
    except IllegalMoveException:
        print("Illegal move !!")
        pass
    finally:
        for player in players:
            player.stdin.close()
            player.terminate()
            player.wait()


def main():
    engine_dir = "../engines/"
    engine_list = os.listdir(engine_dir)
    engine_list = [engine_dir + engine for engine in engine_list]
    ab_engine_list = [engine for engine in engine_list if "ab" in engine]
    uct_engine_list = [engine for engine in engine_list if "uct" in engine]
    hybrid_engine_list = [
        engine for engine in engine_list if "hybrid" in engine]

    for engine1 in hybrid_engine_list:
        for engine2 in uct_engine_list:
            for _ in range(100):
                battle(engine1, engine2)

    for engine1 in hybrid_engine_list:
        for engine2 in ab_engine_list:
            for _ in range(100):
                battle(engine1, engine2)

    for engine1 in uct_engine_list:
        for engine2 in ab_engine_list:
            for _ in range(100):
                battle(engine1, engine2)


if __name__ == "__main__":
    main()
