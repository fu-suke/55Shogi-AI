#include "usi.h"
#include "../common/string_ex.h"
#include <bitset>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// コンストラクタ
USI::USI() {
    // 初期盤面を設定する
    root = Root();
}

std::ofstream logFile("log.txt"); // ログファイルを上書きモードで開く

void USI::loop() {
    StringEx cmd;

    while (std::getline(std::cin, cmd)) {
        logFile << "<< " << cmd
                << std::endl; // コマンドをログファイルに書き出す
        std::vector<std::string> cmds = cmd.Split();
        int len = cmds.size();

        if (cmds[0] == "quit") {
            break;
        }

        else if (cmds[0] == "usi") {
            usi();
        }

        else if (cmds[0] == "isready") {
            isready();
        }
        // Stateの初期盤面を設定する

        else if (cmds[0] == "usinewgame")
            continue;

        else if (cmds[0] == "position") {
            position(cmds[len - 1]);

        }

        else if (cmds[0] == "go") {
            go();
        }

        else if (cmds[0] == "test") {
            test();
        }

        else if (cmds[0] == "display") {
            root.pos.display_bitboards();
            root.pos.display_hands();
        }

        // コマンドで"hello"と入力された場合
        else if (cmds[0] == "hello") {
            // こちらも"Hello!"と出力する
            // あいさつは大事
            std::cout << "Hello!" << std::endl;
        }

        else {
            std::cout << "invalid comannd: " + cmd << std::endl;
            logFile << "invalid comannd: " + cmd << std::endl;
        }
    }

    logFile.close(); // ログファイルを閉じる
}

void USI::usi() {
    send_id(ID_NAME);
    send_id(ID_AUTHOR);
    send_usiok();
}

void USI::isready() { send_readyok(); }

// commandの末尾を受け取って、Stateの盤面を更新する
void USI::position(std::string sfen) {
    // 初期盤面のコマンドは無視する
    if (sfen == "1") {
        return;
    }
    Move move = Move(sfen);
    root.pos.set_captured_piece(move);
    USI::do_move(move);
}

void USI::go() { send_bestmove(); }

void USI::send_id(ID id) {
    if (id == ID_NAME)
        std::cout << "id name " + ENGINE_NAME << std::endl;
    else if (id == ID_AUTHOR)
        std::cout << "id author " + ENGINE_AUTHOR << std::endl;
}

void USI::send_usiok() {
    std::string ret = "usiok";
    logFile << ">> " << ret << std::endl;
    std::cout << ret << std::endl;
}

void USI::send_readyok() {
    std::string ret = "readyok";
    logFile << ">> " << ret << std::endl;
    std::cout << ret << std::endl;
}

void USI::send_bestmove() {
    // Move best_move = pos.get_random_move();
    // Move best_move = root.pos.search_best_move();
    Move best_move = root.search();

    USI::do_move(best_move);

    // "bestmove"を出力した時点で、将棋GUIにより強制的に標準出力を停止させられるので
    // 一番最後に実行するようにする
    logFile << "bestmove " << best_move << std::endl;
    std::cout << "bestmove " << best_move << std::endl;
}

void USI::do_move(Move move) {
    // std::cout << pos.side_to_move << std::endl;
    root.pos.do_move(move);
}

// テスト用
void USI::test() {
    const int LOOP = 1e5;
    std::ofstream outFile(
        "random_playout_results.txt"); // 結果を保存するファイル
    // auto start = std::chrono::high_resolution_clock::now();

    Move m = Move(Move::RESIGN);
    Node *node = new Node(BLACK, root.pos, m, 0);
    node->play_cnt = 1;
    std::stringstream ss; // スコアを保存するためのstringstream
    Position tmp;
    double total_score = 0;
    for (int i = 0; i < LOOP; ++i) {
        // 進捗10%ごとにスコアを表示
        if ((i + 1) % (LOOP / 10) == 0) {
            std::cout << "loop: " << i + 1 << "score:" << total_score / i
                      << std::endl;
        }
        // std::cout << "loop: " << i + 1 << std::endl;
        tmp = node->pos; // 盤面を退避
        double score = node->playout();
        total_score += score;
        node->pos = tmp;          // 盤面を復元
        ss << score << std::endl; // stringstreamにスコアを追加
        // std::cout << "score: " << score << std::endl;
    }
    std::cout << "test finished" << std::endl;
    outFile << ss.str();
    outFile.close();
    // // 終了時刻を取得
    // auto end = std::chrono::high_resolution_clock::now();

    // // 経過時間を計算
    // auto duration =
    //     std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
    //         .count();

    // std::cout << "time: " << duration << "(ms)" << std::endl;
}