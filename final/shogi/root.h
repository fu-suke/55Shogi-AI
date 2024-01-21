#include "../common/position.h"
#include "node.h"
#include <iomanip>
class Root {
  public:
    Root();
    Position pos;
    Move search();

    // 訓練データ生成用
    double calc_playout_score();

  private:
};
