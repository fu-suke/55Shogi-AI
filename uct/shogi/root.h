#include "../common/position.h"
#include "node.h"

class Root {
  public:
    Root();
    Position pos;
    Move search();
};