#include "node.h"
#include <algorithm>
#include <cmath> // abs() for float, and fabs()
#include <vector>

int node_cnt = 0;

Node::Node(Position pos, Move &move, int depth) {
    this->pos = pos;
    this->move = move;
    this->depth = depth;
    node_cnt++;

    // NONEはROOTノードの場合にのみ渡される。
    // ROOTノードは既に指し手を実行した後なのでこの処理をスキップする。
    if (!this->move.is_none()) {
        // 指し手の安全性チェック
        // 違法な指し手の場合は、このノードの勝率を最低値にして終了
        if (!is_safe_move(move, pos.side_to_move, pos)) {
            this->score = -INFTY;
            this->is_illegal = true;
        }
        // 指し手を実行
        this->pos.do_move(move);
        // もしこの指し手が一度訪れた盤面だったら千日手対策として違法手にする
        HASH_KEY hash_key = this->pos.get_hash_key();
        if (std::find(visited_hash_keys.begin(), visited_hash_keys.end(),
                      hash_key) != visited_hash_keys.end()) {
            this->score = -INFTY;
            this->is_illegal = true;
        }
    }
}

// デストラクタ。子ノードを全て削除したのち、自身を削除する
Node::~Node() {
    for (auto child : children) {
        delete child;
    }
    node_cnt--;
}

double Node::search(double beta) {
    // 指し手生成
    std::vector<Move> move_list = generate_move_list(pos);
    if (move_list.size() == 0) {
        // 指し手がない（＝詰み）の場合は前の手番側の勝ち
        // より浅い詰みを選ぶために深さで割る
        this->score = INFTY / this->depth;
        return INFTY / this->depth;
    }

    // 深さの上限に達していたらこのノードの評価値を返す
    // αβは、「『前の手番』から見たこのノードの評価値」を返すのが適切！！
    if (depth == MAX_DEPTH) {
        this->score = eval_pieces(~pos.side_to_move);
        // rootから見た子の評価値が正になるように符号調整
        if (depth % 2 == 0) {
            this->score -= 1;
        }
        return this->score;
    }
    sort_move_list(move_list, pos);

    // α：子ノードの評価値の最大値
    double alpha = -INFTY;
    // 子ノードの探索
    for (auto move : move_list) {
        Node *child = new Node(pos, move, depth + 1);
        // 子ノードが違法手だった場合バグるのでスキップ
        if (child->is_illegal) {
            delete child; // 不要な子ノードの削除
            continue;
        }
        // α更新
        alpha = std::max(alpha, child->search(-alpha));
        if (this->move.is_none()) {
            // rootノードの場合は子ノードを保存
            children.push_back(child);
        } else {
            // 子ノードの削除
            delete child;
        }
        // βカット
        if (alpha > beta) {
            break;
        }
    }

    // このノードの評価値を「敵から見た評価値」に変換する
    this->score = -alpha;
    return this->score;
}

// 駒の価値を考慮した評価値を返す（0.0 ~ 1.0）
double Node::eval_pieces(const Color color) {
    double total_piece_value = 0;
    double piece_value[COLOR_NB] = {0, 0};
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        Piece piece = pos.piece_board[sq];
        if (piece == NO_PIECE) {
            continue;
        }
        piece_value[color_of(piece)] += PIECE_VALUE_MAP.at(type_of(piece));
        total_piece_value += PIECE_VALUE_MAP.at(type_of(piece));
    }
    // 手札にある駒の枚数を数え上げる
    for (int c = BLACK; c < COLOR_NB; ++c) {
        for (Piece pr = RAW_PIECE_BEGIN; pr < RAW_PIECE_NB; ++pr) {
            int num = hand_count(pos.hands[c], (Piece)pr);
            ASSERT(0 <= num && num <= 2, "Number of hand is invalid");
            piece_value[c] += num * PIECE_VALUE_MAP.at(pr);
            total_piece_value += num * PIECE_VALUE_MAP.at(pr);
        }
    }
    // 0.0 ~ 1.0に正規化
    double piece_val = piece_value[color] / total_piece_value;
    ASSERT(0 <= piece_val && piece_val <= 1.0, "eval_pieces() is invalid");

    return piece_val;
}

bool Node::compare(const Node *a, const Node *b) { return b->score < a->score; }

std::ostream &operator<<(std::ostream &os, const Node &node) {
    os << "move: " << node.move << ", score: " << node.score << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes) {
    for (auto node : nodes) {
        os << *node;
    }
    return os;
}