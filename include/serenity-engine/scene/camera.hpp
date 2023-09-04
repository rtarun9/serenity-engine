#pragma once

#include "serenity-engine/core/input.hpp"

namespace serenity::scene
{
    // Provides functions to compute the view matrix that can be used to transform world space coordinates into view
    // space coordinates.
    class Camera
    {
      public:
        // Update the camera position and orientation values (i.e the Euler angles pitch and yaw).
        void update(const float delta_time, const core::Input &input);

        math::XMMATRIX get_view_matrix();

      public:
        // Camera state.
        math::XMFLOAT4 m_camera_position{0.0f, 0.0f, -5.0f, 1.0f};

        math::XMFLOAT4 m_camera_front{0.0f, 0.0f, 1.0f, 0.0f};
        math::XMFLOAT4 m_camera_right{1.0f, 0.0f, 0.0f, 0.0f};
        math::XMFLOAT4 m_camera_up{0.0f, 1.0f, 0.0f, 0.0f};

        // Euler angle for x axis.
        float m_pitch{};

        // Euler angle for y axis.
        float m_yaw{};

        float m_movement_speed{0.1f};
        float m_rotation_speed{0.0015f};

        // Used to control how fast or slow to lerp to rest position.
        float m_friction_factor{0.12f};
    };
} // namespace serenity::scene