pub struct Camera {
    pub eye_position: cgmath::Point3<f32>,
    pub target_position: cgmath::Point3<f32>,
    pub up_direction: cgmath::Vector3<f32>,

    pub aspect_ratio: f32,
    pub znear: f32,
    pub zfar: f32,
}

impl Camera {
    pub fn get_view_projection_matrix(&self) -> cgmath::Matrix4<f32> {
        let view =
            cgmath::Matrix4::look_at_lh(self.eye_position, self.target_position, self.up_direction);
        let projection =
            cgmath::perspective(cgmath::Deg(45.0), self.aspect_ratio, self.znear, self.zfar);

        view * projection
    }
}
