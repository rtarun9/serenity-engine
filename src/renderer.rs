use cgmath::SquareMatrix;
use image::GenericImageView;
use wgpu::util::DeviceExt;
use winit::dpi::PhysicalSize;
use winit::event::{ElementState, MouseButton, WindowEvent};
use winit::window::Window;

use crate::camera::{self, Camera};

#[repr(C)]
#[derive(Debug, Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
struct Vertex {
    position: [f32; 3],
    texture_coords: [f32; 2],
}

#[repr(C)]
#[derive(Debug, Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
struct CameraBuffer {
    view_projection_matrix: [[f32; 4]; 4],
}

impl CameraBuffer {
    fn new() -> Self {
        Self {
            view_projection_matrix: cgmath::Matrix4::identity().into(),
        }
    }

    fn update(&mut self, camera: camera::Camera) {
        self.view_projection_matrix = camera.get_view_projection_matrix().into();
    }
}
const VERTICES: [Vertex; 4] = [
    Vertex {
        position: [-0.5, -0.5, 0.0],
        texture_coords: [0.0, 1.0],
    },
    Vertex {
        position: [-0.5, 0.5, 0.0],
        texture_coords: [0.0, 0.0],
    },
    Vertex {
        position: [0.5, 0.5, 0.0],
        texture_coords: [1.0, 0.0],
    },
    Vertex {
        position: [0.5, -0.5, 0.0],
        texture_coords: [1.0, 1.0],
    },
];

const INDICES: [u32; 6] = [0, 1, 2, 0, 2, 3];

pub struct Renderer {
    instance: wgpu::Instance,
    surface: wgpu::Surface,
    adapter: wgpu::Adapter,
    device: wgpu::Device,
    queue: wgpu::Queue,
    surface_configuration: wgpu::SurfaceConfiguration,

    pub size: PhysicalSize<u32>,
    camera: camera::Camera,
    camera_buffer_data: CameraBuffer,

    clear_color: wgpu::Color,

    render_pipeline: wgpu::RenderPipeline,

    vertex_buffer: wgpu::Buffer,
    index_buffer: wgpu::Buffer,

    bind_group: wgpu::BindGroup,
}

impl Renderer {
    pub async fn new(window: &Window) -> Self {
        let size = window.inner_size();

        // Create camera
        let camera = camera::Camera {
            eye_position: (0.0, 0.0, -10.0).into(),
            target_position: (0.0, 0.0, 0.0).into(),
            up_direction: (0.0, 1.0, 0.0).into(),
            aspect_ratio: size.width as f32 / size.height as f32,
            znear: 0.1,
            zfar: 100.0,
        };

        // Initialize wgpu
        // Use DX12 only for now
        let mut backends = wgpu::Backends::DX12;

        // The instance is the handle to the active wgpu instance
        // Used to create adapter's and surface's
        let instance = wgpu::Instance::new(backends);

        // Surface is the part of the window we draw / render to
        let surface = unsafe { instance.create_surface(window) };

        // Handle to the actual GPU. We need a adapter capable of rendering to the surface
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                compatible_surface: Some(&surface),
                force_fallback_adapter: false,
            })
            .await
            .expect("Failed to get a compatible adapter.");

        println!("Chosen adapter : {}", adapter.get_info().name);

        let (device, queue) = adapter
            .request_device(
                &wgpu::DeviceDescriptor {
                    features: wgpu::Features::PUSH_CONSTANTS,
                    limits: wgpu::Limits {max_push_constant_size: (128), ..Default::default() },
                    label: None,
                },
                None,
            )
            .await
            .expect("Failed to create Device and Queue");

        // Define how the surface creates the underlying SurfaceTexture
        // wgpu::PresentMode::Fifo causes no tearing, supported on all platforms, and is Vsync On
        let surface_configuration = wgpu::SurfaceConfiguration {
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            format: wgpu::TextureFormat::Bgra8UnormSrgb,
            width: size.width,
            height: size.height,
            present_mode: wgpu::PresentMode::Fifo,
            alpha_mode: wgpu::CompositeAlphaMode::Auto
        };

        surface.configure(&device, &surface_configuration);

        let clear_color = wgpu::Color {
            r: 0.2,
            g: 0.2,
            b: 0.2,
            a: 1.0,
        };

        // Create buffers

        // bytemuck is used to cast T from [T; _] to &u[8]
        let vertex_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Vertex Buffer"),
            contents: bytemuck::cast_slice(&VERTICES),
            usage: wgpu::BufferUsages::VERTEX,
        });

        // Create vertex buffer layout
        let vertex_buffer_layout = wgpu::VertexBufferLayout {
            array_stride: std::mem::size_of::<Vertex>() as wgpu::BufferAddress,
            step_mode: wgpu::VertexStepMode::Vertex,
            attributes: &[
                wgpu::VertexAttribute {
                    format: wgpu::VertexFormat::Float32x3,
                    offset: 0,
                    shader_location: 0,
                },
                wgpu::VertexAttribute {
                    format: wgpu::VertexFormat::Float32x2,
                    offset: std::mem::size_of::<[f32; 3]>() as wgpu::BufferAddress,
                    shader_location: 1,
                },
            ],
        };

        // Create index buffer
        let index_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Index Buffer"),
            contents: bytemuck::cast_slice(&INDICES),
            usage: wgpu::BufferUsages::INDEX,
        });

        // Create camera constant / uniform buffer
        let camera_buffer_data = CameraBuffer::new();

        let push_constant_range = wgpu::PushConstantRange {
            stages: wgpu::ShaderStages::VERTEX,
            range: 0..std::mem::size_of::<CameraBuffer>() as u32,
        };

        // Create shader resources

        // Create diffuse texture
        let diffuse_texture_bytes = include_bytes!("../assets/happy_tree.png");
        let diffuse_image = image::load_from_memory(diffuse_texture_bytes)
            .expect("Failed to load happy_tree image.");
        let diffuse_image = diffuse_image
            .as_rgba8()
            .expect("Failed to cast image to rgb8 type.");
        let diffuse_image_dimensions = diffuse_image.dimensions();

        let texture_size = wgpu::Extent3d {
            width: diffuse_image_dimensions.0,
            height: diffuse_image_dimensions.1,
            depth_or_array_layers: 1,
        };

        // Here, usage is set to copy dest as we want to copy data to this texture
        // TEXTURE_BINDING indicates that it will be used as a SRV
        let diffuse_texture = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Diffuse Texture"),
            size: texture_size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
        });

        queue.write_texture(
            wgpu::ImageCopyTexture {
                texture: &diffuse_texture,
                mip_level: 0,
                origin: wgpu::Origin3d { x: 0, y: 0, z: 0 },
                aspect: wgpu::TextureAspect::All,
            },
            &diffuse_image,
            wgpu::ImageDataLayout {
                offset: 0,
                bytes_per_row: std::num::NonZeroU32::new(diffuse_image_dimensions.0 * 4),
                rows_per_image: std::num::NonZeroU32::new(diffuse_image_dimensions.1),
            },
            texture_size,
        );

        // Create texture view and sampler for the texture
        let diffuse_texture_view =
            diffuse_texture.create_view(&wgpu::TextureViewDescriptor::default());
        let anisotropic_sampler = device.create_sampler(&wgpu::SamplerDescriptor {
            label: Some("Anisotropic Sampler"),
            address_mode_u: wgpu::AddressMode::ClampToEdge,
            address_mode_v: wgpu::AddressMode::ClampToEdge,
            address_mode_w: wgpu::AddressMode::ClampToEdge,
            mag_filter: wgpu::FilterMode::Linear,
            min_filter: wgpu::FilterMode::Linear,
            anisotropy_clamp: std::num::NonZeroU8::new(16),
            ..Default::default()
        });

        // Setup bind group ot describe how the resources can be accessed by the shader
        let texture_bind_group_layout =
            device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
                label: Some("Texture Bind Group Layout"),
                entries: &[
                    wgpu::BindGroupLayoutEntry {
                        binding: 0,
                        visibility: wgpu::ShaderStages::FRAGMENT,
                        ty: wgpu::BindingType::Texture {
                            sample_type: wgpu::TextureSampleType::Float { filterable: true },
                            view_dimension: wgpu::TextureViewDimension::D2,
                            multisampled: false,
                        },
                        count: None,
                    },
                    wgpu::BindGroupLayoutEntry {
                        binding: 1,
                        visibility: wgpu::ShaderStages::FRAGMENT,
                        ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                        count: None,
                    },
                ],
            });

        let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("Bind Group"),
            layout: &texture_bind_group_layout,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: wgpu::BindingResource::TextureView(&diffuse_texture_view),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: wgpu::BindingResource::Sampler(&anisotropic_sampler),
                },
            ],
        });

        // Create render pipeline
        let vertex_shader = device.create_shader_module(wgpu::include_spirv!("vs_shader.cso"));
        let pixel_shader = device.create_shader_module(wgpu::include_spirv!("ps_shader.cso"));

        // Setup pipeline layout
        let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("Pipeline Layout"),
            bind_group_layouts: &[&texture_bind_group_layout],
            push_constant_ranges: &[push_constant_range],
        });

        let render_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("Render Pipeline"),
            layout: Some(&pipeline_layout),
            vertex: wgpu::VertexState {
                module: &vertex_shader,
                entry_point: "vs_main",
                buffers: &[vertex_buffer_layout],
            },
            fragment: Some(wgpu::FragmentState {
                module: &pixel_shader,
                entry_point: "ps_main",
                targets: &[Some(wgpu::ColorTargetState {
                    format: surface_configuration.format,
                    blend: Some(wgpu::BlendState::REPLACE),
                    write_mask: wgpu::ColorWrites::ALL,
                })],
            }),
            primitive: wgpu::PrimitiveState {
                topology: wgpu::PrimitiveTopology::TriangleList,
                strip_index_format: None,
                front_face: wgpu::FrontFace::Cw,
                cull_mode: Some(wgpu::Face::Back),
                polygon_mode: wgpu::PolygonMode::Fill,
                unclipped_depth: false,
                conservative: false,
            },
            depth_stencil: None,
            multisample: wgpu::MultisampleState {
                count: 1,
                mask: !0,
                alpha_to_coverage_enabled: false,
            },
            multiview: None,
        });

        Self {
            instance,
            surface,
            adapter,
            device,
            queue,
            surface_configuration,

            camera,
            camera_buffer_data,
            clear_color,
            size,

            render_pipeline,
            vertex_buffer,
            index_buffer,

            bind_group,
        }
    }

    pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
        if new_size.width > 0 && new_size.height > 0 {
            self.size = new_size;
            self.surface_configuration.width = new_size.width;
            self.surface_configuration.height = new_size.height;
            self.surface
                .configure(&self.device, &self.surface_configuration);
        }
    }

    // Returns a bool to indicate if event has been fully processed
    // If true is returned, main loop will halt event processing
    pub fn input(&mut self, event: &WindowEvent) -> bool {
        match event {
            WindowEvent::MouseInput {
                device_id,
                state,
                button,
                ..
            } => {
                if *button == MouseButton::Left && *state == ElementState::Pressed {
                    self.clear_color.r += 0.1;
                } else if *button == MouseButton::Right && *state == ElementState::Pressed {
                    self.clear_color.b -= -0.1;
                }
            }
            _ => {}
        }
        false
    }

    pub fn update(&mut self) {}

    pub fn render(&mut self) -> Result<(), wgpu::SurfaceError> {
        // Get a texture / frame to render into
        let render_target = self
            .surface
            .get_current_texture()
            .expect("Failed to get current render target texture from the surface.");

        // Create a texture view
        let view = render_target
            .texture
            .create_view(&wgpu::TextureViewDescriptor::default());

        // Create command encoder (command buffer) to record commands
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("Render Encoder"),
            });

        // Create render pass and record commands
        // This extra bock is present because encoder.begin_render_pass takes a mutable
        // reference to the encoder. The block will tell rust to drop all variables within the block to release all mutable borrows on encoder
        // The rendered output gets saved / stored into the view
        {
            let mut render_pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("Render Pass"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view: &view,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load: wgpu::LoadOp::Clear(self.clear_color),
                        store: true,
                    },
                })],
                depth_stencil_attachment: None,
            });

            render_pass.set_pipeline(&self.render_pipeline);
            render_pass.set_bind_group(0, &self.bind_group, &[]);
            render_pass.set_push_constants(
                wgpu::ShaderStages::VERTEX,
                0,
                bytemuck::cast_slice(&[self.camera_buffer_data]),
            );

            render_pass.set_index_buffer(self.index_buffer.slice(..), wgpu::IndexFormat::Uint32);
            render_pass.set_vertex_buffer(0, self.vertex_buffer.slice(..));

            render_pass.draw_indexed(0..6, 0, 0..1)
        }

        // Submit the encoder for execution by the GPU
        self.queue.submit(std::iter::once(encoder.finish()));
        render_target.present();

        Ok(())
    }
}
