dxc -spirv -T vs_6_6 -E vs_main src/shaders.hlsl -Fo src/vs_shader.cso
dxc -spirv -T ps_6_6 -E ps_main src/shaders.hlsl -Fo src/ps_shader.cso