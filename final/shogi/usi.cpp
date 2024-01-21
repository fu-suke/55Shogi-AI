#include "usi.h"
#include "json.hpp"
#include <bitset>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>

using json = nlohmann::json;

// コンストラクタ
USI::USI() {
    // 初期盤面を設定する
    root = Root();
    // 初期盤面を「訪れた盤面」に追加する
    visited_hash_keys.push_back(root.pos.get_hash_key());

    int a = Node::load_model();
    if (a == -1) {
        std::cout << "Failed to load the model." << std::endl;
        exit(1);
    }
    // std::cout << "Succeed to load the model." << std::endl;
}

void USI::loop() {
    StringEx cmd;

    while (std::getline(std::cin, cmd)) {
        std::vector<std::string> cmds = cmd.Split();
        size_t len = cmds.size();

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
            std::cout << "ok" << std::endl;
        }

        else if (cmds[0] == "go") {
            go();
        }

        else if (cmds[0] == "test") {
            test();
            break;
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
        }
    }
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
    std::cout << ret << std::endl;
}

void USI::send_readyok() {
    std::string ret = "readyok";
    std::cout << ret << std::endl;
}

void USI::send_bestmove() {
    Move best_move = root.search();

    std::cout << "bestmove " << best_move << std::endl;
    USI::do_move(best_move);

    // "bestmove"を出力した時点で、将棋GUIにより強制的に標準出力を停止させられるので
    // 一番最後に実行するようにする
}

void USI::do_move(Move move) {
    root.pos.do_move(move);
    visited_hash_keys.push_back(root.pos.get_hash_key());
}

// テスト用
void USI::test() {
    const int LOOP = 100;
    // 計測開始
    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < LOOP; i++) {
        std::vector<Move> mlist = generate_move_list(root.pos);
        Move m = root.pos.select_random_move(mlist);
        // USI::do_move(m);
        Node *node = new Node(root.pos, m);
        double score = node->eval_playout_score(root.pos.side_to_move);
    }
    // 計測終了
    auto end = std::chrono::system_clock::now();
    auto dur = end - start;
    auto msec =
        std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << msec << "msec" << std::endl;

    start = std::chrono::system_clock::now();
    for (int i = 0; i < LOOP; i++) {
        std::vector<Move> mlist = generate_move_list(root.pos);
        Move m = root.pos.select_random_move(mlist);
        // USI::do_move(m);
        Node *node = new Node(root.pos, m);
        double score = node->calc_playout_score();
    }
    // 計測終了
    end = std::chrono::system_clock::now();
    dur = end - start;
    msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << msec << "msec" << std::endl;
}