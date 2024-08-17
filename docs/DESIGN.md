# Header-only?

- ❌Compile times are too long

# CRTP for the reactor class?

- The compiler should be smart enough to eliminate the vtable pointer

# FetchContent or Git submodules?

- If you clear your build folder frequently, iterations will take longer time because you'd need to fetch the repo
  again. Not an issue for git submodules

