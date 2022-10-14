use winit::{
    dpi::{PhysicalPosition, PhysicalSize},
    event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::WindowBuilder,
};

mod renderer;

// Initial window dimensions
const WINDOW_WIDTH: u32 = 1080;
const WINDOW_HEIGHT: u32 = 720;

#[tokio::main]
async fn main() {
    // Enable logging to indicate any wgpu errors
    env_logger::init();

    // Get winit event loop and window
    let event_loop = EventLoop::new();

    // Get primary monitor so we can center the window
    let monitor_size = event_loop
        .available_monitors()
        .next()
        .expect("Failed to get monitor")
        .size();

    let window = WindowBuilder::new()
        .with_title("serenity-engine")
        .with_inner_size(PhysicalSize::new(WINDOW_WIDTH, WINDOW_HEIGHT))
        .with_position(PhysicalPosition::new(
            monitor_size.width / 2 - WINDOW_WIDTH / 2,
            monitor_size.height / 2 - WINDOW_HEIGHT / 2,
        ))
        .build(&event_loop)
        .expect("Failed to create winit window.");

    let mut renderer = renderer::Renderer::new(&window).await;

    // Using a moving closure as argument to the event_loop.run function
    // Moving closures are lambdas (anonymous functions) that take ownership of all variables it uses
    event_loop.run(move |event, _, control_flow| match event {
        Event::WindowEvent {
            ref event,
            window_id,
        } if window_id == window.id() => {
            if !renderer.input(event) {
                match event {
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
                    WindowEvent::Resized(new_size) => {
                        renderer.resize(*new_size);
                    }
                    WindowEvent::ScaleFactorChanged { new_inner_size, .. } => {
                        renderer.resize(**new_inner_size);
                    }
                    _ => {}
                }
            }
        }
        Event::RedrawRequested(window_id) if window_id == window.id() => {
            renderer.update();
            match renderer.render() {
                Ok(_) => {}
                // If the surface is lost, reconfigure it
                Err(wgpu::SurfaceError::Lost) => renderer.resize(renderer.size),
                Err(wgpu::SurfaceError::OutOfMemory) => *control_flow = ControlFlow::Exit,
                Err(error_message) => eprintln!("{:?}", error_message),
            }
        }
        Event::MainEventsCleared => {
            window.request_redraw();
        }
        _ => {}
    });
}
