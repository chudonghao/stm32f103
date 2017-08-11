//
// Created by leu19 on 2017/7/28.
//

#ifndef CDH_KEY_BOARD_H
#define CDH_KEY_BOARD_H

namespace cdh{

    class key_board_t {
    public:
        enum key_board_e{
            key_board_none_e,
            key_board_0_e,
            key_board_1_e,
            key_board_2_e,
            key_board_3_e,
            key_board_4_e,
            key_board_5_e,
            key_board_6_e,
            key_board_7_e,
            key_board_8_e,
            key_board_9_e,
            key_board_a_e,
            key_board_b_e,
            key_board_c_e,
            key_board_d_e,
            key_board_e_e,
            key_board_f_e,
        };
        static key_board_e scan();
    };
}



#endif //CDH_KEY_BOARD_H
