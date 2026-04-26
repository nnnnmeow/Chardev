#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <system_error>
#include <string>
#include <cstring>
#include <iostream>

#define MYDEV_IOC_MAGIC 'k'
#define MYDEV_IOC_CLEAR _IO(MYDEV_IOC_MAGIC, 0)

class Chardev {
    int fd_ = -1;
public:
    explicit Chardev(const char *path) {
        fd_ = ::open(path, O_RDWR);
        if (fd_ < 0)
            throw std::system_error(errno, std::system_category(), "open");
    }

    ~Chardev() {
        if (fd_ >= 0)
            ::close(fd_);
    }

    Chardev(const Chardev&) = delete;
    Chardev& operator=(const Chardev&) = delete;

    void write(const std::string& data) {
        if (::write(fd_, data.data(), data.size()) < 0)
            throw std::system_error(errno, std::system_category(), "write");
    }

    std::string read(size_t max = 4096) {
        std::string buf(max, '\0');
        lseek(fd_, 0, SEEK_SET);
        ssize_t n = ::read(fd_, buf.data(), buf.size());
        if (n < 0)
            throw std::system_error(errno, std::system_category(), "read");
        buf.resize(n);
        return buf;
    }

    void clear() {
        if (::ioctl(fd_, MYDEV_IOC_CLEAR) < 0)
            throw std::system_error(errno, std::system_category(), "ioctl clear");
    }
};

int main() {
    try {
        // Test 1: write/read round-trip
        {
            Chardev dev("/dev/mydev");
            dev.write("Hello Kernel!");
            std::string out = dev.read();
            if (out == "Hello Kernel!") {
                std::cout << "[OK] hello\n";
            } else {
                std::cerr << "[FAIL] hello: got '" << out << "'\n";
                return 1;
            }
        }

        // Test 2: clear via ioctl
        {
            Chardev dev("/dev/mydev");
            dev.clear();
            if (dev.read().empty()) {
                std::cout << "[OK] clear\n";
            } else {
                std::cerr << "[FAIL] clear: read non-empty after clear\n";
                return 1;
            }
        }

        // Test 3: write larger than BUFFER_SIZE clamps to 1024
        {
            Chardev dev("/dev/mydev");
            dev.clear();
            std::string big(2000, 'A');
            dev.write(big);
            std::string result = dev.read();
            if (result.size() == 1024 && result == std::string(1024, 'A')) {
                std::cout << "[OK] overflow clamp (2000 -> 1024)\n";
            } else {
                std::cerr << "[FAIL] overflow clamp: got " << result.size() << " bytes\n";
                return 1;
            }
        }

        // Test 4: write replaces previous content (not appends)
        {
            Chardev dev("/dev/mydev");
            dev.write("first");
            dev.write("x");
            std::string result = dev.read();
            if (result == "x") {
                std::cout << "[OK] overwrite\n";
            } else {
                std::cerr << "[FAIL] overwrite: got '" << result << "', want 'x'\n";
                return 1;
            }
        }

        // Test 5: open/close stress — 100 iterations
        {
            for (int i = 0; i < 100; ++i) {
                Chardev dev("/dev/mydev");
                std::string msg = "iter-" + std::to_string(i);
                dev.write(msg);
                std::string result = dev.read();
                if (result != msg) {
                    std::cerr << "[FAIL] stress iter " << i
                              << ": got '" << result << "', want '" << msg << "'\n";
                    return 1;
                }
            }
            std::cout << "[OK] open/close stress (100 iters)\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
