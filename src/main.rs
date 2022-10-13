use winit::{
    dpi::PhysicalSize,
    event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

struct Renderer {
    instance: wgpu::Instance,
    surface: wgpu::Surface,
    adapter: wgpu::Adapter,
    device: wgpu::Device,
    queue: wgpu::Queue,
}

impl Renderer {
    async fn new(window: &Window) -> Self {
        // Initialize wgpu
        // The instance is the context for all wgpu objects
        // Used to create adapter's and surface's
        let instance = wgpu::Instance::new(wgpu::Backends::all());

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

        let (device, queue) = adapter
            .request_device(
                &wgpu::DeviceDescriptor {
                    ..Default::default()
                },
                None,
            )
            .await
            .expect("Failed to create Device and Queue");

        Renderer {
            instance,
            surface,
            adapter,
            device,
            queue,
        }
    }
}

fn main() {
    // Enable logging to indicate any wgpu errors
    env_logger::init();

    // Get winit event loop and window`
    let event_loop = EventLoop::new();

    let window = WindowBuilder::new()
        .with_title("serenity-engine")
        .with_inner_size(PhysicalSize::new(1080, 720))
        .build(&event_loop)
        .expect("Failed to create winit window.");

    let renderer = async { Renderer::new(&window).await };
    async { renderer.await };

    // Using a moving closure as argument to the event_loop.run function
    // Moving closures are lambdas (anonymous functions) that take ownership of all variables it uses
    event_loop.run(move |event, _, control_flow| match event {
        Event::WindowEvent {
            ref event,
            window_id,
        } if window_id == window.id() => match event {
            WindowEvent::CloseRequested
            | WindowEvent::KeyboardInput {
                input:
                    KeyboardInput {
                        state: ElementState::Pressed,
                        virtual_keycode: Some(VirtualKeyCode::Escape),
                        ..
                    },
                ..
            } => *control_flow = ControlFlow::Exit,
            _ => {}
        },
        _ => (),
    });
}
