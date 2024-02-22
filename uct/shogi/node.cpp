#include "node.h"
#include <algorithm>
#include <cmath>
#include <vector>

int node_cnt = 0;
double total_explore = 0;
double total_exploit = 0;
int max_depth = 0;

Node::Node(Color player_color, Position pos, Move &move, int depth) {
    this->player_color = player_color;
    this->pos = pos;
    this->move = move;
    this->depth = depth;
    node_cnt++;
    if (max_depth < depth) {
        max_depth = depth;
    }
}

Node::~Node() {
    for (auto child : children) {
        delete child;
    }
    node_cnt--;
}

double Node::search() {
    play_cnt++;
    // 初めて来る時は指し手を実行し、プレイアウトを実行
    // ただし、違法手の場合はこのノードで終わり
    if (play_cnt == 1) {
        if (!is_safe_move(move, pos.side_to_move, pos)) {
            this->is_illegal = true;
            // 本当はここで自身のスコアを設定しておくのが良さそう
            return NODE_ILLEGAL;
        }
        // 指し手を実行
        this->pos.do_move(move);

        // 注意！！eval_piecesは指し手を実行した後に呼ぶこと
        this->node_piece_value = eval_pieces(player_color);
        double res = do_playout();
        this->score += res;
        return res;
    }
    // 深さの上限に達していたら、プレイアウトの結果を返す
    if (depth == MAX_DEPTH) {
        double res = do_playout();
        this->score += res;
        return res;
    }

    // 2回目に来た時は子ノードを展開
    if (play_cnt == 2) {
        // このノードが持つ盤面から見た手を生成
        std::vector<Move> move_list = generate_move_list(pos);
        // 合法手の数だけ子ノードを生成
        for (auto move : move_list) {
            // 子ノードを生成
            Node *node = new Node(player_color, pos, move, depth + 1);
            children.push_back(node);
        }
        // 子ノードをシャッフル
        // std::random_shuffle(children.begin(), children.end());
    }

    Node *child;
    double res;
    while (true) {
        // 勝率が最大の子ノードを選択
        child = select_child();
        // nullptrが返ってくる場合は打つ手がないので負け
        if (child == nullptr) {
            res = 0.5; // なんで0.5にしてるんだっけ？
            this->score = SCORE_MAX;
            return res;
        }
        res = -child->search();
        if (res == -NODE_ILLEGAL) {
            // 違法手が返ってきた場合は取り除いてから再度探索
            children.erase(std::remove(children.begin(), children.end(), child),
                           children.end());
        } else {
            break;
        }
    }
    // ASSERT(-1 <= res && res <= 1,
    //        "child->search res: " << res << ", depth: " << depth);
    res = NODE_PIECE_WEGHT * node_piece_value + (1 - NODE_PIECE_WEGHT) * res;
    // resが-1~1の間であることを確認。resの値を表示。
    // ASSERT(-1 <= res && res <= 1, "res: " << res << ", depth: " << depth);
    this->score += res;
    return res;
}

double Node::do_playout() {   // プレイアウトを実行
    Position tmp = this->pos; // 盤面を退避
    double playout_res = playout();
    ASSERT(0 <= playout_res && playout_res <= 1,
           "playout_res: " << playout_res);
    this->pos = tmp; // 盤面を復元

    // rootノードにこれらの値を伝播させるため、手番によって符号を調整
    // 注意！！この調整は、必ず盤面を復元した後に行うこと。
    if (player_color == pos.side_to_move) {
        playout_res = -playout_res;
        this->node_piece_value = -this->node_piece_value;
    }

    double res = NODE_PIECE_WEGHT * this->node_piece_value +
                 (1 - NODE_PIECE_WEGHT) * (playout_res);
    // resが-1~1の間であることを確認。resの値を表示。
    ASSERT(-1 <= res && res <= 1, "res: " << res);
    return res;
}

// 完全ランダムのプレイアウト
double Node::playout() {
    // PLAYOUT_LOOP_MAX手まで進める
    Color color_us = pos.side_to_move;
    for (int i = 0; i < PLAYOUT_LOOP_MAX; ++i) {
        // 手番側の可能な指し手を1つランダムにとってくる
        std::vector<Move> move_list = generate_move_list(pos);
        Move move = pos.select_random_move(move_list);

        // 合法手がない場合は勝敗がついたということ
        if (move.is_none()) {
            // 自分の手番なら負け、相手の手番なら勝ち
            if (pos.side_to_move == color_us) {
                return PLAYER_LOSE;
            } else {
                return PLAYER_WIN;
            }
        }

        // 指し手を実行する
        pos.do_move(move);
    }
    // 勝負がついていなければ、駒の枚数に応じた評価値を返す
    // player優勢なら1に近く、opponent優勢なら0に近い値を返す
    return eval_pieces(color_us) * PLAYOUT_PIECE_WEIGHT +
           PLAYER_DRAW * (1 - PLAYOUT_PIECE_WEIGHT);
}

// 駒の価値を考慮した評価値を返す（0.0 ~ 1.0）
double Node::eval_pieces(const Color color_us) {
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
    // -1 <= ret <= 1 になるように正規化
    double piece_val =
        (piece_value[color_us] - piece_value[~color_us]) / total_piece_value;
    // 0.0 ~ 1.0に収める
    piece_val = (piece_val + 1.0) / 2.0;
    ASSERT(0 <= piece_val && piece_val <= 1.0, "eval_pieces() is invalid");

    return piece_val;
}

// 子ノードのちucbが最大のものを返す
Node *Node::select_child() {
    // 子ノードがない場合はnullptrを返す
    if (children.empty()) {
        return nullptr;
    }
    Node *ret = nullptr;
    double max_score = -100;

    // 相手を詰ませられる手があれば、最優先でそれを返す
    for (auto child : children) {
        if (child->score == SCORE_MAX) {
            return child;
        }
    }
    for (auto child : children) {
        if (child->is_illegal) {
            continue;
        }
        double ucb = child->ucb(this->play_cnt);
        if (ucb == UCB_UNREACHED) {
            ret = child;
            break;
        }
        if (max_score < ucb) {
            max_score = ucb;
            ret = child;
        }
    }
    return ret;
}

double Node::ucb(int parent_play_cnt) {
    if (is_illegal) {
        ASSERT(false, "illegal node");
    }
    if (play_cnt == 0) {
        return UCB_UNREACHED;
    }
    double exploit = rate();
    double explore = C * sqrt(2 * log(parent_play_cnt) / play_cnt);
    // std::cout << "exploit: " << exploit << ", explore: " << explore
    //           << std::endl;
    total_exploit += std::fabs(exploit);
    total_explore += explore;
    double ucb = exploit + explore + node_piece_value * UCB_PIECE_WEIGHT;
    // std::cout << "ucb: " << ucb << std::endl;
    return ucb;
}

double Node::rate() const {
    if (play_cnt == 0) {
        std::cout << "play_cnt == 0" << std::endl;
        return 0;
    }
    return score / play_cnt;
}

bool Node::compare(const Node *a, const Node *b) {
    return b->rate() < a->rate();
}

std::ostream &operator<<(std::ostream &os, const Node &node) {
    os << "move: " << node.move << ", play_cnt: "
       << node.play_cnt
       //    << ", score: " << node.score
       //    << ", node_piece_value: " << node.node_piece_value
       << ", rate: " << node.rate() << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes) {
    for (auto node : nodes) {
        // os << *node << "-----" << std::endl;
        os << *node;
    }
    return os;
}