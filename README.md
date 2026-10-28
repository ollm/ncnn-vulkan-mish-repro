# ncnn Vulkan Mish reproducer

This program creates ncnn's Vulkan `Mish` layer directly and verifies that it
returns finite values for large positive FP32 inputs. For `x >= 20`, Mish is
equal to `x` within FP32 precision.

The values `88.8` and `110` are intentionally included because the affected
AMD Windows Vulkan path produced `NaN` for positive values in that range.

## Requirements

- Git, to obtain the ncnn submodule
- A Vulkan SDK
- CMake 3.16 or newer

## Build

The repository includes ncnn as a submodule pinned to the affected revision.
No ncnn installation and no `ncnn_DIR` argument are required.

```sh
git submodule update --init --recursive
cmake -S . -B build -DNCNN_VULKAN=ON
cmake --build build -j"$(nproc)"
./build/ncnn-vulkan-mish-repro
```

On Windows with a Visual Studio generator:

```powershell
git submodule update --init --recursive
cmake -S . -B build -A x64
cmake --build build --config Release
.\build\Release\ncnn-vulkan-mish-repro.exe
```

## Downloading CI binaries

The **release** workflow builds Linux x64 and Windows x64 ZIP archives. Run it
manually from the Actions tab:

- Leave **Create a GitHub release with the binaries** enabled to publish the
  archives in a dated GitHub release.
- Disable it to keep the ZIP archives only as workflow artifacts.

The Windows archive contains `ncnn-vulkan-mish-repro.exe`; the Linux archive
contains `ncnn-vulkan-mish-repro`. Run the executable from a Vulkan-capable
system.

## Expected result

The program exits with code `0` and reports finite values approximately equal
to the input for every line:

```text
input=20 output=20
input=80 output=80
input=88.8 output=88.8
input=110 output=110
```

Before the fix, the affected Windows AMD Vulkan driver may return `NaN` for
the larger positive values. Linux RADV remained finite for the same program.
