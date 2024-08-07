#include "transform.hpp"
// Result\[(.)\]\[(.)\]  res($2,$1)
// \bT\b
namespace BL {
mat4f frustum(float left,
              float right,
              float bottom,
              float top,
              float zNear,
              float zFar) {
    mat4f res = mat4f::Identity();
    res(0, 0) = (static_cast<float>(2) * zNear) / (right - left);
    res(1, 1) = (static_cast<float>(2) * zNear) / (top - bottom);
    res(0, 2) = (right + left) / (right - left);
    res(1, 2) = (top + bottom) / (top - bottom);
    res(2, 2) = zFar / (zNear - zFar);
    res(3, 2) = static_cast<float>(-1);
    res(2, 3) = -(zFar * zNear) / (zFar - zNear);
    return res;
}
mat4f ortho(float left,
            float right,
            float bottom,
            float top,
            float zNear,
            float zFar) {
    mat4f res = mat4f::Identity();
    res(0, 0) = static_cast<float>(2) / (right - left);
    res(1, 1) = static_cast<float>(2) / (top - bottom);
    res(2, 2) = -static_cast<float>(1) / (zFar - zNear);
    res(0, 3) = -(right + left) / (right - left);
    res(1, 3) = -(top + bottom) / (top - bottom);
    res(2, 3) = -zNear / (zFar - zNear);
    return res;
}
mat4f perspective(float fovy, float aspect, float zNear, float zFar) {
    float tanHalfFovy = tan(fovy / static_cast<float>(2));

    mat4f res = mat4f::Zero();
    res(0, 0) = static_cast<float>(1) / (aspect * tanHalfFovy);
    res(1, 1) = static_cast<float>(1) / (tanHalfFovy);
    res(2, 2) = zFar / (zNear - zFar);
    res(3, 2) = static_cast<float>(-1);
    res(2, 3) = -(zFar * zNear) / (zFar - zNear);
    return res;
}
mat4f perspective(float fov,
                  float width,
                  float height,
                  float zNear,
                  float zFar) {
    float rad = fov;
    float h =
        cos(static_cast<float>(0.5) * rad) / sin(static_cast<float>(0.5) * rad);
    float w = h * height / width;

    mat4f res = mat4f::Zero();
    res(0, 0) = w;
    res(1, 1) = h;
    res(2, 2) = zFar / (zNear - zFar);
    res(3, 2) = static_cast<float>(-1);
    res(2, 3) = -(zFar * zNear) / (zFar - zNear);
    return res;
}
mat4f infinite_perspective(float fov, float aspect, float zNear) {
    float range = tan(fov / static_cast<float>(2)) * zNear;
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;

    mat4f res = mat4f::Zero();
    res(0, 0) = (static_cast<float>(2) * zNear) / (right - left);
    res(1, 1) = (static_cast<float>(2) * zNear) / (top - bottom);
    res(2, 2) = static_cast<float>(-1);
    res(3, 2) = static_cast<float>(-1);
    res(2, 3) = static_cast<float>(-2) * zNear;
    return res;
}
static mat4f _look_matrix(const vec3f& eye,
                          const vec3f& f,
                          const vec3f& s,
                          const vec3f& u) {
    mat4f res = mat4f::Identity();
    res(0, 0) = s.x();
    res(0, 1) = s.y();
    res(0, 2) = s.z();
    res(1, 0) = u.x();
    res(1, 1) = u.y();
    res(1, 2) = u.z();
    res(2, 0) = -f.x();
    res(2, 1) = -f.y();
    res(2, 2) = -f.z();
    res(0, 3) = -s.dot(eye);
    res(1, 3) = -u.dot(eye);
    res(2, 3) = f.dot(eye);
    return res;
}
mat4f look_at(const vec3f& eye, const vec3f& center, const vec3f& up) {
    vec3f f = (center - eye).normalized();
    vec3f s = (f.cross(up)).normalized();
    vec3f u = s.cross(f);

    return look_forward(eye, center-eye,up);
}
mat4f look_forward(const vec3f& eye, const vec3f& forward, const vec3f& up) {
    vec3f f = forward;
    vec3f s = f.cross(up);
    vec3f u = s.cross(f);

    return _look_matrix(eye,f,s,u);
}
quatf rotate(const vec3f eulerAngle) /*欧拉角转四元数*/ {
    Eigen::AngleAxisf rollAngle(Eigen::AngleAxisf(eulerAngle.x(), vec3f::UnitX()));
    Eigen::AngleAxisf pitchAngle(Eigen::AngleAxisf(eulerAngle.y(), vec3f::UnitY()));
    Eigen::AngleAxisf yawAngle(Eigen::AngleAxisf(eulerAngle.z(), vec3f::UnitZ()));

    quatf quaternion;
    quaternion = yawAngle * pitchAngle * rollAngle;
    return quaternion;
}
}  // namespace BL