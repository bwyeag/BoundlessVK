#ifndef _BOUNDLESS_COLLISION_CXX_HPP_
#define _BOUNDLESS_COLLISION_CXX_HPP_
#include <concepts>
#include "bl_log.hpp"
#include "bl_math_types.hpp"
namespace BL::Math {
template <std::floating_point Real>
struct AABB {
    using Vec3 = vec3<Real>;
    Vec3 _max, _min;
    Vec3 c() const { return (_max + _min) / static_cast<Real>(2.0); }
    Vec3 u() const { return Vec3::UnitX(); }
    Vec3 v() const { return Vec3::UnitY(); }
    Vec3 w() const { return Vec3::UnitZ(); }
    Vec3 h() const { return (_max - _min) / static_cast<Real>(2.0); }
    float h_u() const { return (_max.x() - _min.x()) / static_cast<Real>(2.0); }
    float h_v() const { return (_max.y() - _min.y()) / static_cast<Real>(2.0); }
    float h_w() const { return (_max.z() - _min.z()) / static_cast<Real>(2.0); }
    Vec3& max() const { return _max; }
    Vec3& min() const { return _min; }
};
template <std::floating_point Real>
struct OBB {
    using Vec3 = vec3<Real>;
    Vec3 _c, _u, _v, _h;
    Vec3 c() const { return _c; }
    Vec3 u() const { return _u; }
    Vec3 v() const { return _v; }
    Vec3 w() const { return _u.cross(_v); }
    Vec3 h() const { return _h; }
    float h_u() const { return _h.x(); }
    float h_v() const { return _h.y(); }
    float h_w() const { return _h.z(); }
    void setCenter(const Vec3& new_c) { _c = new_c; }
    void setBoxSize(const Vec3& x, const Vec3& y, float h_z) {
        _h.x() = x.norm();
        _u = x / _h.x();
        _h.y() = y.norm();
        _v = y / _h.y();
        _h.z() = h_z;
    }
};
template <std::floating_point Real>
struct Ray {
    using Vec3 = vec3<Real>;
    Vec3 _o, _d;
    Vec3& o() const { return _o; }
    Vec3& d() const { return _d; }
};
template <std::floating_point Real>
struct Plane {
    using Vec3 = vec3<Real>;
    using Vec4 = vec4<Real>;
    Vec4 data;
    Vec3 n() const { return data.block<3, 1>(0, 0); }
    Real d() const { return data[3]; }
    // set自动归一化向量
    void set(const Vec3& nn, Real nd) {
        Real rec_len = 1 / nn.norm();
        data.block<3, 1>(0, 0) = nn;
        data[3] = nd;
        data *= rec_len;
    }
    void norm() {
        Real rec_len = 1 / data.block<3, 1>(0, 0).norm();
        data *= rec_len;
    }
    void set_nonorm(const Vec3& nn, Real nd) {
        data.block<3, 1>(0, 0) = nn;
        data[3] = nd;
    }
};
template <std::floating_point Real>
vec2<Real> projOBB(const OBB<Real>& b, vec3<Real>& d) {
    Real h = b.c().dot(d);
    vec3<Real> k{b.u().dot(d) * b.h_u(), b.v().dot(d) * b.h_v(),
                 b.w().dot(d) * b.h_w()};
    Real s = k.array().abs().sum();
    return vec2<Real>(h - s, h + s);
}
template <std::floating_point Real>
vec2<Real> projOBB2(const OBB<Real>& b,
                    const vec3<Real>& d,
                    const vec3<Real>& bw) {
    Real h = b.c().dot(d);
    vec3<Real> k{b.u().dot(d) * b.h_u(), b.v().dot(d) * b.h_v(),
                 bw.dot(d) * b.h_w()};
    Real s = k.array().abs().sum();
    return vec2<Real>(h - s, h + s);
}
template <std::floating_point Real>
vec2<Real> projAABB(const AABB<Real>& b, vec3<Real> d) {
    Real h = b.c().dot(d);
    Real s = (d.array() * b.h().array()).abs().sum();
    return vec2<Real>(h - s, h + s);
}
template <std::floating_point Real>
bool testSAT(vec2<Real> a, vec2<Real> b) {
    return (a.x() < b.y() && b.x() < a.y())
}
template <std::floating_point Real>
bool testSAT_inner(vec2<Real> a, vec2<Real> b) {
    return (a.x() < b.x() && b.y() < a.y())
}
template <std::floating_point Real>
bool testSAT_inner2(vec2<Real> a, vec2<Real> b) {
    return (a.x() < b.x() && b.y() < a.y() || b.x() < a.x() && a.y() < b.y())
}

enum struct CollisionResult { outer = 0x0, intersect = 0x1, inner = 0x2 };
template <typename T>
concept Collisions = std::same_as<T, AABB> || std::same_as<T, OBB>;

template <std::floating_point Real>
CollisionResult intersectTest(const AABB<Real>& A, const AABB<Real>& B) {
    if ((A.min().array() > B.max().array()).any() ||
        (B.min().array() > A.max().array()).any())
        return CollisionResult::outer;
    else if ((A.min() < B.min()).all() && (A.max() > B.max()).all())
        return CollisionResult::inner;
    return CollisionResult::intersect;
}

template <std::floating_point Real>
CollisionResult intersectTest(const OBB<Real>& A, const OBB<Real>& B) {
    int c = 0;
    vec3<Real> d1 = A.w();
    vec3<Real> d2 = B.w();
    vec2<Real> s1 = projOBB2(A, A.u(), d1), s2 = projOBB2(B, A.u(), d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    s1 = projOBB2(A, A.v(), d1), s2 = projOBB2(B, A.v(), d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    s1 = projOBB2(A, d1, d1), s2 = projOBB2(B, d1, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    s1 = projOBB2(A, B.u(), d1), s2 = projOBB2(B, B.u(), d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    s1 = projOBB2(A, B.v(), d1), s2 = projOBB2(B, B.v(), d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    s1 = projOBB2(A, d2, d1), s2 = projOBB2(B, d2, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    vec3<Real> d = A.u().cross(B.v());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    d = A.u().cross(d2);
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    d = A.v().cross(B.u());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    d = A.v().cross(d2);
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    d = d1.cross(B.u());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    d = d1.cross(B.v());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    d = B.u().cross(A.v());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    d = B.u().cross(d1);
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    d = B.v().cross(A.u());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    d = B.v().cross(d1);
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;

    d = d2.cross(A.u());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    d = d2.cross(A.v());
    s1 = projOBB2(A, d, d1), s2 = projOBB2(B, d, d2);
    if (!testSAT(s1, s2))
        return CollisionResult::outer;
    else if (testSAT_inner(s1, s2))
        c++;
    if (c == 12)
        return CollisionResult::inner;
    return CollisionResult::intersect;
}

}  // namespace BL::Math
#endif  //!_BOUNDLESS_COLLISION_CXX_HPP_