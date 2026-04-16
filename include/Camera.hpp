#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
    glm::mat4 _projection;
    glm::mat4 _view;

    // Trackball params
    float     _yaw      = 90.0f;
    float     _pitch    = 45.0f;
    float     _distance = 12.0f;
    glm::vec3 _target   = glm::vec3(4.0f, 0.0f, 4.0f);
    
    // FPS params
    bool      _isFpsMode = false;
    glm::vec3 _fpsPos    = glm::vec3(4.0f, 1.0f, 4.0f);
    float     _savedYaw   = 90.0f;
    float     _savedPitch = 45.0f;

    glm::vec3 _camPos;

    // Raycast Interaction
    int _hoveredX = -1;
    int _hoveredY = -1;

public:
    Camera();
    ~Camera() = default;

    void init(int width, int height);
    void updateInputs();
    void updateViewMatrix();
    void updateRaycast(float mouseX, float mouseY, int screenWidth, int screenHeight);

    void setFpsMode(bool enabled, glm::vec3 pos = glm::vec3(0.0f));

    // Getters
    const glm::mat4& getViewMatrix() const { return _view; }
    const glm::mat4& getProjectionMatrix() const { return _projection; }
    const glm::vec3& getPosition() const { return _camPos; }
    int getHoveredX() const { return _hoveredX; }
    int getHoveredY() const { return _hoveredY; }
    bool isFpsMode() const { return _isFpsMode; }
};
