#include "Camera.hpp"
#include <imgui.h>
#include <cmath>

Camera::Camera() : _projection(1.0f), _view(1.0f), _camPos(0.0f) {
}

void Camera::init(int width, int height) {
    updateViewMatrix();
    _projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
}

void Camera::setFpsMode(bool enabled, glm::vec3 pos) { 
    if (enabled && !_isFpsMode) {
        _savedYaw = _yaw;
        _savedPitch = _pitch;
        
        _pitch = 0.0f;
        _yaw = (pos.z > 4.0f) ? -90.0f : 90.0f;
    } 
    else if (!enabled && _isFpsMode) {
        _yaw = _savedYaw;
        _pitch = _savedPitch;
    }

    _isFpsMode = enabled;
    if (enabled) {
        _fpsPos = pos + glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    updateViewMatrix();
}

void Camera::updateViewMatrix() {
    float yawRad   = glm::radians(_yaw);
    float pitchRad = glm::radians(_pitch);

    if (!_isFpsMode) {
        _camPos.x = _target.x + _distance * cos(pitchRad) * cos(yawRad);
        _camPos.y = _target.y + _distance * sin(pitchRad);
        _camPos.z = _target.z + _distance * cos(pitchRad) * sin(yawRad);
        
        _view = glm::lookAt(_camPos, _target, glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
        _camPos = _fpsPos + glm::vec3(0.0f, 0.4f, 0.0f);
        glm::vec3 front;
        front.x = cos(yawRad) * cos(pitchRad);
        front.y = sin(pitchRad);
        front.z = sin(yawRad) * cos(pitchRad);
        front = glm::normalize(front);
        
        _view = glm::lookAt(_camPos, _camPos + front, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void Camera::updateInputs() {
    ImGuiIO& io = ImGui::GetIO();
    float sensitivity = 0.5f;

    if (!_isFpsMode) {
        _yaw += io.MouseDelta.x * sensitivity;
        _pitch -= io.MouseDelta.y * sensitivity;

        if (_pitch > 89.0f) _pitch = 89.0f;
        if (_pitch < 1.0f) _pitch = 1.0f;
    } else {
        _yaw += io.MouseDelta.x * sensitivity;
        _pitch -= io.MouseDelta.y * sensitivity;

        if (_pitch > 89.0f) _pitch = 89.0f;
        if (_pitch < -89.0f) _pitch = -89.0f;
    }

    updateViewMatrix();
}

void Camera::updateRaycast(float mouseX, float mouseY, int screenWidth, int screenHeight) {
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    glm::mat4 invProj = glm::inverse(_projection);
    glm::vec4 ray_eye = invProj * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    glm::mat4 invView = glm::inverse(_view);
    glm::vec3 ray_wor = glm::vec3(invView * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    _hoveredX = -1;
    _hoveredY = -1;

    if (abs(ray_wor.y) > 0.001f) {
        float t = -_camPos.y / ray_wor.y;
        if (t >= 0.0f) {
            glm::vec3 intersection = _camPos + t * ray_wor;
            
            if (intersection.x >= 0.0f && intersection.x < 8.0f &&
                intersection.z >= 0.0f && intersection.z < 8.0f) {
                _hoveredY = (int)floor(intersection.x);
                _hoveredX = (int)floor(intersection.z);
            }
        }
    }
}
