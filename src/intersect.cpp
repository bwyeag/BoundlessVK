#include "intersect.hpp"

namespace BL {
bool intersect_ray_sphere(const vec3f& o,
                          const vec3f& d,
                          const vec3f& c,
                          float r) {
    vec3f l = c - o;
    float s = l.dot(d);
    float l2 = l.dot(l);
    float r2 = r * r;
    if (s < 0.0 || l2 > r2)
        return false;
    float m2 = l2 - s * s;
    if (m2 > r2)
        return false;
    return true;
}
bool intersect_ray_sphere_res(const vec3f& o,
                              const vec3f& d,
                              const vec3f& c,
                              float r,
                              float* t) {
    vec3f l = c - o;
    float s = l.dot(d);
    float l2 = l.dot(l);
    float r2 = r * r;
    if (s < 0.0 || l2 > r2)
        return false;
    float m2 = l2 - s * s;
    if (m2 > r2)
        return false;
    float q = std::sqrt(r2 - m2);
    if (l2 > r2)
        *t = s - q;
    else
        *t = s + q;
    return true;
}
bool intersect_ray_OBB_res(const vec3f& o,
                           const vec3f& d,
                           const OBB& A,
                           float* t) {
    float t_min = -std::numeric_limits<float>::infinity();
    float t_max = std::numeric_limits<float>::infinity();
    vec3f p = A.c() - o;

    float e = p.dot(A.u());
    float f = d.dot(A.u());
    if (std::abs(f) > ϵ) {
        float f_i = 1 / f;
        float t1 = (e + A.h_u()) * f_i;
        float t2 = (e - A.h_u()) * f_i;
        if (t1 > t2)
            std::swap(t1, t2);
        t_min = std::max(t1, t_min);
        t_max = std::min(t2, t_max);
        if (t_max < 0.0f || t_min > t_max)
            return false;
    } else if (-e - A.h_u() > 0.0f || -e + A.h_u() < 0.0f)
        return false;

    e = p.dot(A.v());
    f = d.dot(A.v());
    if (std::abs(f) > ϵ) {
        float f_i = 1 / f;
        float t1 = (e + A.h_v()) * f_i;
        float t2 = (e - A.h_v()) * f_i;
        if (t1 > t2)
            std::swap(t1, t2);
        t_min = std::max(t1, t_min);
        t_max = std::min(t2, t_max);
        if (t_max < 0.0f || t_min > t_max)
            return false;
    } else if (-e - A.h_v() > 0.0f || -e + A.h_v() < 0.0f)
        return false;

    vec3f w = A.w();
    e = p.dot(w);
    f = d.dot(w);
    if (std::abs(f) > ϵ) {
        float f_i = 1.0f / f;
        float t1 = (e + A.h_w()) * f_i;
        float t2 = (e - A.h_w()) * f_i;
        if (t1 > t2)
            std::swap(t1, t2);
        t_min = std::max(t1, t_min);
        t_max = std::min(t2, t_max);
        if (t_max < 0.0f || t_min > t_max)
            return false;
    } else if (-e - A.h_w() > 0.0f || -e + A.h_w() < 0.0f)
        return false;

    if (t_min > 0.0f)
        *t = t_min;
    else
        *t = t_max;
    return true;
}
bool intersect_ray_AABB_res(const vec3f& o,
                            const vec3f& d,
                            const AABB& A,
                            float* t) {
    float t_min = -std::numeric_limits<float>::infinity();
    float t_max = std::numeric_limits<float>::infinity();
    vec3f amin = A.min();
    vec3f amax = A.max();

    if (std::abs(d.x()) < ϵ) {
        if (o.x() < amin.x() || o.x() > amax.x())
            return false;
    } else {
        float d_i = 1.0f / d.x();
        float t1 = (amin.x() - o.x()) * d_i;
        float t2 = (amax.x() - o.x()) * d_i;
        if (t1 > t2)
            std::swap(t1, t2);
        t_min = std::max(t1, t_min);
        t_max = std::min(t2, t_max);
        if (t_min > t_max)
            return false;
    }
    if (std::abs(d.y()) < ϵ) {
        if (o.y() < amin.y() || o.y() > amax.y())
            return false;
    } else {
        float d_i = 1.0f / d.y();
        float t1 = (amin.y() - o.y()) * d_i;
        float t2 = (amax.y() - o.y()) * d_i;
        if (t1 > t2)
            std::swap(t1, t2);
        t_min = std::max(t1, t_min);
        t_max = std::min(t2, t_max);
        if (t_min > t_max)
            return false;
    }
    if (std::abs(d.z()) < ϵ) {
        if (o.z() < amin.z() || o.z() > amax.z())
            return false;
    } else {
        float d_i = 1.0f / d.z();
        float t1 = (amin.z() - o.z()) * d_i;
        float t2 = (amax.z() - o.z()) * d_i;
        if (t1 > t2)
            std::swap(t1, t2);
        t_min = std::max(t1, t_min);
        t_max = std::min(t2, t_max);
        if (t_min > t_max)
            return false;
    }
    if (t_min > 0.0f)
        *t = t_min;
    else
        *t = t_max;
    return true;
}
int intersect_plane_AABB(const vec3f& n, float d, const AABB& A) {
    vec3f amin = A.min(), amax = A.max(),  // 获取A的对角位置
        n_abs = n.array().abs();
    float e = A.h().dot(n_abs);
    float s = A.c().dot(n) + d;
    if (s - e > 0.0f)
        return -1;
    if (s + e < 0.0f)
        return +1;
    return 0;
}
int intersect_plane_OBB(const vec3f& n, float d, const OBB& A) {
    vec3f amin = A.min(), amax = A.max();
    vec3f n_abs;
    n_abs.x() = A.u().dot(n);
    n_abs.y() = A.v().dot(n);
    n_abs.z() = A.w().dot(n);
    n_abs = n_abs.array().abs().matrix();

    float e = A.h().dot(n_abs);
    float s = A.c().dot(n) + d;
    if (s - e > 0.0f)
        return -1;
    if (s + e < 0.0f)
        return +1;
    return 0;
}
bool intersect_sphere_AABB(const vec3f& c, float r, const AABB& A) {
    vec3f e = (A.min() - c).array().max(0.0f).matrix() +
              (c - A.max()).array().max(0.0f).matrix();
    float d = e.dot(e);
    if (d > r * r)
        return false;
    else
        return true;
}
bool intersect_AABB(const AABB& A, const AABB& B) {
    vec3f amin = A.min();
    vec3f amax = A.max();
    vec3f bmin = B.min();
    vec3f bmax = B.max();
    if (amin.x() > bmax.x() || bmin.x() > amax.x())
        return false;
    if (amin.y() > bmax.y() || bmin.y() > amax.y())
        return false;
    if (amin.z() > bmax.z() || bmin.z() > amax.z())
        return false;
    return true;
}
void spawn_frustum_plane(std::array<Plane,6>& planes, const mat4& vp_mat) {
    planes[left] = {{vp_mat(0, 3) + vp_mat(0, 0), vp_mat(1, 3) + vp_mat(1, 0),
                     vp_mat(2, 3) + vp_mat(2, 0)},
                    vp_mat(3, 3) + vp_mat(3, 0)};
    planes[right] = {{vp_mat(0, 3) - vp_mat(0, 0), vp_mat(1, 3) - vp_mat(1, 0),
                      vp_mat(2, 3) - vp_mat(2, 0)},
                     vp_mat(3, 3) - vp_mat(3, 0)};
    planes[top] = {{vp_mat(0, 3) - vp_mat(0, 1), vp_mat(1, 3) - vp_mat(1, 1),
                    vp_mat(2, 3) - vp_mat(2, 1)},
                   vp_mat(3, 3) - vp_mat(3, 1)};
    planes[bottom] = {{vp_mat(0, 3) + vp_mat(0, 1), vp_mat(1, 3) + vp_mat(1, 1),
                       vp_mat(2, 3) + vp_mat(2, 1)},
                      vp_mat(3, 3) + vp_mat(3, 1)};
    planes[near] = {{vp_mat(0, 2), vp_mat(1, 2), vp_mat(2, 2)}, vp_mat(3, 2)};
    planes[far] = {{vp_mat(0, 3) - vp_mat(0, 2), vp_mat(1, 3) - vp_mat(1, 2),
                    vp_mat(2, 3) - vp_mat(2, 2)},
                   vp_mat(3, 3) - vp_mat(3, 2)};
    for (int i = 0; i < 6; i++) {
        planes[i].norm();
    }
}
bool intersect_frustum_planes_AABB(const std::array<Plane, 6>& ps, const AABB& A) {
    // 法线朝向视锥体外部
    for (uint32_t i = 0; i < 6; i++) {
        if (intersect_plane_AABB(ps[i], A) <
            0) /*在面内或相交时大于等于0，存在一个不成立即不在视锥体内*/
            return false;
    }
    return true;
}
bool intersect_frustum_planes_OBB(const std::array<Plane, 6>& ps, const OBB& A) {
    // 法线朝向视锥体外部
    for (uint32_t i = 0; i < 6; i++) {
        if (intersect_plane_OBB(ps[i], A) <
            0) /*在面内或相交时大于等于0，存在一个不成立即不在视锥体内*/
            return false;
    }
    return true;
}
}  // namespace BL