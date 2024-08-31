#ifndef _BOUNDLESS_TRANSFORM_CXX_FILE_
#define _BOUNDLESS_TRANSFORM_CXX_FILE_
#include <cmath>
#include "bl_context.hpp"
#include "math_types.hpp"
/*
 * 使用右手坐标系，NDC：x:[-1,+1],y:[-1,+1],z:[0,+1]
 */
namespace BL {
constexpr double MATH_PI = 3.1415926535897932384626;
template <std::floating_point T>
T radians(T angle) {
    return angle * static_cast<T>((2.0 * MATH_PI) / 360.0);
}
// 此部分代码由GLM库改写而来
mat4f frustum(float left,
              float right,
              float bottom,
              float top,
              float zNear,
              float zFar);
mat4f ortho(float left,
            float right,
            float bottom,
            float top,
            float zNear,
            float zFar);
mat4f perspective(float fov, float aspect, float zNear, float zFar);
mat4f perspective(float fov,
                  float width,
                  float height,
                  float zNear,
                  float zFar);
mat4f infinite_perspective(float fov, float aspect, float zNear);
mat4f look_matrix(const vec3f& eye,
                  const vec3f& f,
                  const vec3f& s,
                  const vec3f& u);
mat4f look_at(const vec3f& eye, const vec3f& center, const vec3f& up);
mat4f look_forward(const vec3f& eye, const vec3f& forward, const vec3f& up);
quatf rotate(const vec3f eulerAngle) /*欧拉角转四元数*/;
struct ModelTransform {
    mutable mat4f modelMatrix;
    vec3f position;
    vec3f scale;
    quatf rotate;
    mutable bool isEdited = true;

    const mat4f& get_matrix() const {
        if (isEdited) {
            modelMatrix = mat4f::Identity();
            modelMatrix(0, 0) = scale.x();
            modelMatrix(1, 1) = scale.y();
            modelMatrix(2, 2) = scale.z();
            modelMatrix.block<3, 3>(0, 0) =
                rotate.toRotationMatrix() * modelMatrix.block<3, 3>(0, 0);
            modelMatrix(0, 0) += modelMatrix(3, 0) * position.x();
            modelMatrix(0, 1) += modelMatrix(3, 1) * position.x();
            modelMatrix(0, 2) += modelMatrix(3, 2) * position.x();
            modelMatrix(0, 3) += modelMatrix(3, 3) * position.x();
            modelMatrix(1, 0) += modelMatrix(3, 0) * position.y();
            modelMatrix(1, 1) += modelMatrix(3, 1) * position.y();
            modelMatrix(1, 2) += modelMatrix(3, 2) * position.y();
            modelMatrix(1, 3) += modelMatrix(3, 3) * position.y();
            modelMatrix(2, 0) += modelMatrix(3, 0) * position.z();
            modelMatrix(2, 1) += modelMatrix(3, 1) * position.z();
            modelMatrix(2, 2) += modelMatrix(3, 2) * position.z();
            modelMatrix(2, 3) += modelMatrix(3, 3) * position.z();
            isEdited = false;
        }
        return modelMatrix;
    }
};
struct CameraTransform {
    mutable mat4f viewMatrix;
    mutable mat4f projMatrix;

    vec3f position;         // 位置
    mutable vec3f forward;  // 前向量（右手系）= z
    mutable vec3f up;       // 上向量 = y
    mutable vec3f right;    // 右向量 = x

    float zNear, zFar, fov;
    mutable float aspect = 0.0f;

    bool isFrustum;
    mutable bool isViewEdited = true;
    mutable bool isProjEdited = true;
    void normalize_vecs() const {
        if (isViewEdited) {
            forward.normalize();
            right = forward.cross(up).normalized();
            up = right.cross(forward);
        }
    }
    const mat4f& get_view_matrix(bool& changed) const {
        if (isViewEdited) {
            this->normalize_vecs();
            viewMatrix = look_matrix(position, forward, right, up);
            isViewEdited = false;
            changed |= true;
        } else {
            changed |= false;
        }
        return viewMatrix;
    }
    const mat4f& get_proj_matrix(bool& changed) const {
        float width = CurContext().vulkanInfo.swapchainCreateInfo.imageExtent.width;
        float height =
            CurContext().vulkanInfo.swapchainCreateInfo.imageExtent.height;
        float aspect_new = width / height;
        if (isProjEdited || aspect != aspect_new) {
            aspect = aspect_new;
            if (isFrustum) {
                projMatrix = perspective(fov, aspect, zNear, zFar);
            } else {
                projMatrix = ortho(-width / 2, width / 2, -height / 2,
                                   height / 2, zNear, zFar);
            }
            isProjEdited = false;
            changed |= true;
        } else {
            changed |= false;
        }
        return projMatrix;
    }
};
}  // namespace BL
#endif  //!_BOUNDLESS_TRANSFORM_CXX_FILE_