# Chardev

Linux character driver kernel module with a 1 KB scratch buffer and RAII test

## Usage

### Build

```bash
make
sudo insmod chardev.ko
sudo chmod 666 /dev/mydev
```

### Test

```bash
cd test
make
./test
```

### Unload

```bash
sudo rmmod chardev
```

## Architecture

### Structure

```
chardev.c
Makefile
README.md
test/
  test.cpp
  Makefile
```

### Lifecycle

**kzalloc** — makes 1024 byte buffer in kernel
**alloc_chrdev_region** — gets available major number
**cdev_init/cdev_add** — registers operation
**class_create + device_create** — creates /dev/mydev using udev

### Operations

**read** — copies from kernel buffer to userspace
**write** — copies from userspace, truncates writes larger than 1024, each write replaces buffer
**clear** — clears buffer

## Testing

### Test cases

**hello** — write/read test of "Hello Kernel!"
**clear** — ioctl(MYDEV_IOC_CLEAR) clears buffer and checks if it's clear
**overflow clamp** — writes 2000 bytes and checks if it's cut to 1024
**overwrite** — checks if it is overwritten or appended
**stress** — 100 iterations of open/write/read/close

**Tested on** Linux 6.19.12-arch1-1, gcc/g++ 15.2.1

## License

GPL-2.0
