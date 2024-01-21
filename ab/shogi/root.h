#include "../common/position.h"
#include "node.h"

class Root {
  public:
    Root();
    Position pos;

    // 手を選ぶ関数
    Move search();
};
