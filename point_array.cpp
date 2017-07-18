//
// Created by leu19 on 2017/7/12.
//

#include <cmath>
#include <cstdio>
#include "point_array.h"

using namespace std;
namespace cdh {

    vec2_t *line_t::next_point() {
        ++current_point_index;
        if(current_point_index == div)
            return &end;
        if(current_point_index == div + 1)
            return NULL;
        current_point += dir;
        return &current_point;
    }

    vec2_t *circle_t::next_point() {
        if (all_readed)
            return 0;
        if (current_point_index >= div) {
            all_readed = true;
            current_point.y = center.y;
            current_point.x = center.x + r;
        } else {
            current_point.x = center.x + r * cos(2 * 3.141592653f * current_point_index / div);
            current_point.y = center.y + r * sin(2 * 3.141592653f * current_point_index / div);
            ++current_point_index;
        }
        return &current_point;
    }

    static void map_index_to_point(int index, cdh::vec2_t &dir) {
        switch (index) {
            case 0:
                dir.x = 4;
                dir.y = 0;
                break;
            case 1:
                dir.x = 3;
                dir.y = 3;
                break;
            case 2:
                dir.x = 0;
                dir.y = 4;
                break;
            case 3:
                dir.x = -3;
                dir.y = 3;
                break;
            case 4:
                dir.x = -4;
                dir.y = 0;
                break;
            case 5:
                dir.x = -3;
                dir.y = -3;
                break;
            case 6:
                dir.x = 0;
                dir.y = -4;
                break;
            case 7:
                dir.x = 3;
                dir.y = -3;
                break;
        }
    }

    vec2_t *tracking_t::next_point() {
        //        2
        //   3         1
        // 4             0
        //   5         7
        //        6
        unsigned char data = infra_red->data();

        //第一次检测到有效电平
        if (current_dir.x == 0 && current_dir.y == 0) {
            if (data == 0) {
                fix_line_start();
                ++fix_line_start_count;
            } else {
                fix_line_start_count = 0;
                for (int i = 0; i < 8; ++i) {
                    if (((data >> i) & 0x1) == 1) {// 有效电平
                        vec2_t point;
                        map_index_to_point(i, point);
                        current_dir.x = point.x;
                        current_dir.y = point.y;
                    }
                }
            }

        } else if (data == 0x00) {
            ++fix_line_end_count;
            if (fix_line_end_count >= 0) {
                return fix_line_end();
            }
        } else {
            fix_line_end_count = -10;
            for (int i = 0; i < 8; ++i) {
                if (((data >> i) & 0x1) == 1) {// 有效电平
                    vec2_t point;
                    map_index_to_point(i, point);
                    float cos_s =
                            (current_dir.x * point.x + current_dir.y * point.y) / 2 / sqrt(float(point.length2())) /
                            sqrt(current_dir.x * current_dir.x + current_dir.y * current_dir.y);

                    if (cos_s >= 0) {
                        current_dir.x *= 4;
                        current_dir.y *= 4;
                        current_dir.x += (cos_s + 0.5) * point.x;
                        current_dir.y += (cos_s + 0.5) * point.y;
                        current_dir.x /= (4 + (cos_s + 0.5));
                        current_dir.y /= (4 + (cos_s + 0.5));
//                    if (current_dir.x * point.x + current_dir.y * point.y >= 0) {
//                        current_dir.x *= 2;
//                        current_dir.y *= 2;
//                        current_dir.x += 1 * point.x;
//                        current_dir.y += 1 * point.y;
//                        current_dir.x /= (1 + 2);
//                        current_dir.y /= (1 + 2);
                    }
                }
            }
        }
        old_data = data;
        if (current_dir.length2() < 9)
            current_dir.x *= 1.5;
        current_point.x += current_dir.x;
        current_point.y += current_dir.y;
        return &current_point;
    }

    static vec2f_t rotate(const vec2f_t &point, float a) {
        vec2f_t res;
        res.x = point.x * cos(a) - point.y * sin(a);
        res.y = point.x * sin(a) + point.y * cos(a);
        return res;
    }

    static vec2f_t normolize_to_5(const vec2f_t &point) {
        float length = sqrt(point.x * point.x + point.y * point.y);
        vec2f_t res;
        res.x = point.x * 5 / length;
        res.y = point.y * 5 / length;
        return res;
    }

    vec2_t *tracking_t::fix_line_end() {
        vec2f_t move;
        switch (fix_line_end_count) {
            case 0:
            case 1:
            case 2:
            case 9:
            case 10:
            case 11:
                move = rotate(normolize_to_5(current_dir), 1.57);
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                move = rotate(normolize_to_5(current_dir), -1.57);
                break;
            default:
                return NULL;
        }
        //printf("fix_line_end():%d\r\n", fix_line_end_count);
        //printf("current_point:%d,%d\r\n", current_point.x, current_point.y);
        //printf("current_dir:%f,%f\r\n", current_dir.x, current_dir.y);
        //printf("move:%f,%f\r\n", move.x, move.y);
        ++fix_line_end_count;
        current_point.x += move.x;
        current_point.y += move.y;
        return &current_point;
    }

    vec2_t *tracking_t::fix_line_start() {
        vec2_t move;
        switch (fix_line_start_count) {
            case 0:
            case 1:
            case 6:
            case 7:
                move.x = 5;
                move.y = 0;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                move.x = -5;
                move.y = 0;
                break;
            case 8:
            case 9:
            case 14:
            case 15:
                move.x = 0;
                move.y = 5;
                break;
            case 10:
            case 11:
            case 12:
            case 13:
                move.x = 0;
                move.y = -5;
                break;
            default:
                return NULL;
        }
        current_point += move;
        return &current_point;
    }

    float vec2f_t::length2() {
        return x * x + y * y;
    }

    vec2_t *heart_line_t::next_point() {
        if (t >= 6.28)
            return NULL;
        float sin_t = sin(t);
        current_point.x = 16 * sin_t * sin_t * sin_t;
        current_point.y = 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);
        current_point.x *= 15;
        current_point.y *= 15;
        t += 0.001;
        current_point += center;
        return &current_point;
    }
}
