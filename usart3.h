//
// Created by leu19 on 2017/7/29.
//

#ifndef CDH_USART3_H
#define CDH_USART3_H

namespace cdh{
class usart3_t {
public:
    static bool have_sentence();
    static char* c_str();
    static void print(char*);
};

}


#endif //CDH_USART3_H
