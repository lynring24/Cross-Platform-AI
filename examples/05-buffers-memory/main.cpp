// 이전 장에서 생성한 논리적 디바이스와 큐를 바탕으로 연산할 데이터를 준비하고 연결합니다.
#include <vulkan/vulkan_raii.hpp>

// TODO: <array>와 <cstddef> 헤더를 추가합니다.
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <utility>

class ComputeApplication
{
public:
    bool run()
    {
        if (!initVulkan())
        {
            return false;
        }
        return true;
    }

private:
    static constexpr const char* validationLayer = "VK_LAYER_KHRONOS_validation";

    // TODO: 버퍼 크기와 BufferResource 구조체를 추가합니다.
    static_assert(sizeof(float) == 4);
    static constexpr std::uint32_t elementCount = 8;
    static constexpr vk::DeviceSize bufferSize = sizeof(float) * elementCount;
    struct BufferResource
    {
        vk::raii::DeviceMemory memory = nullptr;
        vk::raii::Buffer buffer = nullptr;
    // }
    };

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    std::uint32_t computeQueueFamilyIndex = 0;
    vk::raii::Device device = nullptr;
    vk::raii::Queue computeQueue = nullptr;
    // TODO: inputA, inputB, output 멤버를 추가합니다.
    BufferResource inputA;
    BufferResource inputB;
    BufferResource output; 

    bool initVulkan()
    {
        // TODO: 마지막에 createStorageBuffers() 호출을 연결합니다.
        return createInstance()
            && setupDebugMessenger()
            && pickPhysicalDevice()
            && createLogicalDevice()
            && createStorageBuffers();
    }

    bool checkValidationLayerSupport()
    {
        const auto layers = context.enumerateInstanceLayerProperties(); // C API: vkEnumerateInstanceLayerProperties
        for (const auto& layer : layers)
        {
            if (std::strcmp(layer.layerName, validationLayer) == 0)
            {
                return true;
            }
        }
        std::cerr << "Required layer not found: " << validationLayer << '\n';
        return false;
    }

    bool createInstance()
    {
        if (!checkValidationLayerSupport())
        {
            return false;
        }

        const auto extensions = context.enumerateInstanceExtensionProperties(); // C API: vkEnumerateInstanceExtensionProperties
        bool debugUtilsAvailable = false;
        for (const auto& extension : extensions)
        {
            if (std::strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                debugUtilsAvailable = true;
            }
        }
        if (!debugUtilsAvailable)
        {
            std::cerr << "Required instance extension not found: VK_EXT_debug_utils\n";
            return false;
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "05-buffers-memory";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        const char* requiredExtensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        auto debugCreateInfo = makeDebugMessengerCreateInfo();
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &validationLayer;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = requiredExtensions;
        createInfo.pNext = &debugCreateInfo;

        instance = vk::raii::Instance(context, createInfo); // C API: vkCreateInstance
        return true;
    }

    static vk::DebugUtilsMessengerCreateInfoEXT makeDebugMessengerCreateInfo()
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                   | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                               | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                               | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        createInfo.setPfnUserCallback(debugCallback);
        return createInfo;
    }

    bool setupDebugMessenger()
    {
        debugMessenger = instance.createDebugUtilsMessengerEXT(makeDebugMessengerCreateInfo()); // C API: vkCreateDebugUtilsMessengerEXT
        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*)
    {
        std::cerr << "[debug "
                  << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity))
                  << ' ' << vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(type)) << "] "
                  << (callbackData->pMessageIdName ? callbackData->pMessageIdName : "unnamed")
                  << ": " << callbackData->pMessage << '\n';
        return VK_FALSE;
    }

    bool pickPhysicalDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices(); // C API: vkEnumeratePhysicalDevices
        if (physicalDevices.empty())
        {
            std::cerr << "No Vulkan physical devices found.\n";
            return false;
        }

        for (auto& candidate : physicalDevices)
        {
            const auto properties = candidate.getProperties(); // C API: vkGetPhysicalDeviceProperties
            if (properties.apiVersion < VK_API_VERSION_1_3)
            {
                continue;
            }
            const auto queueFamily = findComputeQueueFamily(candidate);
            if (!queueFamily)
            {
                continue;
            }

            physicalDevice = std::move(candidate);
            computeQueueFamilyIndex = *queueFamily;
            return true;
        }
        std::cerr << "No Vulkan 1.3 physical device with a compute queue family found.\n";
        return false;
    }

    static std::optional<std::uint32_t> findComputeQueueFamily(
        const vk::raii::PhysicalDevice& candidate)
    {
        const auto queueFamilies = candidate.getQueueFamilyProperties(); // C API: vkGetPhysicalDeviceQueueFamilyProperties
        for (std::uint32_t index = 0; index < queueFamilies.size(); ++index)
        {
            const auto& family = queueFamilies[index];
            if (family.queueCount > 0 && (family.queueFlags & vk::QueueFlagBits::eCompute))
            {
                return index;
            }
        }
        return std::nullopt;
    }


    bool createLogicalDevice()
    {
        const float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        vk::PhysicalDeviceFeatures enabledFeatures{};
        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.pEnabledFeatures = &enabledFeatures;

        device = vk::raii::Device(physicalDevice, createInfo); // C API: vkCreateDevice
        computeQueue = vk::raii::Queue(device, computeQueueFamilyIndex, 0); // C API: vkGetDeviceQueue
        return true;
    }
    // TODO: createStorageBuffers(), createBuffer(), findMemoryType(),
    bool createBuffer(vk::DeviceSize size, BufferResource& resource)
    {
        // 버퍼의 크기와 용도 지정 (compute shader용으로 사용하므로 eStoragebuffer로 지정 )
        // ?? 그럼 다른 플래그는 뭐가 있지
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
        // 큐 패밀리간 자원 공유 방식
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        resource.buffer = vk::raii::Buffer(device, bufferInfo);
        // 버퍼가 요구하는 메모리 확인 
        const auto requirements = resource.buffer.getMemoryRequirements();
        // 버퍼가 허용하는 유형 중에서도 CPU가 직접 기록할 수 있는 메모리
        // *ehostvisible (cpu 접근 가능) *ehostCoherent (cpu 접근에 필요한 flush, invalidate를 명시 필요 x )
        const auto properties = vk::MemoryPropertyFlagBits::eHostVisible 
            | vk::MemoryPropertyFlagBits::eHostCoherent;
        std::uint32_t memoryTypeIndex = 0;
        if (!findMemoryType(requirements.memoryTypeBits, properties, memoryTypeIndex))
        {
            // returne false;
            return false;
        }
        // 메모리를 할당하고 버퍼에 연결하기 
        vk::MemoryAllocateInfo allocateInfo{};
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;
        resource.memory = vk::raii::DeviceMemory(device, allocateInfo);
        resource.buffer.bindMemory(*resource.memory, 0);
        std::cout << "Buffer memory: size " << requirements.size
                << ", alignment " << requirements.alignment
                << ", type " << memoryTypeIndex << "\n";
        return true;
    }

    // 사용할 메모리 유형 찾기 
    bool findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties, 
                        std::uint32_t& typeIndex)
    {
        const auto memoryProperties = physicalDevice.getMemoryProperties();
        for (std::uint32_t i=0; i<memoryProperties.memoryTypeCount; ++i)
        {
            // 버퍼의 조건과 필요한 메모리 속성을 검사한다.
            const bool allowed = (typeFilter & (1u << i)) != 0; 
            // 그 외 조건을 검사
            const bool hasProperties = 
            ( memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
            if (allowed && hasProperties)
            {
                typeIndex = i; 
                return true;
            }
        }
        std::cerr << "No compatible HOST_VISIBLE | HOST_COHERENT memory type found.\n";
        return false;
    }

    // writeAndVerifyBuffer()를 추가합니다.
    bool writeAndVerifyBuffer(BufferResource& resource, 
                            const std::array<float, elementCount>& values,
                            const char* label)
    {
        void* mapped = resource.memory.mapMemory(0, bufferSize);
        std::memcpy(mapped, values.data(), static_cast<std::size_t>(bufferSize));
        resource.memory.unmapMemory();
        
        std::array<float, elementCount> readback{};
        mapped = resource.memory.mapMemory(0, bufferSize);
        std::memcpy(readback.data(), mapped, static_cast<std::size_t>(bufferSize));
        resource.memory.unmapMemory();
        if (readback != values)
        {
            std::cerr << label << ": cpu data verification failed.\n";
            return false;
        }
        std::cout << label << ":";
        for (float value : readback)
        {
            std::cout << " " << value;
        }
        std::cout <<"\n";
        return true;
    }

    bool createStorageBuffers()
    {
        const std::array buffers = {&inputA, &inputB, &output};
        for (auto* buffer : buffers)
        {
            if (!createBuffer(bufferSize, *buffer))
            {
                return false;
            }
        }

        std::array<float, elementCount> valuesA{};
        std::array<float, elementCount> valuesB{};
        const std::array<float, elementCount> initialOutput{};
        for (std::uint32_t i = 0; i < elementCount; ++i)
        {
            valuesA[i] = static_cast<float>(i);
            valuesB[i] = 10.0f + static_cast<float>(i);
        }
        if (!writeAndVerifyBuffer(inputA, valuesA, "Input A")
            || !writeAndVerifyBuffer(inputB, valuesB, "Input B")
            || !writeAndVerifyBuffer(output, initialOutput, "Output (initial)"))
        {
            return false;
        }
        std::cout << "Host-visible storage buffers ready: " << buffers.size()
                << " x " << bufferSize << " bytes.\n"
                << "CPU write/readback verified. No GPU dispatch yet.\n";
        return true;
    }


};

int main()
{
    {
        ComputeApplication app;
        if (!app.run())
        {
            return 1;
        }
    }

    std::cout << "Buffers and memory destroyed.\n";
    return 0;
}

/*
vk::BufferUsageFlagBits의 주요 플래그

eTransferSrc / eTransferDst: 복사 명령(vkCmdCopyBuffer 등)의 원본/대상. staging buffer로 GPU 전용 메모리에 올릴 때 사용합니다.
eUniformBuffer: 셰이더에서 읽기 전용으로 쓰는 작은 상수 데이터(UBO)
eStorageBuffer: 셰이더에서 읽기와 쓰기가 모두 가능한 큰 데이터(SSBO). compute에서 주로 사용합니다.
eUniformTexelBuffer / eStorageTexelBuffer: 포맷이 있는 texel 단위로 접근하는 버퍼
eVertexBuffer / eIndexBuffer: 그래픽스 파이프라인의 정점/인덱스 데이터
eIndirectBuffer: vkCmdDispatchIndirect, DrawIndirect의 인자를 GPU 버퍼에서 읽을 때
eShaderDeviceAddress (Vulkan 1.2+): 버퍼의 GPU 주소를 셰이더에서 포인터처럼 쓸 때
*/