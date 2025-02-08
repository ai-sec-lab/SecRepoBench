void mem_initialize_device_procs(gx_device *dev)
{
    set_dev_proc(dev, get_initial_matrix, mem_get_initial_matrix);
    set_dev_proc(dev, sync_output, gx_default_sync_output);
    // <MASK>
    set_dev_proc(dev, stroke_path, gx_default_stroke_path);
    set_dev_proc(dev, fill_mask, gx_default_fill_mask);
    set_dev_proc(dev, fill_trapezoid, gx_default_fill_trapezoid);
    set_dev_proc(dev, fill_parallelogram, gx_default_fill_parallelogram);
    set_dev_proc(dev, fill_triangle, gx_default_fill_triangle);
    set_dev_proc(dev, draw_thin_line, mem_draw_thin_line);
    set_dev_proc(dev, get_clipping_box, gx_default_get_clipping_box);
    set_dev_proc(dev, begin_typed_image, gx_default_begin_typed_image);
    set_dev_proc(dev, composite, gx_default_composite);
    set_dev_proc(dev, get_hardware_params, gx_default_get_hardware_params);
    set_dev_proc(dev, text_begin, gx_default_text_begin);
    set_dev_proc(dev, transform_pixel_region, mem_transform_pixel_region);

    /* Defaults that may well get overridden. */
    set_dev_proc(dev, open_device, mem_open);
    set_dev_proc(dev, copy_alpha, gx_default_copy_alpha);
    set_dev_proc(dev, map_cmyk_color, gx_default_map_cmyk_color);
    set_dev_proc(dev, strip_tile_rectangle, gx_default_strip_tile_rectangle);
    set_dev_proc(dev, get_bits_rectangle, mem_get_bits_rectangle);
}