//
// Created by leu19 on 2017/7/21.
//

#ifndef CDH_VEC2_H
#define CDH_VEC2_H

#include <cmath>
# define M_PI       3.14159265358979323846f  /* pi */
# define M_PI_2     1.57079632679489661923f  /* pi/2 */
# define M_PI_4     0.78539816339744830962f  /* pi/4 */
# define M_1_PI     0.31830988618379067154f  /* 1/pi */
# define M_2_PI     0.63661977236758134308f  /* 2/pi */

namespace cdh {
    template<typename T>
    class vec2 {
    public:
        T x;
        T y;

        vec2() : x(static_cast<T>(0)), y(static_cast<T>(0)) {}

        vec2(T x, T y) : x(x), y(y) {}
    };

    template<typename T>
    static inline vec2<T> operator-(const vec2<T> &point1, const vec2<T> &point2) {
        return vec2<T>(point1.x - point2.x, point1.y - point2.y);
    }

    template<typename T>
    static inline vec2<T> operator+(const vec2<T> &point1, const vec2<T> &point2) {
        return vec2<T>(point1.x + point2.x, point1.y + point2.y);
    }

    template<typename T>
    static inline vec2<T> &operator+=(vec2<T> &point1, const vec2<T> &point2) {
        point1 = point1 + point2;
        return point1;
    }

    template<typename T>
    static inline vec2<T> &operator-=(vec2<T> &point1, const vec2<T> &point2) {
        point1 = point1 - point2;
        return point1;
    }

    template<typename T>
    static inline vec2<T> operator*(const vec2<T> &point1, T i) { return vec2<T>(point1.x * i, point1.y * i); }

    template<typename T>
    static inline vec2<T> operator/(const vec2<T> &point1, T i) { return vec2<T>(point1.x / i, point1.y / i); }

    template<typename T>
    static inline vec2<T> &operator*=(vec2<T> &point1, T i) {
        point1 = point1 * i;
        return point1;
    }

    template<typename T>
    static inline vec2<T> &operator/=(vec2<T> &point1, T i) {
        point1 = point1 / i;
        return point1;
    }

    template<typename T>
    static inline bool operator==(const vec2<T> &point1, const vec2<T> &point2) {
        return point1.x == point2.x && point1.y == point2.y;
    }
    template<typename T>
    static inline float length(const vec2<T> &point1) {
        return std::sqrt(point1.x*point1.x + point1.y*point1.y);
    }
    template <typename T>
    static inline float angle(const vec2<T> &point1, const vec2<T> &point2){
        return std::acos(
                ((point1.x*point2.x)+(point1.y*point2.y))
                / length(point1) / length(point2)
        );
    }

}


#endif //CDH_VEC2_H
