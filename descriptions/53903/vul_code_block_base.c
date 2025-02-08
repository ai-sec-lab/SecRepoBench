set_dev_proc(dev, output_page, gx_default_output_page);
    set_dev_proc(dev, close_device, mem_close);
    set_dev_proc(dev, get_params, gx_default_get_params);
    set_dev_proc(dev, put_params, gx_default_put_params);
    set_dev_proc(dev, get_page_device, gx_forward_get_page_device);
    set_dev_proc(dev, get_alpha_bits, gx_default_get_alpha_bits);
    set_dev_proc(dev, fill_path, gx_default_fill_path);