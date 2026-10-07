# 00. 환경 설정 실행 기록 (Ubuntu 24.04)

[`examples/00-env-settings`](../examples/00-env-settings/README.md)의 절차를 Ubuntu 24.04 노트북에서 실행한 명령과 결과를 기록합니다.

- 실행일: 2026-10-07
- 결과: 완료 기준 4개 항목 모두 통과

## 실행 환경

| 항목 | 값 |
|---|---|
| OS | Ubuntu 24.04.4 LTS (Linux 7.0.0-34-generic, x86_64) |
| C++ 컴파일러 | GCC 13.3.0 (`/usr/bin/c++`) |
| CMake | 3.28.3 (요구: 3.24 이상) |
| glslc | shaderc 2023.8, spirv-tools 2023.6~rc1, glslang 14.0.0 |
| Vulkan 로더 | 1.3.275 (`/usr/lib/x86_64-linux-gnu/libvulkan.so`) |
| Vulkan 헤더 | `/usr/include` (`libvulkan-dev`) |
| CMake 생성기 | Unix Makefiles |

## 1. 도구 설치

```sh
sudo apt update
sudo apt install build-essential cmake libvulkan-dev vulkan-tools glslc vulkan-validationlayers
```

GPU 드라이버는 이미 설치되어 있었습니다. Intel GPU는 Mesa 25.2.8, NVIDIA GPU는 드라이버 580.173.02를 사용합니다.

## 2. 도구 확인

```sh
cmake --version
```

```text
cmake version 3.28.3
```

```sh
glslc --version
```

```text
shaderc 2023.8-1build1
spirv-tools 2023.6~rc1-2
glslang 14.0.0-2

Target: SPIR-V 1.0
```

`Target: SPIR-V 1.0`은 기본 출력 대상입니다. 빌드에서는 `--target-env=vulkan1.3`으로 대상을 지정합니다.

```sh
vulkaninfo --summary
```

출력 중 필요한 부분만 발췌했습니다.

```text
Vulkan Instance Version: 1.3.275

Instance Layers: count = 6
--------------------------
VK_LAYER_INTEL_nullhw       INTEL NULL HW                1.1.73   version 1
VK_LAYER_KHRONOS_validation Khronos Validation Layer     1.3.275  version 1
VK_LAYER_MESA_device_select Linux device selection layer 1.4.303  version 1
VK_LAYER_MESA_overlay       Mesa Overlay layer           1.4.303  version 1
VK_LAYER_NV_optimus         NVIDIA Optimus layer         1.4.312  version 1
VK_LAYER_NV_present         NVIDIA GR2608 layer          1.4.312  version 1

Devices:
========
GPU0:
	apiVersion         = 1.4.318
	deviceType         = PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
	deviceName         = Intel(R) Graphics (ARL)
	driverName         = Intel open-source Mesa driver
	driverInfo         = Mesa 25.2.8-0ubuntu0.24.04.2
GPU1:
	apiVersion         = 1.4.312
	deviceType         = PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
	deviceName         = NVIDIA GeForce RTX 5090 Laptop GPU
	driverName         = NVIDIA
	driverInfo         = 580.173.02
GPU2:
	apiVersion         = 1.4.318
	deviceType         = PHYSICAL_DEVICE_TYPE_CPU
	deviceName         = llvmpipe (LLVM 20.1.2, 256 bits)
	driverName         = llvmpipe
```

장치 3개가 표시됩니다.

- GPU0: 내장 Intel GPU
- GPU1: 외장 NVIDIA GPU
- GPU2: Mesa의 CPU 소프트웨어 구현(llvmpipe). 실제 GPU가 아닙니다.

이후 장에서 장치를 선택할 때 `deviceType`이 `DISCRETE_GPU`인 GPU1을 우선 사용합니다.

장치의 `apiVersion`은 1.4이지만 로더는 1.3.275입니다. 애플리케이션이 사용할 수 있는 버전은 로더와 장치 버전 중 낮은 쪽이므로, 이 환경에서는 Vulkan 1.3을 기준으로 합니다.

## 3. 예제 빌드와 실행

```sh
cd examples/00-env-settings
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

```text
-- The CXX compiler identification is GNU 13.3.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found Vulkan: /usr/lib/x86_64-linux-gnu/libvulkan.so (found suitable version "1.3.275", minimum required is "1.3") found components: glslc missing components: glslangValidator
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: build
```

`missing components: glslangValidator`는 무시해도 됩니다. `CMakeLists.txt`는 `glslc` 컴포넌트만 요구합니다.

```sh
cmake --build build --config Debug --target env_check
```

```text
[ 33%] Compile check.comp
[ 33%] Built target env_shader
[ 66%] Building CXX object CMakeFiles/env_check.dir/main.cpp.o
[100%] Linking CXX executable env_check
[100%] Built target env_check
```

```sh
./build/env_check
echo $?
```

```text
Vulkan loader version: 1.3.275
0
```

## 4. 셰이더 개별 컴파일

```sh
glslc --target-env=vulkan1.3 check.comp -o build/check-manual.spv
echo $?
```

```text
0
```

생성된 파일:

```text
build/check.comp.spv     336 bytes
build/check-manual.spv   336 bytes
```

CMake가 만든 `check.comp.spv`와 직접 컴파일한 `check-manual.spv`는 바이트 단위로 같습니다(`cmp` 확인).

## 완료 기준 결과

| 확인 항목 | 필요한 결과 | 결과 |
|---|---|---|
| 장치 조회 | GPU 이름과 1.3 이상의 API 버전 표시 | 통과: RTX 5090 Laptop GPU 1.4.312, Intel Graphics (ARL) 1.4.318 |
| 레이어 설치 | `VK_LAYER_KHRONOS_validation` 표시 | 통과: 1.3.275 |
| C++ 빌드와 실행 | 1.3 이상의 로더 버전 출력, 종료 코드 0 | 통과: 1.3.275, 종료 코드 0 |
| 셰이더 컴파일 | 오류 없이 `check.comp.spv` 생성 | 통과: 336 bytes |

## 참고 사항

- Ubuntu 24.04 apt 패키지의 Vulkan 로더와 헤더는 1.3.275입니다. Vulkan 1.4 기능이 필요한 장에서는 [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home)를 설치해야 합니다.
- 노트북에 GPU가 여러 개 있으므로 이후 예제에서 장치를 명시적으로 선택해야 합니다. 첫 번째 장치(GPU0)는 내장 GPU입니다.
