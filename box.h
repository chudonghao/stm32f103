//
// Created by leu19 on 2017/7/11.
//

#ifndef CDH_BOX_H
#define CDH_BOX_H

#include "point_array.h"

namespace cdh {
    class box_t {
        box_t() {
            position.x = 0;
            position.y = 0;
        }

        static box_t *box;
    public:
        vec2_t position;

        static inline box_t *instance() {
            if (!box)box = new box_t();
            return box;
        }

    };
}


#endif //CDH_BOX_H
