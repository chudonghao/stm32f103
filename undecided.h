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
        vec2 v_, a_, position_;
    public:
        ball_t() : v_(0.f, 0.f), a_(0.f, 0.f), position_(0.f, 0.f) {}

        const vec2 &v() { return v_; }

        void v(const vec2 &v_) { this->v_ = v_; }

        const vec2 &a() { return a_; }

        void a(const vec2 &a_) { this->a_ = a_; }

        const vec2 &position() { return position_; }

        void position(const vec2 &position_) { this->position_ = position_; }
    };

    class track_t {
        float dip_angle_;
        float height_;
        float height_base_;
    public:
        track_t() : dip_angle_(), height_(), height_base_() {}

        void set_base() {
            dip_angle_ = 0.f;
            height_ = 0.f;
            height_base_ = 0.f;
            step_motor_couple_t::set_current_steps(ivec2(0, 0));
        }

        void height_base(float height_base_) { this->height_base_ = height_base_; }

        float dip_angle() {
            vec2 length = step_motor_couple_t::current_length();
            return (length.y - length.x) / 590.f;
        }

        void dip_angle(float dip_angle_) { this->dip_angle_ = dip_angle_; }

        float height() { return height_; }

        void height(float height_) { this->height_ = height_; }

        int motor() {
            int res;
            vec2 length;
            float tan_dip_angle = tan(dip_angle_);
            length.x = height_ - (height_base_) * tan(dip_angle_);
            length.y = height_ + (590.f - height_base_) * tan(dip_angle_);
            ivec2 steps = step_motor_couple_t::map_length_to_steps(length);
            //printf("motor_dip_angle length=(%f,%f),steps=(%d,%d)\r\n",length.x,length.y,steps.x,steps.y);
            res = step_motor_couple_t::set_next_steps(steps, true);
            step_motor_couple_t::step();
            return res;
        }
    };

    const static vec2 distance_motors_and_zero = vec2(650.f/2.f,650.f/2.f);//TODO

    class flat_board_t {
        vec2 dip_angle_;
    public:
        flat_board_t() : dip_angle_(0.f, 0.f) {}

        vec2 real_dip_angle() {
            vec2 res = step_motor_couple_t::current_length();
            res.x /= distance_motors_and_zero.x;
            res.y /= distance_motors_and_zero.y;
            return res;
        }
        const vec2 &dip_angle() { return dip_angle_; }

        void dip_angle(const vec2 &dip_angle_) { this->dip_angle_ = dip_angle_; }

        void set_base() {
            dip_angle_ = vec2(0.f, 0.f);
            step_motor_couple_t::set_current_steps(ivec2(0,0));
        }
        int motor(){
            int res;
            vec2 length;
            length.x = tan(dip_angle_.x) * distance_motors_and_zero.x;
            length.y = tan(dip_angle_.y) * distance_motors_and_zero.y;
            ivec2 steps = step_motor_couple_t::map_length_to_steps(length);
            res = step_motor_couple_t::set_next_steps(steps, true);
            step_motor_couple_t::step();
            return res;
        }
    };
}


#endif //CDH_UNDECIDED_H
