#include "position.h"
#include <algorithm> // ソートを使えるようにする
#include <bitset>    // std::bitsetを使うために必要
#include <ctime>     // std::timeを使えるようにする
#include <random>    // 乱数を使えるようにする
#include <unordered_set>
#include <vector> // 可変長配列を使えるようにする

std::vector<HASH_KEY> visited_hash_keys;

// 初期盤面を生成するコンストラクタ
Position::Position() {
    // piece_bitboardsの初期化

    // 先手の駒
    piece_bitboards[B_GOLD] = Bitboard(1U << 19);
    piece_bitboards[B_KING] = Bitboard(1U << 24);
    piece_bitboards[B_PAWN] = Bitboard(1U << 23);
    piece_bitboards[B_SILVER] = Bitboard(1U << 14);
    piece_bitboards[B_BISHOP] = Bitboard(1U << 9);
    piece_bitboards[B_ROOK] = Bitboard(1U << 4);
    piece_bitboards[B_PRO_PAWN] = Bitboard(0);
    piece_bitboards[B_PRO_SILVER] = Bitboard(0);
    piece_bitboards[B_HORSE] = Bitboard(0);
    piece_bitboards[B_DRAGON] = Bitboard(0);

    // 後手の駒
    piece_bitboards[W_GOLD] = Bitboard(1U << 5);
    piece_bitboards[W_KING] = Bitboard(1U << 0);
    piece_bitboards[W_PAWN] = Bitboard(1U << 1);
    piece_bitboards[W_SILVER] = Bitboard(1U << 10);
    piece_bitboards[W_BISHOP] = Bitboard(1U << 15);
    piece_bitboards[W_ROOK] = Bitboard(1U << 20);
    piece_bitboards[W_PRO_PAWN] = Bitboard(0);
    piece_bitboards[W_PRO_SILVER] = Bitboard(0);
    piece_bitboards[W_HORSE] = Bitboard(0);
    piece_bitboards[W_DRAGON] = Bitboard(0);

    // 各マスと駒を対応付けるデータ
    piece_board[0] = W_KING;
    piece_board[1] = W_PAWN;
    piece_board[4] = B_ROOK;
    piece_board[5] = W_GOLD;
    piece_board[9] = B_BISHOP;
    piece_board[10] = W_SILVER;
    piece_board[14] = B_SILVER;
    piece_board[15] = W_BISHOP;
    piece_board[19] = B_GOLD;
    piece_board[20] = W_ROOK;
    piece_board[23] = B_PAWN;
    piece_board[24] = B_KING;

    // 手番の初期化
    side_to_move = BLACK;

    // 手札の初期化
    hands[BLACK] = Hand(0);
    hands[WHITE] = Hand(0);
}

// Positionをコピーするコンストラクタ
Position::Position(const Position &pos) {
    // 盤面のコピー
    std::copy(std::begin(pos.piece_board), std::end(pos.piece_board),
              std::begin(piece_board));
    std::copy(std::begin(pos.piece_bitboards), std::end(pos.piece_bitboards),
              std::begin(piece_bitboards));
    // 手番のコピー
    side_to_move = pos.side_to_move;
    // 手札のコピー
    std::copy(std::begin(pos.hands), std::end(pos.hands), std::begin(hands));
}

// 手駒と盤上の駒からハッシュキーを計算する
HASH_KEY Position::get_hash_key() {
    HASH_KEY hash_key = 0;
    // 盤上の駒のハッシュ値を計算
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        Piece pc = piece_board[sq];
        hash_key += Zobrist::psq[pc][sq];
    }
    // 手駒のハッシュ値を計算
    for (int c = BLACK; c < COLOR_NB; ++c) {
        for (int pr = RAW_PIECE_BEGIN; pr < RAW_PIECE_NB; ++pr) {
            int num = hand_count(hands[c], (Piece)pr);
            // ASSERT(0 <= num && num <= 2, "Number of hand is invalid");
            for (int i = 0; i < num; ++i) {
                hash_key += Zobrist::hand[c][pr];
            }
        }
    }
    // ここまでハッシュ値の0ビット目は必ず0のはず
    // ASSERT(!(hash_key & 0b1), "bit 0 is not 0 !!!");
    // 手番のハッシュ値を代入
    hash_key += side_to_move;
    return hash_key;
}

void Position::do_move(const Move &move) {
    if (move.is_resign()) {
        std::cout << "Position::do_move resign" << std::endl;
        return;
    }
    if (move.is_none()) {
        std::cout << "Position::do_move none" << std::endl;
        return;
    }
    // 駒打ちの場合
    if (move.is_drop()) {
        Piece pr = move.get_dropped_piece(); // 駒種を取得
        sub_hand(hands[side_to_move], pr);   // 持ち駒を減らす
        Square to = move.get_to();           // bit4..0を取得
        // そこに駒を打つ
        Piece dropped = (Piece)(pr + side_to_move * PIECE_WHITE);
        set_piece(to, dropped);
    } else {
        Square from = move.get_from(); // bit5..9を取得
        Square to = move.get_to();     // bit0..4を取得
        // 駒を動かして、さらに取られた駒を取得
        Piece captured = move_piece(from, to, move.is_promote());
        // 捕られた駒があれば、手番側の持ち駒に加える
        if (captured) {
            add_hand(hands[side_to_move], to_raw(captured));
        }
    }

    // 手番を反転させる
    side_to_move = ~side_to_move;
}

void Position::undo_move(const Move &move) {
    // 駒打ちの場合
    if (move.is_drop()) {
        // 打たれた駒を、打ち手の持ち駒に戻す
        Piece pr = move.get_dropped_piece();
        add_hand(hands[~side_to_move], pr);
        // 打たれた駒のBitboardをクリア
        Square to = move.get_to();
        clear_piece(to);

    }
    // 駒打ち以外の場合
    else {
        Square from = move.get_from();
        Square to = move.get_to();
        // 移動した駒を、成りを考慮して元に戻す
        unmove_piece(from, to, move.is_promote());
        // 取られた駒のチェック
        Piece pn = move.get_captured_piece();
        // 取られた駒があれば、前のプレイヤーの持ち駒から手放して手番側の盤面に置く
        if (pn) {
            // 生駒に戻してから手札から引く
            Piece pr = to_raw(pn);
            sub_hand(hands[~side_to_move], pr);
            Piece pc = (Piece)(pn | side_to_move * PIECE_WHITE);
            set_piece(to, pc);
        }
    }

    // 手番を反転させる
    side_to_move = ~side_to_move;
}

// fromからtoへ駒を移動させる関数。捕られた駒を返す。
Piece Position::move_piece(Square from, Square to, bool is_promote) {
    Piece moved = clear_piece(from);
    // fromに駒があることを保証する
    // ASSERT(moved != NO_PIECE, "from is empty");
    // 成りの指し手であれば成る
    if (is_promote) {
        moved = moved | PIECE_PROMOTE;
    }
    Piece captured = clear_piece(to);
    if (captured) {
        // 動いた駒と捕られた駒は別陣営のはず
        // ASSERT(color_of(moved) != color_of(captured), "capture own piece");
    }
    set_piece(to, moved);
    return captured;
}

// sqにある駒を取り除く関数。取り除かれた駒（先後の区別あり）を返す。
Piece Position::clear_piece(Square sq) {
    Piece removed = piece_board[sq];
    if (removed == NO_PIECE)
        return NO_PIECE;
    piece_board[sq] = NO_PIECE;
    piece_bitboards[removed].clear_bit(sq);
    return removed;
}

// sqに駒pcをセットする関数
void Position::set_piece(Square sq, Piece pc) {
    // セットする先は空のマスであるはず
    // ASSERT(piece_board[sq] == NO_PIECE, "sq is not empty");
    if (pc == NO_PIECE) {
        return;
    }
    piece_board[sq] = pc;
    piece_bitboards[pc].set_bit(sq);
}

// toからfromへ、駒を戻す関数。引数の順番に注意。unpromoteは成りを解除するかどうか。
void Position::unmove_piece(Square from, Square to, bool unpromote) {
    Piece moved_piece = clear_piece(to);
    // toに駒があることを保証する
    // ASSERT(moved_piece != NO_PIECE, "to is empty");
    // 成りの指し手であれば成りを解除する
    if (unpromote) {
        moved_piece = moved_piece & ~PIECE_PROMOTE;
    }
    set_piece(from, moved_piece);
}

// color側の玉が王手されているかどうかを返す
bool Position::is_check(Color color) {
    return all_effect(~color) && piece_bitboards[KING + color * PIECE_WHITE];
}

// piece_bitboardsの論理和を返す
Bitboard Position::occupied_bb(Color color) {
    if (color == BLACK) {
        return piece_bitboards[B_PAWN] | piece_bitboards[B_PRO_PAWN] |
               piece_bitboards[B_SILVER] | piece_bitboards[B_PRO_SILVER] |
               piece_bitboards[B_GOLD] | piece_bitboards[B_KING] |
               piece_bitboards[B_BISHOP] | piece_bitboards[B_HORSE] |
               piece_bitboards[B_ROOK] | piece_bitboards[B_DRAGON];
    } else if (color == WHITE) {
        return piece_bitboards[W_PAWN] | piece_bitboards[W_PRO_PAWN] |
               piece_bitboards[W_SILVER] | piece_bitboards[W_PRO_SILVER] |
               piece_bitboards[W_GOLD] | piece_bitboards[W_KING] |
               piece_bitboards[W_BISHOP] | piece_bitboards[W_HORSE] |
               piece_bitboards[W_ROOK] | piece_bitboards[W_DRAGON];
    } else if (color == COLOR_ALL) {
        return piece_bitboards[B_PAWN] | piece_bitboards[B_PRO_PAWN] |
               piece_bitboards[B_SILVER] | piece_bitboards[B_PRO_SILVER] |
               piece_bitboards[B_GOLD] | piece_bitboards[B_KING] |
               piece_bitboards[B_BISHOP] | piece_bitboards[B_HORSE] |
               piece_bitboards[B_ROOK] | piece_bitboards[B_DRAGON] |
               piece_bitboards[W_PAWN] | piece_bitboards[W_PRO_PAWN] |
               piece_bitboards[W_SILVER] | piece_bitboards[W_PRO_SILVER] |
               piece_bitboards[W_GOLD] | piece_bitboards[W_KING] |
               piece_bitboards[W_BISHOP] | piece_bitboards[W_HORSE] |
               piece_bitboards[W_ROOK] | piece_bitboards[W_DRAGON];
    }
    ASSERT(false, "occupied_bb()'s argument is invalid");
}

// 玉の位置を返す
Square Position::king_square(Color color) {
    Bitboard king = piece_bitboards[KING + color * PIECE_WHITE];
    // return static_cast<Square>(__builtin_ctz(king.p));
    return static_cast<Square>(ctz(king.p));
}

// 歩の利き
Bitboard Position::pawn_effect(Square sq, Color color) {
    return PAWN_EFFECT_BB[sq][color];
}

// 金
Bitboard Position::gold_effect(Square sq, Color color) {
    return GOLD_EFFECT_BB[sq][color];
}

// 銀
Bitboard Position::silver_effect(Square sq, Color color) {
    return SILVER_EFFECT_BB[sq][color];
}

// 玉の利き
Bitboard Position::king_effect(Square sq, Color color) {
    return KING_EFFECT_BB[sq];
}

// 角の利き
Bitboard Position::bishop_effect(Square sq, Color color) {
    Bitboard occ = occupied_bb(COLOR_ALL);
    // 角の斜め方向にあるコマしか利きに影響を与えないので、それ以外の駒を無視する
    occ &= BISHOP_EFFECT_BB[sq][DIRECTION_X];

    // ハッシュ表を漁る
    HASH_KEY key = Bitboards::h(sq, occ);
    auto it = Bitboards::hash_bishop_effect.find(key);

    // キーが見つかった場合
    if (it != Bitboards::hash_bishop_effect.end()) {
        Bitboard result = it->second;
        return result;

    } else {
        // キーが見つからなかった場合は地道に計算する
        Bitboard result = Bitboard(0);
        // 右上方向
        Bitboard mask = BISHOP_EFFECT_BB[sq][DIRECTION_UPPER_RIGHT];
        Bitboard upper_right = mask & occ;             // マスク
        upper_right = reverse_bitboard(upper_right.p); // 逆転
        Bitboard minus_one = upper_right.p - 1;        // 1を引く
        upper_right ^= minus_one; // 1を引いたものとXOR
        upper_right = reverse_bitboard(upper_right.p); // 逆転
        upper_right &= mask; // 再度マスクをかける

        // 右下方向
        mask = BISHOP_EFFECT_BB[sq][DIRECTION_LOWER_RIGHT];
        Bitboard lower_right = mask & occ;
        lower_right = reverse_bitboard(lower_right.p);
        minus_one = lower_right.p - 1;
        lower_right ^= minus_one;
        lower_right = reverse_bitboard(lower_right.p);
        lower_right &= mask;

        // 左下方向
        mask = BISHOP_EFFECT_BB[sq][DIRECTION_LOWER_LEFT];
        Bitboard lower_left = mask & occ;
        minus_one = lower_left.p - 1;
        lower_left ^= minus_one;
        lower_left &= mask;

        // 左上方向
        mask = BISHOP_EFFECT_BB[sq][DIRECTION_UPPER_LEFT];
        Bitboard upper_left = mask & occ;
        minus_one = upper_left.p - 1;
        upper_left ^= minus_one;
        upper_left &= mask;

        result = upper_right | lower_right | lower_left | upper_left;

        Bitboards::hash_bishop_effect[key] = result;
        return result;
    }
}

// 飛車の利き
Bitboard Position::rook_effect(Square sq, Color color) {
    Bitboard occ = occupied_bb(COLOR_ALL);
    occ &= ROOK_EFFECT_BB[sq][DIRECTION_CROSS];

    // ハッシュ表を漁る
    HASH_KEY key = Bitboards::h(sq, occ);
    auto it = Bitboards::hash_rook_effect.find(key);

    // キーが見つかった場合
    if (it != Bitboards::hash_rook_effect.end()) {
        Bitboard result = it->second;
        return result;

    } else {
        Bitboard result = Bitboard(0);
        // 右方向
        Bitboard mask = ROOK_EFFECT_BB[sq][DIRECTION_RIGHT];
        Bitboard right = mask & occ;
        right = reverse_bitboard(right.p);
        Bitboard minus_one = right.p - 1;
        right ^= minus_one;
        right = reverse_bitboard(right.p);
        right &= mask;

        // 上方向
        mask = ROOK_EFFECT_BB[sq][DIRECTION_UP];
        Bitboard up = mask & occ;
        up = reverse_bitboard(up.p);
        minus_one = up.p - 1;
        up ^= minus_one;
        up = reverse_bitboard(up.p);
        up &= mask;

        // 左方向
        mask = ROOK_EFFECT_BB[sq][DIRECTION_LEFT];
        Bitboard left = mask & occ;
        minus_one = left.p - 1;
        left ^= minus_one;
        left &= mask;

        // 下方向
        mask = ROOK_EFFECT_BB[sq][DIRECTION_DOWN];
        Bitboard down = mask & occ;
        minus_one = down.p - 1;
        down ^= minus_one;
        down &= mask;

        result = right | down | left | up;

        Bitboards::hash_rook_effect[key] = result;
        return result;
    }
}

// 馬の利き
Bitboard Position::horse_effect(Square sq, Color color) {
    Bitboard bishop = bishop_effect(sq, color);
    Bitboard cross = CROSS_EFFECT_BB[sq];
    Bitboard result = bishop | cross;
    return result;
}

// 龍の利き
Bitboard Position::dragon_effect(Square sq, Color color) {
    Bitboard rook = rook_effect(sq, color);
    Bitboard x = X_EFFECT_BB[sq];
    Bitboard result = rook | x;
    return result;
}

// color側の全ての駒の利きを合算したBitboardを返す
Bitboard Position::all_effect(Color color) {
    Bitboard result = Bitboard(0);
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        Piece piece = piece_board[sq];
        if (piece == NO_PIECE) {
            continue;
        }
        if (color != color_of(piece)) {
            continue;
        }
        // 先後の区別をなくす
        piece = type_of(piece);
        result |= (this->*Piece_to_EffectFunc.at(piece))(sq, color);
    }
    return result;
}

// piece_boardを標準出力する
void Position::display_piece_board() {
    char board[5][11] = {"__________", "__________", "__________", "__________",
                         "__________"};
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        Piece piece = piece_board[sq];
        if (piece == NO_PIECE) {
            continue;
        }
        int file = sq / 5;
        int rank = sq % 5;
        board[rank][8 - file * 2] = PIECE_TO_CHAR.at(piece);
        if (is_promoted(piece)) {
            board[rank][8 - file * 2 + 1] = '+';
        }
    }

    // コンソールに出力
    std::cout << "**********" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << board[i] << std::endl;
    }
    std::cout << "**********" << std::endl;
}

void Position::display_bitboards() {
    // 5x5の盤面を文字列で表現（5x11のchar配列として）
    char board[5][11] = {"__________", "__________", "__________", "__________",
                         "__________"};

    for (int col = 0; col < 5; col++) {
        for (int row = 4; row >= 0; row--) {
            Square sq = static_cast<Square>(row * 5 + col);
            Bitboard sq_bb = Bitboard(
                1U << sq); // sqの位置にビットが立っているBitboardを生成
            for (Piece i = COLOR_PIECE_BEGIN; i < COLOR_PIECE_NB; i++) {
                Bitboard bb = piece_bitboards[i];
                // もしその位置にコマがあれば
                if (bb.p & sq_bb.p) {
                    board[col][8 - row * 2] = PIECE_TO_CHAR.at(
                        i); // constの辞書を引くときはat関数を使う
                    // 成っているコマであれば、その横に+を付ける
                    if (is_promoted(i)) {
                        board[col][8 - row * 2 + 1] = '+';
                    }
                    break;
                }
            }
        }
    }

    // コンソールに出力
    std::cout << "**********" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << board[i] << std::endl;
    }
    std::cout << "**********" << std::endl;
}

// 全てのpiece_bitboard を標準出力する
void Position::debug_all_bitboards() {
    for (int i = COLOR_PIECE_BEGIN; i < COLOR_PIECE_NB; i++) {
        // キーが存在するときだけ出力する
        if (PIECE_TO_CHAR.find((Piece)i) != PIECE_TO_CHAR.end()) {
            std::cout << PIECE_TO_CHAR.at((Piece)i)
                      << (is_promoted((Piece)i) ? "+" : "") << " = \n"
                      << piece_bitboards[i] << std::endl;
        }
    }
}

void Position::set_captured_piece(Move &move) {
    // 投了は1C->1Cなので、ここで打ち切らないと1Cにあるコマを取ろうとしてバグる
    if (move.is_resign()) {
        return;
    }
    if (move.is_drop()) {
        return;
    }
    Piece captured = piece_board[move.get_to()];
    if (captured == NO_PIECE) {
        return;
    }
    // ASSERT(color_of(captured) == ~side_to_move, "This piece is not opponent's
    // "
    //                                             "piece !!!");
    move.set_captured_piece(captured);
    // ASSERT(move.get_captured_piece() == type_of(captured),
    //        "captured piece is invalid !!!");
}

Move Position::select_random_move(std::vector<Move> &move_list) {
    // static std::mt19937 mt = std::mt19937(20231008);
    static std::mt19937 mt =
        std::mt19937(static_cast<unsigned int>(std::time(nullptr)));
    // 空のリストを渡された場合（==合法手がない場合）
    if (move_list.empty()) {
        return Move(Move::NONE);
    }

    Move m;
    int random_index;

    while (!move_list.empty()) {
        random_index = mt() % move_list.size();
        m = move_list[random_index];
        // 安全な指し手なら返す
        if (is_safe_move(m, side_to_move, *this)) {
            return m;
        }
        // 安全でない指し手ならリストから削除してループを継続する
        else {
            move_list.erase(move_list.begin() + random_index);
        }
    }

    return Move(Move::NONE);
}

Move Position::select_weighted_random_move(std::vector<Move> &move_list) {
    Bitboard enemy_effect = all_effect(~side_to_move);
    Bitboard our_effect = all_effect(side_to_move);
    Bitboard danger_zone = enemy_effect & ~our_effect;
    static std::mt19937 mt =
        std::mt19937(static_cast<unsigned int>(std::time(nullptr)));

    if (move_list.empty()) {
        return Move(Move::NONE);
    }

    std::vector<Move> capture_list = {};
    std::vector<Move> check_list = {};
    std::vector<Move> danger_list =
        {}; // 敵の利きがあり、かつ自分の利きがないマスに移動する手
    std::vector<Move> other_list = {};

    for (const auto &move : move_list) {
        bool is_capture = move.get_captured_piece() != NO_PIECE &&
                          move.get_captured_piece() != PAWN;
        bool is_check = move.is_check(*this);
        bool is_danger = move.is_danger(*this, danger_zone);
        if (is_capture && is_check) {
            capture_list.push_back(move);
            check_list.push_back(move);
        } else if (is_capture) {
            capture_list.push_back(move);
        } else if (is_danger) {
            danger_list.push_back(move);
        } else if (is_check) {
            check_list.push_back(move);
        } else {
            other_list.push_back(move);
        }
    }

    // この比率でランダムに手を選ぶ
    int capture_weight = 30;
    int check_weight = 6;
    int danger_weight = 1;
    int other_weight = 2;

    Move best_move = select_by_weight(capture_list, check_list, danger_list,
                                      other_list, capture_weight, check_weight,
                                      danger_weight, other_weight);

    return best_move;
}

Move Position::select_by_weight(std::vector<Move> &mlist1,
                                std::vector<Move> &mlist2, int w1, int w2) {
    Move move;
    static std::mt19937 mt =
        std::mt19937(static_cast<unsigned int>(std::time(nullptr)));
    int r = mt() % (w1 + w2);
    if (r < w1) {
        move = select_random_move(mlist1);
        if (move.is_none()) {
            move = select_random_move(mlist2);
        }
        return move;
    } else {
        move = select_random_move(mlist2);
        if (move.is_none()) {
            move = select_random_move(mlist1);
        }
        return move;
    }
}

Move Position::select_by_weight(std::vector<Move> &mlist1,
                                std::vector<Move> &mlist2,
                                std::vector<Move> &mlist3, int w1, int w2,
                                int w3) {
    static std::mt19937 mt =
        std::mt19937(static_cast<unsigned int>(std::time(nullptr)));
    int r = mt() % (w1 + w2 + w3);
    Move move;
    if (r < w1) {
        move = select_random_move(mlist1);
        if (move.is_none()) {
            move = select_by_weight(mlist2, mlist3, w2, w3);
        }
        return move;
    } else if (r < w1 + w2) {
        move = select_random_move(mlist2);
        if (move.is_none()) {
            move = select_by_weight(mlist1, mlist3, w1, w3);
        }
        return move;
    } else {
        move = select_random_move(mlist3);
        if (move.is_none()) {
            move = select_by_weight(mlist1, mlist2, w1, w2);
        }
        return move;
    }
}

Move Position::select_by_weight(std::vector<Move> &mlist1,
                                std::vector<Move> &mlist2,
                                std::vector<Move> &mlist3,
                                std::vector<Move> &mlist4, int w1, int w2,
                                int w3, int w4) {
    static std::mt19937 mt =
        std::mt19937(static_cast<unsigned int>(std::time(nullptr)));
    int r = mt() % (w1 + w2 + w3 + w4);
    Move move;
    if (r < w1) {
        move = select_random_move(mlist1);
        if (move.is_none()) {
            move = select_by_weight(mlist2, mlist3, mlist4, w2, w3, w4);
        }
        return move;
    } else if (r < w1 + w2) {
        move = select_random_move(mlist2);
        if (move.is_none()) {
            move = select_by_weight(mlist1, mlist3, mlist4, w1, w3, w4);
        }
        return move;
    } else if (r < w1 + w2 + w3) {
        move = select_random_move(mlist3);
        if (move.is_none()) {
            move = select_by_weight(mlist1, mlist2, mlist4, w1, w2, w4);
        }
        return move;
    } else {
        move = select_random_move(mlist4);
        if (move.is_none()) {
            move = select_by_weight(mlist1, mlist2, mlist3, w1, w2, w3);
        }
        return move;
    }
}