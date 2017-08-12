//
// Created by leu19 on 2017/8/12.
//

#ifndef CDH_MAIN_H
#define CDH_MAIN_H

extern "C" unsigned char uart1_have_data_to_read();

extern bool code_type_1;
void pid_init_1();
void pid_loop_1();
void key_board_init_1();
void key_board_loop_1();
void main_init_1();
void main_loop_1();
#endif //CDH_MAIN_H
