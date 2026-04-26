# Chardev

Linux character driver kernel module with a 1 KB scratch buffer and RAII test

## Usage

### Build

```sh
make
sudo insmod chardev.ko
sudo chmod 666 /dev/mydev
```

### Test

```sh
cd test
make
./test
```

### Unload

```sh
sudo rmmod chardev
```

## Architecture


## Structure

```
chardev.c
Makefile
README.md
test/
  test.cpp
  Makefile
```

**Tested on** Linux 6.19.12-arch1-1, gcc/g++ 15.2.1

## License

GPL
