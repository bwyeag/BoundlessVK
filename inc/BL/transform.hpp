#ifndef _BOUNDLESS_TRANSFORM_CXX_FILE_
#define _BOUNDLESS_TRANSFORM_CXX_FILE_
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include "init.hpp"
/*
 * 使用右手坐标系，NDC：x:[-1,+1],y:[-1,+1],z:[0,+1]
 */
namespace BL {
using vec2i = Eigen::Vector2i;
using vec3i = Eigen::Vector3i;
using vec4i = Eigen::Vector4i;
using vec2f = Eigen::Vector2f;
using vec3f = Eigen::Vector3f;
using vec4f = Eigen::Vector4f;
using vec2d = Eigen::Vector2d;
using vec3d = Eigen::Vector3d;
using vec4d = Eigen::Vector4d;
using mat2f = Eigen::Matrix2f;
using mat3f = Eigen::Matrix3f;
using mat4f = Eigen::Matrix4f;
using mat2d = Eigen::Matrix2d;
using mat3d = Eigen::Matrix3d;
using mat4d = Eigen::Matrix4d;
using mat2x3f = Eigen::Matrix<float,2,3>;
using mat2x4f = Eigen::Matrix<float,2,4>;
using mat3x2f = Eigen::Matrix<float,3,2>;
using mat3x4f = Eigen::Matrix<float,3,4>;
using mat4x2f = Eigen::Matrix<float,4,2>;
using mat4x3f = Eigen::Matrix<float,4,3>;
using mat2x3d = Eigen::Matrix<double,2,3>;
using mat2x4d = Eigen::Matrix<double,2,4>;
using mat3x2d = Eigen::Matrix<double,3,2>;
using mat3x4d = Eigen::Matrix<double,3,4>;
using mat4x2d = Eigen::Matrix<double,4,2>;
using mat4x3d = Eigen::Matrix<double,4,3>;
using quatf = Eigen::Quaternionf;
using quatd = Eigen::Quaterniond;
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

    vec3f position;          // 位置
    mutable vec3f forward;  // 前向量（右手系）
    mutable vec3f up;  // 上向量

    float zNear, zFar, fov;
    mutable float aspect = 0.0f;

    bool isFrustum;
    mutable bool isViewEdited = true;
    mutable bool isProjEdited = true;

    const mat4f& get_view_matrix(bool& changed) const {
        if (isViewEdited) {
            forward.normalize();
            up.normalize();
            viewMatrix = look_forward(position, forward, up);
            isViewEdited = false;
            changed = true;
        }
        return viewMatrix;
    }
    const mat4f& get_proj_matrix(bool& changed) const {
        float width = context.vulkanInfo.swapchainCreateInfo.imageExtent.width;
        float height =
            context.vulkanInfo.swapchainCreateInfo.imageExtent.height;
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
            changed = true;
        }
        return projMatrix;
    }
};
}  // namespace BL
#endif  //!_BOUNDLESS_TRANSFORM_CXX_FILE_