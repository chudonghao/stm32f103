
#include <stm32f10x_conf.h>
#include "usart1.h"
int main(){
    SystemCoreClockUpdate();
    usart1_t::init();
    
    
}
