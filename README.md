# SilencePNG

A lightweight CLI tool for Windows that uses `oxipng` to losslessly compress and optimize PNG images.

## Features

- **Recursive scanning**: Finds and optimizes all `.png` files in the specified directory and its subdirectories.
- **Single-file support**: Optimizes a single PNG file when a file path is provided.
- **Automatic dependency installation**: Automatically installs `oxipng` via `winget` if it is not available on the system.
- **Graceful cancellation**: Pressing `Ctrl + C` immediately terminates the active child process and safely stops the operation.
- **Optimization statistics**: Displays the total size before and after optimization, space saved, and the number of modified files.

## Requirements

- Windows (x64)
- [oxipng](https://github.com/shssoichiro/oxipng) — automatically installed via `winget` on first use if not already available.

## Installation

Download and install SilencePNG from [here]().

The installer automatically adds SilencePNG to the system `PATH`, so you can use the following command directly from Command Prompt after installation:

```cmd
silencepng <path>
```

## Usage

```cmd
silencepng <file or directory path>
```

### Examples

Optimize all PNG files in a directory:

```cmd
silencepng "C:\path\to\images"
```

Optimize a single PNG file:

```cmd
silencepng "C:\path\to\image.png"
```

## Building

### GCC (MinGW-w64)

```cmd
gcc -O2 -s silencepng.c -o silencepng.exe
```

### MSVC

```cmd
cl /O2 /Fe:silencepng.exe silencepng.c
```
