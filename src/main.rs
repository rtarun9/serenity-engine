use raw_window_handle::{HasRawWindowHandle, RawWindowHandle};
use sdl2::{event::Event, keyboard::Keycode};
use windows::{
    core::Interface,
    Win32::Foundation::*,
    Win32::Graphics::{
        self,
        Direct3D::*,
        Direct3D11::*,
        Dxgi::{Common::*, *},
    },
};

fn main() {
    // Initialize SDL2 and create window.
    let sdl_context = sdl2::init().expect("Failed to initialize SDL2.");

    let video_subsystem = sdl_context
        .video()
        .expect("Failed to get SDL2 video subsystem.");

    let window = video_subsystem
        .window("serenity-engine", 1080, 720)
        .allow_highdpi()
        .position_centered()
        .build()
        .expect("Failed to create SDL2 window.");

    let mut event_pump = sdl_context
        .event_pump()
        .expect("Failed to get SDL2 event pump.");

    // Create the DXGI factory get the best performing adapter.
    const FACTORY_CREATION_FLAGS: u32 = if cfg!(debug_assertions) {
        DXGI_CREATE_FACTORY_DEBUG
    } else {
        0
    };

    let factory: IDXGIFactory6 = unsafe {
        CreateDXGIFactory2(FACTORY_CREATION_FLAGS).expect("Failed to create DXGI factory.")
    };

    // Create the Adapter, which represents the display subsystem (GPU, video memory, etc).
    let adapter: IDXGIAdapter1 = unsafe {
        factory
            .EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE)
            .expect("Failed to get adapter.")
    };

    let adapter_description = unsafe {
        adapter
            .GetDesc()
            .expect("Failed to get adapter description.")
            .Description
    };
    let adapter_description = String::from_utf16(&adapter_description)
        .expect("Failed to convert adapter description (utf 16) to String.");

    println!("Description of chosen adapter : {adapter_description}");

    // Create the D3D11 Device, used for creation of resources (represents a virtual adapter).
    // Also create the D3D11DeviceContext, which represents device context and used for recording GPU commands.
    let mut device: Option<ID3D11Device> = None;
    let mut device_context: Option<ID3D11DeviceContext> = None;

    let device_creation_flags: D3D11_CREATE_DEVICE_FLAG = if cfg!(debug_assertions) {
        D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT
    } else {
        D3D11_CREATE_DEVICE_BGRA_SUPPORT
    };

    unsafe {
        D3D11CreateDevice(
            &adapter,
            D3D_DRIVER_TYPE_UNKNOWN,
            None,
            device_creation_flags,
            Some(&[D3D_FEATURE_LEVEL_11_1]),
            D3D11_SDK_VERSION,
            Some(&mut device as *mut Option<ID3D11Device>),
            None,
            Some(&mut device_context as *mut Option<ID3D11DeviceContext>),
        )
        .expect("Failed to create D3D11 Device and D3D11 DeviceContext.");
    }

    let device: ID3D11Device5 = device
        .expect("Failed to create D3D11Device.")
        .cast()
        .expect("Failed to cast ID3D11Device to ID3D11Device5.");

    let device_context: ID3D11DeviceContext2 = device_context
        .expect("Failed to create D3D11DeviceContext.")
        .cast()
        .expect("Failed to cast ID3D11DeviceContext to ID3D11DeviceContext2.");

    // Enable the debug controller to validate commands at runtime. Also set breakpoints on invalid API usage.
    if cfg!(debug_assertions) {
        let debug_controller: ID3D11Debug = device
            .cast()
            .expect("Failed to get debug controller from the D3D11Device.");

        let info_queue: ID3D11InfoQueue = device
            .cast()
            .expect("Failed to get D3D11InfoQueue from the D3D11Device.");
        unsafe {
            info_queue
                .SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true)
                .unwrap();
            info_queue
                .SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true)
                .unwrap();
            info_queue
                .SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true)
                .unwrap();
        }
    }

    // Create the swapchain (implements one or more surfaces for storing rendered data before presenting to output).
    let swapchain_desc = DXGI_SWAP_CHAIN_DESC1 {
        Width: 0,
        Height: 0,
        Format: DXGI_FORMAT_R8G8B8A8_UNORM,
        Stereo: BOOL(0),
        SampleDesc: DXGI_SAMPLE_DESC {
            Count: 1,
            Quality: 0,
        },
        BufferUsage: DXGI_USAGE_RENDER_TARGET_OUTPUT,
        BufferCount: 2,
        Scaling: DXGI_SCALING_STRETCH,
        SwapEffect: DXGI_SWAP_EFFECT_FLIP_DISCARD,
        AlphaMode: DXGI_ALPHA_MODE_UNSPECIFIED,
        Flags: 0,
    };

    // Get raw window handle (HWND) from the SDL window.
    let window_raw_handle = window.raw_window_handle();

    let hwnd = match window_raw_handle {
        RawWindowHandle::Win32(win32_handle) => win32_handle.hwnd,
        _ => panic!("Was not able to get HWND from the SDL window!"),
    };

    let swapchain: IDXGISwapChain1 = unsafe {
        factory
            .CreateSwapChainForHwnd(&device, HWND(hwnd as isize), &swapchain_desc, None, None)
            .expect("Failed to create DXGI swapchain")
    };

    // Main game loop.
    'game_loop: loop {
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. } => {
                    break 'game_loop;
                }
                Event::KeyDown { keycode, .. } => {
                    if keycode == Some(Keycode::Escape) {
                        break 'game_loop;
                    }
                }
                _ => {}
            }
        }
    }
}
