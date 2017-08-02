//
// Created by leu19 on 2017/8/3.
//

#ifndef CDH_UNDECIDED_H
#define CDH_UNDECIDED_H

#include <cstdio>
#include "step_motor_couple.h"

using namespace glm;
using namespace std;
namespace cdh {
    class ball_t {
        float v_, a_, position_;
    public:
        ball_t() : v_(), a_(), position_() {}

        float v() { return v_; }

        void v(float v_) { this->v_ = v_; }

        float a() { return a_; }

        void a(float a_) { this->a_ = a_; }

        float position() { return position_; }

        void position(float position_) { this->position_ = position_; }
    };

    class track_t {
        float dip_angle_;
        float height_;
        float height_base_;
    public:
        track_t() : dip_angle_(), height_(), height_base_() {}

        void set_base(){
            step_motor_couple_t::set_current_steps(ivec2(0, 0));
        }

        void height_base(float height_base_) { this->height_base_ = height_base_; }

        float dip_angle() { return dip_angle_; }

        void dip_angle(float dip_angle_) { this->dip_angle_ = dip_angle_; }

        float height() { return height_; }

        void height(float height_) { this->height_ = height_; }

        int motor_dip_angle() {
            int res;
            vec2 length;
            float tan_dip_angle = tan(dip_angle_);
            length.x = height_ - (height_base_) * tan(dip_angle_);
            length.y = height_ + (590.f - height_base_) * tan(dip_angle_);
            ivec2 steps = step_motor_couple_t::map_length_to_steps(length);
            printf("motor_dip_angle length=(%f,%f),steps=(%d,%d)\r\n",length.x,length.y,steps.x,steps.y);
            res = step_motor_couple_t::set_next_steps(steps);
            step_motor_couple_t::step();
            return res;
        }

        int motor_height() {
            return motor_dip_angle();
        }
    };

}


#endif //CDH_UNDECIDED_H
