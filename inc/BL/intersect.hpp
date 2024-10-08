#ifndef _BOUNDLESS_INTERSECT_CXX_FILE_
#define _BOUNDLESS_INTERSECT_CXX_FILE_
#include <limits>
#include "bl_math_types.hpp"
namespace BL {
const float ϵ = 1e-20;
bool intersect_ray_sphere(const vec3f& o,
                          const vec3f& d,
                          const vec3f& c,
                          float r);
bool intersect_ray_sphere_res(const vec3f& o,
                              const vec3f& d,
                              const vec3f& c,
                              float r,
                              float* t);
class OBB {
    vec3f _c, _u, _v, _h;

   public:
    vec3f c() const { return _c; }
    vec3f u() const { return _u; }
    vec3f v() const { return _v; }
    vec3f w() const { return _u.cross(_v); }
    vec3f h() const { return _h; }
    float h_u() const { return _h.x(); }
    float h_v() const { return _h.y(); }
    float h_w() const { return _h.z(); }

    void set_c(const vec3f& new_c) { _c = new_c; }
    void set_size(const vec3f& x, const vec3f& y, float h_z) {
        _h.x() = x.norm();
        _u = x / _h.x();
        _h.y() = y.norm();
        _v = y / _h.y();
        _h.z() = h_z;
    }
};
bool intersect_ray_OBB_res(const vec3f& o,
                           const vec3f& d,
                           const OBB& A,
                           float* t);
inline bool intersect_ray_OBB(const vec3f& o, const vec3f& d, const OBB& A) {
    float t;
    return intersect_ray_OBB_res(o, d, A, &t);
}
class AABB {
    vec3f _c, _s;

   public:
    vec3f c() const { return _c; }
    vec3f u() const { return vec3f::UnitX(); }
    vec3f v() const { return vec3f::UnitY(); }
    vec3f w() const { return vec3f::UnitZ(); }
    vec3f h() const { return _s; }
    float h_u() const { return _s.x(); }
    float h_v() const { return _s.y(); }
    float h_w() const { return _s.z(); }
    vec3f max() const { return _c + _s; }
    vec3f min() const { return _c - _s; }

    void set_c(const vec3f& new_c) { _c = new_c; }
    void set_size(const vec3f& new_s) { _s = new_s; }
};
bool intersect_ray_AABB_res(const vec3f& o,
                            const vec3f& d,
                            const AABB& A,
                            float* t);
inline bool intersect_ray_AABB(const vec3f& o, const vec3f& d, const AABB& A) {
    float t;
    return intersect_ray_AABB_res(o, d, A, &t);
}
// 结果：0相交，+1在面内，-1在面外，n为单位向量，以法线方向为面外
int intersect_plane_AABB(const vec3f& n, float d, const AABB& A);
int intersect_plane_OBB(const vec3f& n, float d, const OBB& A);

bool intersect_sphere_AABB(const vec3f& c, float r, const AABB& A);
bool intersect_AABB(const AABB& A, const AABB& B);
struct Plane {
    vec3f n;
    float d;

    // 自动归一化向量，无需提前进行
    void set(const vec3f& nn, float nd) {
        float len = nn.norm();
        n = nn / len;
        d = nd / len;
    }
    void norm() {
        float len = n.norm();
        n = n / len;
        d = d / len;
    }
    void set_nonorm(const vec3f& nn, float nd) {
        n = nn;
        d = nd;
    }
};
enum FrustumPlaneIndex { left = 0, right, top, bottom, near, far };
void spawn_frustum_plane(std::array<Plane, 6>& planes, const mat4f& vp_mat);
inline int intersect_plane_AABB(const Plane& p, const AABB& A) {
    return intersect_plane_AABB(p.n, p.d, A);
}
inline int intersect_plane_OBB(const Plane& p, const OBB& A) {
    return intersect_plane_OBB(p.n, p.d, A);
}
bool intersect_frustum_planes_AABB(const std::array<Plane, 6>& ps,
                                   const AABB& A);
bool intersect_frustum_planes_OBB(const std::array<Plane, 6>& ps, const OBB& A);
}  // namespace BL
#endif  //!_BOUNDLESS_INTERSECT_CXX_FILE_