#ifndef _BOUNDLESS_CAMERA_CXX_FILE_
#define _BOUNDLESS_CAMERA_CXX_FILE_
#include "transform.hpp"
namespace BL {
enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };
struct Camera_debug {
    CameraTransform transform;
    vec3f worldUp;
    vec3f worldForward;
    vec3f worldRight;
    float yaw;
    float pitch;
    float moveSpeed;
    float mouseSensitivity;

    vec3f& position() { return transform.position; }
    vec3f& forward() { return transform.forward; }
    vec3f& up() { return transform.up; }
    vec3f& right() { return transform.right; }
    float& fov() { return transform.fov; }
    void process_keyboard(CameraMovement direction, float deltaTime) {
        transform.isViewEdited = true;
        float velocity = moveSpeed * deltaTime;
        transform.normalize_vecs();
        if (direction == FORWARD)
            this->position() += this->forward() * velocity;
        if (direction == BACKWARD)
            this->position() -= this->forward() * velocity;
        if (direction == LEFT)
            this->position() -= this->right() * velocity;
        if (direction == RIGHT)
            this->position() += this->right() * velocity;
    }
    void process_mouse_movement(float xoffset, float yoffset) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;
        yaw += xoffset;
        pitch += yoffset;
        pitch = std::clamp(pitch, -89.8f, 89.8f);

        transform.isViewEdited = true;
        vec3f front = std::cos(radians(yaw)) * std::cos(radians(pitch)) * worldForward;
        front += std::sin(radians(yaw)) * std::cos(radians(pitch)) * worldRight;
        front += std::sin(radians(pitch)) * worldUp;
        front.normalize();
        this->forward() = front;
        this->right() = front.cross(worldUp);
        this->up() = this->right().cross(front);
    }
    void process_mouse_scroll(float yoffset) {
        transform.isProjEdited = true;
        this->fov() -= yoffset * 0.2;
        this->fov() = std::clamp(this->fov(), 0.0f, float(MATH_PI / 2.0f));
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