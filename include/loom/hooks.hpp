#pragma once

#include <dlfcn.h>

#define $LOOM_MAIN(a, b)                                                                 \
    int __main(int, char **);                                                            \
    int main(int argc, char *argv[]) {                                                   \
        hook_libc();                                                                     \
        return __main(argc, argv);                                                       \
    }                                                                                    \
    int __main(a, b)

#define $HOOK_DECLARE(func, ret, ...)                                                    \
    typedef ret (*func##_t)(__VA_ARGS__);                                                \
    __attribute__((unused)) static func##_t original_##func = nullptr

#define HOOK_CHECK(name)                                                                 \
    do {                                                                                 \
        if (original_##name == nullptr) {                                                \
            original_##name = (decltype(original_##name))dlsym(RTLD_NEXT, #name);        \
            if (original_##name == nullptr) {                                            \
                const char *error = dlerror();                                           \
                if (error != nullptr) {                                                  \
                    fprintf(stderr, "Hook failed for %s: %s\n", #name, error);           \
                } else {                                                                 \
                    fprintf(stderr, "Hook failed for %s: unknown error\n", #name);       \
                }                                                                        \
                exit(-1);                                                                \
            }                                                                            \
        }                                                                                \
    } while (0)

extern "C" {
__attribute__((unused)) const char *libc_name = "/usr/lib/libSystem.B.dylib";
__attribute__((unused)) void *libc = nullptr;

void hook_libc() {
    if (libc == nullptr) {
        libc = RTLD_NEXT;
        if (libc == nullptr) {
            fprintf(stderr, "ERROR: open %s failed.\n", libc_name);
            std::exit(-1);
        }
    }
}
$HOOK_DECLARE(read, ssize_t, int fd, void *buf, size_t count);
$HOOK_DECLARE(write, ssize_t, int fd, const void *buf, size_t count);

ssize_t write(int fd, const void *buf, size_t count) {
    HOOK_CHECK(write);
    printf("Write hook called\n");
    return original_write(fd, buf, count);
}
}