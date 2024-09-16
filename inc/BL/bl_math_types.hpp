#ifndef _BOUNDLESS_MATH_TYPES_CXX_FILE_
#define _BOUNDLESS_MATH_TYPES_CXX_FILE_
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <concepts>
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
template<typename T>
using vec2 = Eigen::Matrix<T, 2, 1>;
template<typename T>
using vec3 = Eigen::Matrix<T, 3, 1>;
template<typename T>
using vec4 = Eigen::Matrix<T, 4, 1>;
template<typename T>
using mat2 = Eigen::Matrix<T, 2, 2>;
template<typename T>
using mat3 = Eigen::Matrix<T, 3, 3>;
template<typename T>
using mat4 = Eigen::Matrix<T, 4, 4>;
} // namespace BL
#endif //!_BOUNDLESS_MATH_TYPES_CXX_FILE_