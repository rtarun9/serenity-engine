use wgpu::include_wgsl;
use winit::dpi::PhysicalSize;
use winit::event::{ElementState, MouseButton, WindowEvent};

use winit::window::Window;
pub struct Renderer {
    instance: wgpu::Instance,
    surface: wgpu::Surface,
    adapter: wgpu::Adapter,
    device: wgpu::Device,
    queue: wgpu::Queue,
    surface_configuration: wgpu::SurfaceConfiguration,

    pub size: PhysicalSize<u32>,
    pub clear_color: wgpu::Color,

    pub render_pipeline: wgpu::RenderPipeline
}

impl Renderer {
    pub async fn new(window: &Window) -> Self {
        // Initialize wgpu

        // Use DX12 on windows, or wgpu::Backends::all() otherwise (which will a backend that the  OS supports)
        let mut backends = wgpu::Backends::all();

        if cfg!(windows) {
            backends = wgpu::Backends::DX12;
        }

        // The instance is the handle to the active wgpu instance
        // Used to create adapter's and surface's
        let instance = wgpu::Instance::new(backends);

        let size = window.inner_size();

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

        let adapter_name = adapter.get_info().name;
        println!("Chosen adapter : {adapter_name}");

        let (device, queue) = adapter
            .request_device(
                &wgpu::DeviceDescriptor {
                    features: wgpu::Features::empty(),
                    limits: wgpu::Limits::default(),
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
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            width: size.width,
            height: size.height,
            present_mode: wgpu::PresentMode::Fifo,
        };

        surface.configure(&device, &surface_configuration);

        let clear_color = wgpu::Color {
            r: 0.2,
            g: 0.2,
            b: 0.2,
            a: 1.0,
        };

        // Create render pipeline
        let vertex_shader =  device.create_shader_module(wgpu::include_spirv!("vs_shader.cso"));
        let pixel_shader = device.create_shader_module(wgpu::include_spirv!("ps_shader.cso"));

        // Setup pipeline layout
        let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("Pipeline Layout"),
            bind_group_layouts: &[],
            push_constant_ranges: &[]
        });

        let render_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("Render Pipeline"),
            layout: Some(&pipeline_layout),
            vertex: wgpu::VertexState {
                module: &vertex_shader,
                entry_point: "vs_main",
                buffers: &[]
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

            clear_color,
            size,

            render_pipeline
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
                modifiers,
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
            render_pass.draw(0..3, 0..1);
        }

        // Submit the encoder for execution by the GPU
        self.queue.submit(std::iter::once(encoder.finish()));
        render_target.present();

        Ok(())
    }
}
