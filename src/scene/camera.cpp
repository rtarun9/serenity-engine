#include "serenity-engine/scene/camera.hpp"

using namespace math;

namespace serenity::scene
{
    void Camera::update(const float delta_time, const core::Input &input)
    {
        // Logic for making camera movement smooth.
        // pitch_to, yaw_to, camera_move_to_position are all depending on only the current frame, and values are set
        // based on user input at that current instance of time. The static variables (yaw_shift, pitch_shift, move_to)
        // etc are persistent across frames, hence made static.

        // Their values (of the persistent static variables) are linearly interpolated over frames. If no input given
        // (for the specific movement), the values are slowly interpolated to
        // the current frame value (i.e the movements are not instantaneous). By this logic, the values will be
        // interpolated to 0 when no input is given. Hence, this case (of no input) need not be separately handled.
        // In short, we want our camera position and euler angle orientation values to lerp between frames.

        // The friction "smooth" or how slowly / fast
        //  Reference (WickedEngine) :
        //  https://github.com/turanszkij/WickedEngine/commit/42d7592444ff74180f8b48d14a05e947b37fd387.

        const auto movement_speed = m_movement_speed * delta_time;
        const auto rotation_speed = m_rotation_speed * delta_time;

        const auto camera_front = math::XMLoadFloat4(&m_camera_front);
        const auto camera_right = math::XMLoadFloat4(&m_camera_right);

        auto camera_position = math::XMLoadFloat4(&m_camera_position);

        auto camera_move_to_direction = math::XMVECTOR{0.0f, 0.0f, 0.0f, 0.0f};

        auto pitch_to = 0.0f;
        auto yaw_to = 0.0f;

        // Static variables to ensure that the move_to / yaw_shift / pitch_shift persist across frames.
        // Is there is no user input, the values slowly lerp towards thier defaults (i.e 0).
        static auto move_to = math::XMVECTOR{0.0f, 0.0f, 0.0f, 0.0f};
        static auto yaw_shift = 0.0f;
        static auto pitch_shift = 0.0f;

        // Handle input.
        const auto &keyboard = input.keyboard;

        if (keyboard.is_key_pressed(core::Keys::W))
        {
            camera_move_to_direction += camera_front * movement_speed;
        }
        else if (keyboard.is_key_pressed(core::Keys::S))
        {
            camera_move_to_direction -= camera_front * movement_speed;
        }

        if (keyboard.is_key_pressed(core::Keys::A))
        {
            camera_move_to_direction -= camera_right * movement_speed;
        }
        else if (keyboard.is_key_pressed(core::Keys::D))
        {
            camera_move_to_direction += camera_right * movement_speed;
        }

        if (keyboard.is_key_pressed(core::Keys::ArrowUp))
        {
            pitch_to -= rotation_speed;
        }
        else if (keyboard.is_key_pressed(core::Keys::ArrowDown))
        {
            pitch_to += rotation_speed;
        }

        if (keyboard.is_key_pressed(core::Keys::ArrowLeft))
        {
            yaw_to -= rotation_speed;
        }
        else if (keyboard.is_key_pressed(core::Keys::ArrowRight))
        {
            yaw_to += rotation_speed;
        }

        camera_move_to_direction = math::XMVector3Normalize(camera_move_to_direction) * movement_speed;
        move_to = math::XMVectorLerp(move_to, camera_move_to_direction, m_friction_factor);

        camera_position += move_to;

        pitch_shift = std::lerp(pitch_shift, pitch_to, m_friction_factor);
        yaw_shift = std::lerp(yaw_shift, yaw_to, m_friction_factor);

        m_pitch += pitch_shift;
        m_yaw += yaw_shift;

        math::XMStoreFloat4(&m_camera_position, camera_position);
    }

    math::XMMATRIX Camera::get_view_matrix()
    {
        // Load all XMFLOATX into XMVECTOR's.
        // The target is camera position + camera front direction (i.e direction it is looking at).

        const auto rotation_matrix = math::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

        static constexpr auto world_up = math::XMVECTOR{0.0f, 1.0f, 0.0f, 0.0f};
        static constexpr auto world_right = math::XMVECTOR{1.0f, 0.0f, 0.0f, 0.0f};
        static constexpr auto world_front = math::XMVECTOR{0.0f, 0.0f, 1.0f, 0.0f};

        const auto camera_right = math::XMVector3Normalize(math::XMVector3TransformCoord(world_right, rotation_matrix));

        const auto camera_front = math::XMVector3Normalize(math::XMVector3TransformCoord(world_front, rotation_matrix));

        const auto camera_up = math::XMVector3Normalize(math::XMVector3Cross(camera_front, camera_right));

        const auto camera_position = math::XMLoadFloat4(&m_camera_position);

        const auto camera_target = camera_position + camera_front;

        math::XMStoreFloat4(&m_camera_right, camera_right);
        math::XMStoreFloat4(&m_camera_front, camera_front);
        math::XMStoreFloat4(&m_camera_up, camera_up);

        return math::XMMatrixLookAtLH(camera_position, camera_target, camera_up);
    }
} // namespace serenity::scene
