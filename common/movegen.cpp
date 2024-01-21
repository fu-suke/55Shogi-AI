#include "movegen.h"

Move::Move() { value = NONE; }

// MoveTypeからのコンストラクタ
Move::Move(MoveType move_type) { value = move_type; }

// 文字列からのコンストラクタ
Move::Move(const std::string &move) {
    value = NONE;
    std::string to = move.substr(2, 2);

    // 2文字目が'*'なら駒打ち
    if (move[1] == '*') {
        value |= DROP;
        Piece pr = CHAR_TO_RAW_PIECE.at(move[0]);
        value |= static_cast<uint16_t>(pr << 5);
        int to_index = sq_to_index(to);
        value |= static_cast<uint16_t>(to_index);
        return;
    }

    std::string from = move.substr(0, 2);
    int from_index = sq_to_index(from);
    int to_index = sq_to_index(to);

    value |= static_cast<uint16_t>(from_index << 5); // 移動元のindex
    value |= static_cast<uint16_t>(to_index);        // 移動先のindex

    // 文字列の末尾に"+"があれば成りフラグを立てる
    if (move.size() == 5 && move[4] == '+') {
        value |= PROMOTE;
    }
}

// Squareからのコンストラクタ
Move::Move(Square from, Square to) { value = (to + (from << 5)); }

// PieceとSquareからのコンストラクタ(駒打ち用)
Move::Move(Piece pr, Square to) {
    // prは生駒であることを保証する
    // ASSERT(!is_promoted(pr), "This piece is promoted !!!");
    value = (to + (pr << 5) + DROP);
}

// マス目（"1a", "5e"など）から一意のインデックス（0～24）に変換する関数
int sq_to_index(const std::string &sq) {
    char file_char = sq[0]; // file(筋)
    char rank_char = sq[1]; // rank(段)
    int file = file_char - '1';
    int rank = rank_char - 'a';
    return file * 5 + rank;
}

// 盤面情報を受け取り、手番側の指し手のリストを返す関数
std::vector<Move> generate_move_list(Position &pos) {
    std::vector<Move> move_list; // 指し手のリスト
    move_list = generate_move_list(pos, move_list);
    for (auto &m : move_list) {
        pos.set_captured_piece(m);
    }
    return move_list;
}

std::vector<Move> generate_move_list(Position &pos,
                                     std::vector<Move> &move_list) {
    Color us = pos.side_to_move; // 手番側の色

    // 自玉が不在なら投了
    // 本来は必要ない関数だが、GUIが玉の不在を検知できない場合があるので念のため
    if (pos.piece_bitboards[KING + us * PIECE_WHITE].p == 0) {
        return move_list;
    }

    // 敵玉を捕れる手があるならそれを返す
    Move m = generate_king_capture_move(us, move_list, pos);
    // NONE以外が返ってきたらそれを返す
    if (m != Move(Move::NONE)) {
        move_list.push_back(m);
        return move_list;
    }

    // 自玉に王手がかかっているなら王手回避
    if (pos.is_check(us)) {
        // 玉の指し手を生成
        generate_piece_moves<KING>(us, move_list, pos);
        // 玉以外の駒で王手を防ぐ指し手を生成
        // 合駒で防ぐ手も含まれる
        generate_block_moves(us, move_list, pos);
        return move_list;
    }

    generate_moves(us, move_list, pos); // 駒打ち以外の指し手を生成
    generate_drop_moves(us, move_list, pos); // 駒打ちの指し手を生成

    return move_list;
}

void sort_move_list(std::vector<Move> &move_list, Position &pos) {
    int idx = 0;
    // 指し手のリストで、敵の駒を捕る手が手前側にくるようにソートする
    for (int i = 0; i < move_list.size(); ++i) {
        Move m = move_list[i];
        if (m.is_drop()) {
            continue;
        }
        Piece captured = m.get_captured_piece();
        if (captured != NO_PIECE) {
            // 敵の駒を捕る手なら、その手を前側に移動させる
            std::swap(move_list[idx], move_list[i]);
            idx++;
        }
    }
}

std::vector<Move> generate_capture_mlist(const std::vector<Move> &move_list) {
    std::vector<Move> capture_mlist;
    for (auto m : move_list) {
        if (m.is_drop()) {
            continue;
        }
        Piece captured = m.get_captured_piece();
        if (captured != NO_PIECE && type_of(captured) != PAWN) {
            capture_mlist.push_back(m);
        }
    }
    return capture_mlist;
}

// 駒打ち以外の指し手を生成する関数
void generate_moves(Color color, std::vector<Move> &move_list, Position &pos) {
    generate_piece_moves<PAWN>(color, move_list, pos);
    generate_piece_moves<SILVER>(color, move_list, pos);
    generate_piece_moves<GOLD>(color, move_list, pos);
    generate_piece_moves<BISHOP>(color, move_list, pos);
    generate_piece_moves<ROOK>(color, move_list, pos);
    generate_piece_moves<PRO_PAWN>(color, move_list, pos);
    generate_piece_moves<PRO_SILVER>(color, move_list, pos);
    generate_piece_moves<HORSE>(color, move_list, pos);
    generate_piece_moves<DRAGON>(color, move_list, pos);
    generate_piece_moves<KING>(color, move_list, pos);
}

// targetへの駒打ちを生成する関数
// 駒打ちの指し手を生成する関数
void generate_drop_moves(Color color, Bitboard target,
                         std::vector<Move> &move_list, Position &pos) {
    // コマが無く、かつtargetに含まれる場所を探す
    Bitboard not_occupied = ~pos.occupied_bb(COLOR_NB) & target;

    // 歩だけ二歩と打ち歩詰めをチェックする
    if (hand_exists(pos.hands[color], PAWN)) {
        // 二歩対策
        // 歩がある筋は除外する
        Bitboard pawn =
            pos.piece_bitboards[PAWN + color * PIECE_WHITE]; // 自陣の歩
        Square pawn_sq = pawn.pop();          // 歩があるマス
        File pawn_file = sq_to_file(pawn_sq); // 歩がある筋
        // 二歩になる筋と、成れる場所（＝敵陣）を除外する
        Bitboard not_occupied_for_pawn =
            not_occupied & ~FILE_BB[pawn_file] & ~PROMOTE_ZONE[color];

        // 打ち歩詰めはis_safe_move()でチェックし、ここではチェックしない
        while (not_occupied_for_pawn.p != 0) {
            Square to = not_occupied_for_pawn.pop(); // 駒打ちする場所
            Move m = Move(PAWN, to);
            move_list.push_back(m);
        }
    }

    while (not_occupied.p != 0) {
        Square to = not_occupied.pop(); // 駒打ちする場所

        // 銀
        if (hand_exists(pos.hands[color], SILVER)) {
            Move m = Move(SILVER, to);
            move_list.push_back(m);
        }
        // 金
        if (hand_exists(pos.hands[color], GOLD)) {
            Move m = Move(GOLD, to);
            move_list.push_back(m);
        }
        // 角
        if (hand_exists(pos.hands[color], BISHOP)) {
            Move m = Move(BISHOP, to);
            move_list.push_back(m);
        }
        // 飛車
        if (hand_exists(pos.hands[color], ROOK)) {
            Move m = Move(ROOK, to);
            move_list.push_back(m);
        }
    }
}

// 駒打ちの指し手（空いている場所全てが駒打ち候補）を生成する関数
void generate_drop_moves(Color color, std::vector<Move> &move_list,
                         Position &pos) {
    generate_drop_moves(color, Bitboard(0xFFFFFFFF), move_list, pos);
}

// 王手を防ぐために、玉以外の駒で王手を防ぐ指し手を生成し、move_listに追加する関数
void generate_block_moves(Color color, std::vector<Move> &move_list,
                          Position &pos) {
    int checkers_cnt = 0; // 王手をしている駒の数
    const Color enemy_color = ~color;
    // 「王手をしている駒と玉の間のマス」に1を立てたBitboard
    Bitboard between_bb;
    Square king_sq = pos.king_square(color); // 自玉の位置
    Square checker_sq;

    // 王手をしている敵駒の位置を取得
    for (auto &[piece, effect_func] : Piece_to_EffectFunc) {
        Bitboard bb = pos.piece_bitboards[piece + enemy_color * PIECE_WHITE];
        Square enemy_sq;
        while (bb.p != 0) {
            enemy_sq = bb.pop();
            // 敵の駒の利きと、自分の玉の位置が重複するなら
            if ((pos.*effect_func)(enemy_sq, enemy_color) &&
                pos.piece_bitboards[KING + color * PIECE_WHITE]) {
                checker_sq = enemy_sq;
                ++checkers_cnt;
                if (checkers_cnt >= 2) {
                    // cout << "double check" << endl;
                    return; // 両王手なら玉を動かすしかないので、ここで終了
                }
                between_bb = get_between_bb(enemy_sq, king_sq);
            }
        }
    }

    // この時点で、between_bbが完成しており、かつ王手をしている敵の駒は1つに限定された
    // この関数が実行されている時点で、王手を仕掛けている駒が存在するはずなので
    // checkers_cnt == 0 もあり得ない
    // ASSERT(checkers_cnt == 1, "checkers_cnt is not 1");

    // 合駒で王手を防ぐ
    generate_drop_moves(color, between_bb, move_list, pos);

    // 移動合いで王手を防ぐ
    // 攻撃してきている敵の駒の位置も移動先の候補となる（その駒を捕ってしまえば防御できるので）
    between_bb |= Bitboard(checker_sq);
    generate_piece_moves<PAWN>(color, move_list, between_bb, pos);
    generate_piece_moves<SILVER>(color, move_list, between_bb, pos);
    generate_piece_moves<GOLD>(color, move_list, between_bb, pos);
    generate_piece_moves<BISHOP>(color, move_list, between_bb, pos);
    generate_piece_moves<ROOK>(color, move_list, between_bb, pos);
    generate_piece_moves<PRO_PAWN>(color, move_list, between_bb, pos);
    generate_piece_moves<PRO_SILVER>(color, move_list, between_bb, pos);
    generate_piece_moves<HORSE>(color, move_list, between_bb, pos);
    generate_piece_moves<DRAGON>(color, move_list, between_bb, pos);
}

// 敵玉を捕れる指し手を生成する関数
Move generate_king_capture_move(Color color, std::vector<Move> &move_list,
                                Position &pos) {
    Bitboard enemy_king = pos.piece_bitboards[KING + ~color * PIECE_WHITE];

    //  自軍の利きと、敵玉の位置が重複するなら
    if (pos.is_check(~color)) {
        // move_list からそのようなmoveを探し出して返す
        Square to = enemy_king.pop();
        for (auto m : move_list) {
            if (m.get_to() == to) {
                // 敵玉を捕れるMoveであれば1つ返せば十分
                return m;
            }
        }
    }
    return Move(Move::NONE);
}

template <Piece piece>
// ある種の駒（複数枚可）がtargetに移動するような指し手を生成する関数
void generate_piece_moves(Color color, std::vector<Move> &mlist,
                          const Bitboard target, Position &pos) {
    // 駒の種類が正しいかチェック
    // ASSERT(piece == PAWN || piece == SILVER || piece == BISHOP ||
    //            piece == ROOK || piece == GOLD || piece == KING ||
    //            piece == PRO_PAWN || piece == PRO_SILVER || piece == HORSE ||
    //            piece == DRAGON,
    //        "Unsupported piece type");

    Bitboard pieces = pos.piece_bitboards[piece + color * PIECE_WHITE];
    Bitboard bb;
    while (pieces.p != 0) {
        Square from = pieces.pop();

        // 利き（移動先の候補）を計算
        bb = (pos.*Piece_to_EffectFunc[piece])(from, color);
        // 自軍の駒がある場所は移動先から除外する
        bb &= ~pos.occupied_bb(color);
        // targetに行けないような場所は移動先から除外する
        bb &= target;

        // 歩・飛車・角（成れるなら必ず成る属）
        if constexpr (piece == PAWN || piece == BISHOP || piece == ROOK) {
            while (bb.p != 0) {
                Square to = bb.pop();
                Move m = Move(from, to);
                // 成れるなら必ず成る
                if (m.can_promote(color)) {
                    m.promote();
                }
                mlist.push_back(m);
            }
        }
        // と・金・全・馬・竜（成れない属）
        else if constexpr (piece == PRO_PAWN || piece == GOLD ||
                           piece == PRO_SILVER || piece == HORSE ||
                           piece == DRAGON) {
            while (bb.p != 0) {
                Square to = bb.pop();
                Move m = Move(from, to);
                mlist.push_back(m);
            }
        }
        // 銀（成り・不成どちらの手もある）
        else if constexpr (piece == SILVER) {
            while (bb.p != 0) {
                Square to = bb.pop();
                Move m = Move(from, to);
                // 不成の手を生成
                mlist.push_back(m);
                // 成りの手を生成
                if (m.can_promote(color)) {
                    m.promote();
                    mlist.push_back(m);
                }
            }
        }
        // 玉
        else if constexpr (piece == KING) {
            // 自玉を除く（敵の飛車角の利きに影響するので）
            Square king_sq = pos.king_square(color);
            Piece king = pos.clear_piece(king_sq);
            // 自玉が居ない状態での敵の利きを再計算
            // （例：左から飛車の王手がかかっているとき、玉が１マス右に逃げてしまうのを防止する）
            Bitboard enemy_effect_bb = pos.all_effect(~color);
            // 敵の駒の利きでない、かつ自分の利き、かつ自分の駒がない場所を取得
            Bitboard safety_zone = ~enemy_effect_bb &
                                   pos.king_effect(from, color) &
                                   ~pos.occupied_bb(color);
            // その場所へ移動するような指し手を生成
            while (safety_zone.p != 0) {
                Square to = safety_zone.pop();
                Move m = Move(from, to);
                mlist.push_back(m);
            }
            // 玉を戻す
            pos.set_piece(king_sq, king);
        }
    }
}

// 特にtargetを指定しない場合の指し手生成関数
template <Piece piece>
void generate_piece_moves(Color color, std::vector<Move> &mlist,
                          Position &pos) {
    Bitboard all = Bitboard(0xFFFFFFFF);
    generate_piece_moves<piece>(color, mlist, all, pos);
}

// そのMoveをしたときに、color側が王手になるかどうかを判定する関数
bool is_safe_move(Move move, Color color, Position &pos) {
    // 指し手生成アルゴリズムの原理的に、駒打ちの指し手は安全である
    if (move.is_drop()) {
        // 打ち歩詰めチェック
        if (move.get_dropped_piece() == PAWN && move.is_check(pos)) {
            pos.do_move(move);
            std::vector<Move> mlist = generate_move_list(pos);
            Move m = pos.select_random_move(mlist);
            if (m.is_none()) {
                pos.undo_move(move);
                return false;
            }
            pos.undo_move(move);
        }
        return true;
    }
    return is_safe_move(move.get_from(), move.get_to(), color, pos);
}

// Sqにある駒を移動させたときに、color側が王手になるかどうかを判定する関数
bool is_safe_move(Square from, Square to, Color color, Position &pos) {
    // ASSERT(color_of(pos.piece_board[from]) == color, "move is not legal
    // !!!"); 駒を実際に移動させる
    Piece removed = pos.move_piece(from, to, false);
    // 自玉の王手チェック
    bool is_safe = !pos.is_check(color);
    // 駒を戻す。unmove->set_pieceの順番に注意
    pos.unmove_piece(from, to, false);
    // 取り除いた敵の駒があれば戻す
    if (removed) {
        pos.set_piece(to, removed);
    }
    return is_safe;
}

bool Move::is_check(Position &pos) const {
    // 指し手を実行
    pos.do_move(*this);
    // 王手チェック
    bool is_check = pos.is_check(pos.side_to_move);
    // 指し手を戻す
    pos.undo_move(*this);
    return is_check;
}

bool Move::is_danger(Position &pos, Bitboard &danger_zone) const {
    Square to = get_to();
    if (is_drop()) {
        return danger_zone.check_bit(to);
    }
    Square from = get_from();
    // ASSERT(color_of(pos.piece_board[from]) == pos.side_to_move,
    //        "move is not legal !!!");
    // 駒を実際に移動させる
    Piece removed = pos.move_piece(from, to, false);
    Bitboard enemy_effect = pos.all_effect(~pos.side_to_move);
    Bitboard our_effect = pos.all_effect(pos.side_to_move);
    Bitboard d = enemy_effect & ~our_effect;
    bool is_danger = d.check_bit(to);
    // 駒を戻す。unmove->set_pieceの順番に注意
    pos.unmove_piece(from, to, false);
    // 取り除いた敵の駒があれば戻す
    if (removed) {
        pos.set_piece(to, removed);
    }
    return is_danger;
}