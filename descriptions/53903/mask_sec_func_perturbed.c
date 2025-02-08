void mem_initialize_device_procs(gx_device *device)
{
    set_dev_proc(device, get_initial_matrix, mem_get_initial_matrix);
    set_dev_proc(device, sync_output, gx_default_sync_output);
    // <MASK>
    set_dev_proc(device, stroke_path, gx_default_stroke_path);
    set_dev_proc(device, fill_mask, gx_default_fill_mask);
    set_dev_proc(device, fill_trapezoid, gx_default_fill_trapezoid);
    set_dev_proc(device, fill_parallelogram, gx_default_fill_parallelogram);
    set_dev_proc(device, fill_triangle, gx_default_fill_triangle);
    set_dev_proc(device, draw_thin_line, mem_draw_thin_line);
    set_dev_proc(device, get_clipping_box, gx_default_get_clipping_box);
    set_dev_proc(device, begin_typed_image, gx_default_begin_typed_image);
    set_dev_proc(device, composite, gx_default_composite);
    set_dev_proc(device, get_hardware_params, gx_default_get_hardware_params);
    set_dev_proc(device, text_begin, gx_default_text_begin);
    set_dev_proc(device, transform_pixel_region, mem_transform_pixel_region);

    /* Defaults that may well get overridden. */
    set_dev_proc(device, open_device, mem_open);
    set_dev_proc(device, copy_alpha, gx_default_copy_alpha);
    set_dev_proc(device, map_cmyk_color, gx_default_map_cmyk_color);
    set_dev_proc(device, strip_tile_rectangle, gx_default_strip_tile_rectangle);
    set_dev_proc(device, get_bits_rectangle, mem_get_bits_rectangle);
}