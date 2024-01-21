#pragma once

#include "../common/string_ex.h"
#include "root.h"
#include <string>

const std::string ENGINE_NAME = "random";
const std::string ENGINE_AUTHOR = "yu-suke";

class USI {
  public:
    USI();
    void loop();

  private:
    enum ID { ID_NAME, ID_AUTHOR, ID_NB };
    void send_id(ID id);
    void send_usiok();
    void send_readyok();
    void send_bestmove();
    void usi();
    void isready();
    void position(std::string str);
    void go();
    void test();
    void do_move(Move move);
    int create_train_data();
    Root root;
    torch::Tensor create_tensor();
    std::vector<int> create_board_array();
};