#ifndef _BOUNDLESS_CAMERA_CXX_FILE_
#define _BOUNDLESS_CAMERA_CXX_FILE_
#include "transform.hpp"
namespace BL {
enum CameraMovement {
    NONE_MOVEMENT = 0x0,
    FORWARD = 0x1,
    BACKWARD = 0x2,
    LEFT = 0x4,
    RIGHT = 0x8,
    ROTATE_LEFT = 0x10,
    ROTATE_RIGHT = 0x20,
    FORWARD_UP = 0x40,
    FORWARD_DOWN = 0x80,
    FORWARD_LEFT = 0x100,
    FORWARD_RIGHT = 0x200,
    LOCK = 0x400,
    ZOOM_UP = 0x800,
    ZOOM_DOWN = 0x1000,
    MOVEMENT = 0x3FF,
    VIEW = 0x1800
};
enum CameraProjType { FRUSTUM = 1, ORTHO = 0 };
class Camera_debug {
    CameraTransform transform;
    float moveVelocity;
    float zoomMax, zoomMin;
    float mouseSensitivity;
    float keySensitivity;
    float zoomSensitivity;
    bool _isLocked = true; /*摄像机是否上锁，为true时无法移动等等*/
   public:
    vec3f& Position() { return transform.position; }
    vec3f& Forward() { return transform.forward; }
    vec3f& Up() { return transform.up; }
    vec3f& Right() { return transform.right; }
    float& NearDist() { return transform.zNear; }
    float& FarDist() { return transform.zFar; }
    float& Fov() { return transform.fov; }
    void set_zoom_range(float m, float M) { zoomMin = m, zoomMax = M; }
    void set_move_velocity(float v) { moveVelocity = v; }
    void set_mouse_sensitivity(float v) { mouseSensitivity = v; }
    void set_key_sensitivity(float v) { keySensitivity = v; }
    void set_zoom_sensitivity(float v) { zoomSensitivity = v; }
    void set_proj_type(CameraProjType t) { transform.isFrustum = bool(t); }
    bool isLocked() const { return _isLocked; }
    void process_keyboard(CameraMovement direction, float deltaTime) {
        if (direction & LOCK) {
            if (_isLocked) {
                _isLocked = false;
                glfwSetInputMode(CurContext().windowInfo.pWindow, GLFW_CURSOR,
                                 GLFW_CURSOR_DISABLED);
                print_log("Camera Debug", "Camera Unlocked!");
            } else {
                _isLocked = true;
                glfwSetInputMode(CurContext().windowInfo.pWindow, GLFW_CURSOR,
                                 GLFW_CURSOR_NORMAL);
                print_log("Camera Debug", "Camera Locked!");
            }
            return;
        }
        if (!_isLocked) {
            if (direction & MOVEMENT) {
                using angle_axisf = Eigen::AngleAxisf;
                transform.isViewEdited = true;
                float velocity = moveVelocity * deltaTime;
                transform.normalize_vecs();
                if (direction & FORWARD)
                    this->Position() += this->Forward() * velocity;
                if (direction & BACKWARD)
                    this->Position() -= this->Forward() * velocity;
                if (direction & LEFT)
                    this->Position() -= this->Right() * velocity;
                if (direction & RIGHT)
                    this->Position() += this->Right() * velocity;
                float delta_angle = keySensitivity * deltaTime;
                bool transFlag = false;
                quatf rotation = quatf::Identity();
                if (direction & ROTATE_LEFT) {
                    angle_axisf rot(delta_angle, Forward());
                    rotation = rot * rotation;
                    transFlag = true;
                }
                if (direction & ROTATE_RIGHT) {
                    angle_axisf rot(-delta_angle, Forward());
                    rotation = rot * rotation;
                    transFlag = true;
                }
                if (direction & FORWARD_UP) {
                    angle_axisf rot(delta_angle, Right());
                    rotation = rot * rotation;
                    transFlag = true;
                }
                if (direction & FORWARD_DOWN) {
                    angle_axisf rot(-delta_angle, Right());
                    rotation = rot * rotation;
                    transFlag = true;
                }
                if (direction & FORWARD_LEFT) {
                    angle_axisf rot(delta_angle, Up());
                    rotation = rot * rotation;
                    transFlag = true;
                }
                if (direction & FORWARD_RIGHT) {
                    angle_axisf rot(-delta_angle, Up());
                    rotation = rot * rotation;
                    transFlag = true;
                }
                if (transFlag) {
                    Forward() = rotation * Forward();
                    Up() = rotation * Up();
                    Right() = Forward().cross(Up());
                }
            }
            if (direction & VIEW) {
                transform.isProjEdited = true;
                if (direction & ZOOM_UP)
                    this->Fov() -= keySensitivity * deltaTime;
                if (direction & ZOOM_DOWN)
                    this->Fov() += keySensitivity * deltaTime;
                this->Fov() = std::clamp(this->Fov(), zoomMin, zoomMax);
            }
        }
    }
    void process_mouse_movement(float xoffset, float yoffset) {
        if (!_isLocked) {
            using angle_axisf = Eigen::AngleAxisf;
            transform.isViewEdited = true;
            angle_axisf rotYaw(-xoffset * mouseSensitivity, Up());
            angle_axisf rotPitch(yoffset * mouseSensitivity, Right());
            quatf rotation =  rotPitch*rotYaw;
            Forward() = rotation * Forward();
            Up() = rotation * Up();
            Right() = Forward().cross(Up());
        }
    }
    void process_mouse_scroll(float yoffset) {
        if (!_isLocked) {
            transform.isProjEdited = true;
            this->Fov() -= zoomSensitivity * yoffset;
            this->Fov() = std::clamp(this->Fov(), zoomMin, zoomMax);
        }
    }
    const mat4f& get_view_matrix(bool& changed) {
        return transform.get_view_matrix(changed);
    }
    const mat4f& get_proj_matrix(bool& changed) {
        return transform.get_proj_matrix(changed);
    }
};
}  // namespace BL
#endif  // ! _BOUNDLESS_CAMERA_CXX_FILE_