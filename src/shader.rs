use std::ffi::CString;
use widestring::U16CString;
use windows::{
    core::*,
    Win32::Graphics::{
        Direct3D::{Fxc::*, ID3DBlob, *},
        Direct3D11::*,
    },
};

// This file will be mostly unsafe, and will include lot of type conversions between strings, CString and UTF16CStrings. There will be some manually null terminations as well.
// Some assumptions taken are : entry point of VS shaders must be vs_main, and for PS shaders ps_main, and so on.
// Provides public safe functions for compiling shaders, where these functions return a ID3D11XShader.
enum ShaderType {
    Vertex(String),
    Pixel(String),
}

pub fn create_vertex_shader(device: &ID3D11Device5, shader_path: &str) -> ID3D11VertexShader {
    let shader_path = String::from(shader_path);

    let vs_bytecode = compile_shader(ShaderType::Vertex(shader_path))
        .expect("Failed to compile vertex shader : {shader_path}.");

    let bytecode_length = unsafe { vs_bytecode.GetBufferSize() };

    let vs_bytecode = unsafe { vs_bytecode.GetBufferPointer() as *const u8 };

    // Using std::slice::from_raw_parts to get a slice from a pointer and the length.
    let vs_bytecode = unsafe { std::slice::from_raw_parts(vs_bytecode, bytecode_length) };

    let vertex_shader: ID3D11VertexShader = unsafe {
        device
            .CreateVertexShader(vs_bytecode, None)
            .expect("Failed to create vertex shader.")
    };

    vertex_shader
}

pub fn create_pixel_shader(device: &ID3D11Device5, shader_path: &str) -> ID3D11PixelShader {
    let shader_path = String::from(shader_path);

    let ps_bytecode = compile_shader(ShaderType::Pixel(shader_path))
        .expect("Failed to compile pixel shader : {shader_path}.");

    let bytecode_length = unsafe { ps_bytecode.GetBufferSize() };

    let ps_bytecode = unsafe { ps_bytecode.GetBufferPointer() as *const u8 };
    // Using std::slice::from_raw_parts to get a slice from a pointer and the length.
    let ps_bytecode = unsafe { std::slice::from_raw_parts(ps_bytecode, bytecode_length) };

    let pixel_shader: ID3D11PixelShader = unsafe {
        device
            .CreatePixelShader(ps_bytecode, None)
            .expect("Failed to create pixel shader.")
    };

    pixel_shader
}

fn check_errors(blob: &Option<ID3DBlob>) {
    unsafe {
        // Reference for this ffi complicated stuff : https://github.com/seonhwi07jp/Directx12RustSample/blob/cc5536326e85919a4e682259da5981b4b1d5d7a2/ssun_renderer/src/shaders/shader.rs
        let error_message = blob
            .as_ref()
            .expect("Error Blob was Null")
            .GetBufferPointer() as *mut i8;

        let error_message = CString::from_raw(error_message);
        let error_message = error_message
            .to_str()
            .expect("Failed to convert CString to String.");

        println!("Error : {:?}", error_message);
    }
}

// note(rtarun9) : very strange and bad code, because of PCWSTR and how windows-rs handles strings.
// The target profile and entry point also need to be manually null terminated.
fn compile_shader(shader_type: ShaderType) -> Option<ID3DBlob> {
    let compile_flags = if cfg!(debug_assertions) {
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
    } else {
        0
    };

    let mut compiled_shader_blob: Option<ID3DBlob> = None;
    let mut compilation_error_blob: Option<ID3DBlob> = None;

    let mut shader_path: U16CString = U16CString::new();
    let mut entry_point: PCSTR = PCSTR::null();
    let mut target_profile: PCSTR = PCSTR::null();

    match shader_type {
        ShaderType::Vertex(vs_shader_path) => {
            entry_point = PCSTR("vs_main\0".as_ptr());
            target_profile = PCSTR("vs_5_0\0".as_ptr());
            shader_path = U16CString::from_str(vs_shader_path)
                .expect("Failed to converot String to U16CString.");
        }
        ShaderType::Pixel(ps_shader_path) => {
            entry_point = PCSTR("ps_main\0".as_ptr());
            target_profile = PCSTR("ps_5_0\0".as_ptr());
            shader_path = U16CString::from_str(ps_shader_path)
                .expect("Failed to converot String to U16CString.");
        }
    };

    unsafe {
        match D3DCompileFromFile(
            PCWSTR(shader_path.as_ptr()),
            None,
            None,
            entry_point,
            target_profile,
            compile_flags,
            0,
            &mut compiled_shader_blob,
            Some(&mut compilation_error_blob as *mut Option<ID3DBlob>),
        ) {
            Ok(()) => {}
            Err(error) => {
                println!("Error Code : {:?}", error.message());
                check_errors(&compilation_error_blob);
            }
        }
    }

    return compiled_shader_blob;
}
