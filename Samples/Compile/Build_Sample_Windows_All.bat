@REM author: LostPeter
@REM time:   2022-11-05

set debug=%1

call ./Build_Sample_Windows.bat test_vulkan_0000_base_code %debug%
call ./Build_Sample_Windows.bat test_vulkan_0001_instance_creation %debug%
call ./Build_Sample_Windows.bat test_vulkan_0002_validation_layers %debug%
call ./Build_Sample_Windows.bat test_vulkan_0003_physical_device_selection %debug%
call ./Build_Sample_Windows.bat test_vulkan_0004_logical_device %debug%
call ./Build_Sample_Windows.bat test_vulkan_0005_window_surface %debug%
call ./Build_Sample_Windows.bat test_vulkan_0006_swap_chain_creation %debug%
call ./Build_Sample_Windows.bat test_vulkan_0007_image_views %debug%
call ./Build_Sample_Windows.bat test_vulkan_0008_graphics_pipeline %debug%
call ./Build_Sample_Windows.bat test_vulkan_0009_shader_modules %debug%
call ./Build_Sample_Windows.bat test_vulkan_0010_fixed_functions %debug%
call ./Build_Sample_Windows.bat test_vulkan_0011_render_passes %debug%
call ./Build_Sample_Windows.bat test_vulkan_0012_graphics_pipeline_complete %debug%
call ./Build_Sample_Windows.bat test_vulkan_0013_framebuffers %debug%
call ./Build_Sample_Windows.bat test_vulkan_0014_command_buffers %debug%
call ./Build_Sample_Windows.bat test_vulkan_0015_hello_triangle %debug%
call ./Build_Sample_Windows.bat test_vulkan_0016_swap_chain_recreation %debug%
call ./Build_Sample_Windows.bat test_vulkan_0017_vertex_input %debug%
call ./Build_Sample_Windows.bat test_vulkan_0018_vertex_buffer %debug%
call ./Build_Sample_Windows.bat test_vulkan_0019_staging_buffer %debug%
call ./Build_Sample_Windows.bat test_vulkan_0020_index_buffer %debug%
call ./Build_Sample_Windows.bat test_vulkan_0021_descriptor_layout %debug%
call ./Build_Sample_Windows.bat test_vulkan_0022_descriptor_sets %debug%
call ./Build_Sample_Windows.bat test_vulkan_0023_texture_image %debug%
call ./Build_Sample_Windows.bat test_vulkan_0024_sampler %debug%
call ./Build_Sample_Windows.bat test_vulkan_0025_texture_mapping %debug%
call ./Build_Sample_Windows.bat test_vulkan_0026_depth_buffering %debug%
call ./Build_Sample_Windows.bat test_vulkan_0027_model_loading %debug%
call ./Build_Sample_Windows.bat test_vulkan_0028_mipmapping %debug%
call ./Build_Sample_Windows.bat test_vulkan_0029_multisampling %debug%