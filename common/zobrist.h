#include "shogi.h"

// 局面のhash keyを求めるときに用いるZobrist key
namespace Zobrist {
extern HASH_KEY zero; // ゼロ(==0)
extern HASH_KEY side; // 手番(==1)
// 駒pcが盤上sqに配置されている状態を表すZobrist Key
extern HASH_KEY psq[COLOR_PIECE_NB][SQ_NB];
// c側の手駒prが一枚増えるごとにこれを加算するZobristKey
extern HASH_KEY hand[COLOR_NB][RAW_PIECE_NB];
// c側の駒がsqにある状態を表すZobristKey
extern HASH_KEY occ[COLOR_NB][SQ_NB];
void init();
}; // namespace Zobrist
