extern {
    fn gets(buffer: *mut u8);

    fn system(buffer: *const u8);
}

fn main() {
    let buf1 = String::from("Hello");
    let buf2 = String::from("World");
    unsafe {
        gets(buf1.as_ptr() as *mut _);
    }
    if buf2 == "There" {
        unsafe {
            system("/bin/sh".as_ptr());
        }
    }
}
