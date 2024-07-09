use std::ffi::c_int;

// the upkr config to use, this can be modified to use other configs
fn config() -> upkr::Config {
    let mut config = upkr::Config::default();
    // Settings for "Z80 mode"
    config.use_bitstream = true;
    config.bitstream_is_big_endian = true;
    config.invert_bit_encoding = true;
    config.simplified_prob_update = true;
    config
}

#[no_mangle]
pub extern "C" fn upkr_compress(
    output_buffer: *mut u8,
    output_buffer_size: usize,
    input_buffer: *const u8,
    input_size: usize,
    compression_level: c_int,
) -> usize {
    let output_buffer = unsafe { std::slice::from_raw_parts_mut(output_buffer, output_buffer_size) };
    let input_buffer = unsafe { std::slice::from_raw_parts(input_buffer, input_size) };

    let packed_data = upkr::pack(input_buffer, compression_level.max(0).min(9) as u8, &config(), None);
    let copy_size = packed_data.len().min(output_buffer.len());
    output_buffer[..copy_size].copy_from_slice(&packed_data[..copy_size]);

    packed_data.len()
}
