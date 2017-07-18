//
// Created by leu19 on 2017/7/12.
//

#ifndef CDH_POINT_ARRAY_H
#define CDH_POINT_ARRAY_H

#include <cstddef>
#include "infra_red.h"

namespace cdh {

    class vec2_t {
    public:
        int x;
        int y;

        vec2_t(int x = 0, int y = 0) : x(x), y(y) {}

        inline int length2() const {
            return x * x + y * y;
        }
    };

    class vec2f_t {
    public:
        float x;
        float y;

        float length2();
    };

    static inline vec2_t operator-(const vec2_t &point1, const vec2_t &point2) {
        return vec2_t(point1.x - point2.x, point1.y - point2.y);
    }

    static inline vec2_t operator+(const vec2_t &point1, const vec2_t &point2) {
        return vec2_t(point1.x + point2.x, point1.y + point2.y);
    }

    static inline vec2_t &operator+=(vec2_t &point1, const vec2_t &point2) {
        point1 = point1 + point2;
        return point1;
    }

    static inline vec2_t &operator/=(vec2_t &point1, int i) {
        point1.x /= i;
        point1.y /= i;
        return point1;
    }

    static inline vec2_t &operator*=(vec2_t &point1, int i) {
        point1.x *= i;
        point1.y *= i;
        return point1;
    }

    static inline vec2_t operator*(const vec2_t &point1, int i) { return vec2_t(point1.x * i, point1.y * i); }

    static inline vec2_t operator/(const vec2_t &point1, float i) { return vec2_t(point1.x / i, point1.y / i); }

    static inline bool operator==(const vec2_t &point1, const vec2_t &point2) {
        return point1.x == point2.x && point1.y == point2.y;
    }

    class point_array_t {
    public:
        virtual ~point_array_t() {}

        virtual vec2_t *next_point() = 0;
    };

    class line_t : public point_array_t {
        vec2_t start, end;
        vec2_t current_point;
        vec2_t dir;
        int current_point_index;
        int div;
    public:
        line_t(const vec2_t &start, const vec2_t &end, const int div = 30) {
            current_point_index = 0;
            this->start = start;
            this->end = end;
            vec2_t l = start - end;
            if (l.length2() < 900)
                this->div = 1;
            else
                this->div = div;
            dir = end - start;
            dir /= div;
            current_point = start;
        }

        virtual vec2_t *next_point();
    };

    class circle_t : public point_array_t {
        bool all_readed;
        int div;
        vec2_t center;
        int r;
        vec2_t current_point;
        int current_point_index;
    public:
        circle_t(const vec2_t &center, int r) {
            all_readed = false;
            this->center = center;
            this->r = r;
            div = 360;
            current_point_index = 0;
            current_point.y = center.y;
            current_point.x = center.x + r;
        }

        virtual vec2_t *next_point();
    };

    class heart_line_t : public point_array_t {
        float t;
        vec2_t center;
        vec2_t current_point;
    public:
        heart_line_t() {
            this->center = vec2_t(400, 700);
            t = 0.1;
        }

        virtual vec2_t *next_point();
    };

    class tracking_t : public point_array_t {

        infra_red_t *infra_red;
        int fix_line_end_count;
        int fix_line_start_count;
        vec2f_t current_dir;
        vec2_t current_point;
    public:
        tracking_t(const vec2_t &current_point) {
            this->current_point = current_point;
            this->current_dir = vec2f_t();
            this->fix_line_end_count = 0;
            fix_line_start_count = 0;
            old_data = 0;
            infra_red = infra_red_t::open();
        }

        virtual vec2_t *next_point();

        vec2_t *fix_line_start();

        vec2_t *fix_line_end();

        unsigned char old_data;
    };
}


#endif //CDH_POINT_ARRAY_H
