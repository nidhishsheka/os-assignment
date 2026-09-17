# OpenMP Matrix Multiplication — Live Visualization

This version connects the **real C++/OpenMP multiplication engine** to a browser visualization.

## Requirements

- macOS
- Homebrew
- LLVM/Clang
- OpenMP runtime (`libomp`)
- Python 3 for the local web server

Install OpenMP if needed:

```bash
brew install libomp
```

## Build

From this project folder:

```bash
make
```

## Run

### Terminal 1

Start the visualization server:

```bash
python3 -m http.server 8000
```

Open:

```text
http://localhost:8000/visualization/
```

### Terminal 2

Run the actual C++ engine:

```bash
./matrix_multi
```

The C++ program continuously updates:

```text
matrix_data.json
```

The browser reads that file and displays the live state.

## What is actually being visualized?

The browser does **not** perform matrix multiplication.

The C++ OpenMP engine performs:

```text
C = A × B
```

using the optimized:

```text
i → k → j
```

loop order.

The visualization receives:

- Matrix A
- Matrix B
- completed result rows
- active rows
- completed rows per thread
- thread row ranges
- final execution time
- verification result

## Change matrix size / threads

Edit these values near the top of `main.cpp`:

```cpp
const int N = 100;
const int threads = 4;
```

For example:

```cpp
const int N = 500;
const int threads = 8;
```

Then:

```bash
make
./matrix_multi
```

The benchmark still runs with:

```text
1, 2, 4, 8 threads
```

## Clean

```bash
make clean
```
