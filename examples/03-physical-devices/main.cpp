#include <vulkan/vulkan_raii.hpp>

#include <cstring>
#include <iostream>
#include <cstdint>
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

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    std::uint32_t computeQueueFamilyIndex = 0 ;

    // TODO: 물리적 디바이스와 큐 패밀리 인덱스 멤버를 추가합니다.

    bool pickPhysicalDevice()
    {
        // 인스턴스로 접근 가능한 물리 디바이스 목록을 가져온다. 
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty())
        {
            std::cerr << "No vulkan physical devices found.\n";
            return false;
        }
        // 필요한 장치가 목록에 있는지 확인
        for (auto& candidate : physicalDevices)
        {
            // 이 장치의 API 버전과 큐 패밀리를 확인한다. 
            // 조건에 맞다면 장치 선택
            // 장치 이름, 종류, 지원하는 버전 API
            const auto properties = candidate.getProperties();
            if (properties.apiVersion < VK_API_VERSION_1_3)
            {
                continue;
            }
            // 사용가는한 연산용 큐가 있는지 확인한다. 
            const auto queueFamily = findComputeQueueFamily(candidate);
            if (!queueFamily)
            {
                continue;
            }
            physicalDevice = std::move(candidate);
            computeQueueFamilyIndex = *queueFamily;
            printDeviceInfo();
            return true;
        }
        std::cerr << "No Vulkan 1.3 physical device with a compute queue family found.\n";
        return false;
    }
    // 작업할 큐 집합 중 처리할 수 있는 패밀리를 검색한다.
    static std::optional<std::uint32_t> findComputeQueueFamily(
        const vk::raii::PhysicalDevice& candidate
    )
    {
        const auto queueFamilies = candidate.getQueueFamilyProperties();
        for (std::uint32_t index = 0 ; index < queueFamilies.size(); ++index)
        {
            const auto& family = queueFamilies[index];
            // 큐가 1가지 이상 있고, 컴퓨트 명령을 지원한다면 사용이 가능하다
            if (family.queueCount > 0 && (family.queueFlags& vk::QueueFlagBits::eCompute))
            {
                return index;
            }
        }
        return std::nullopt;
    }

    void printDeviceInfo()
    {
        const auto properties = physicalDevice.getProperties();
        std::cout << "Selected physical device: " << properties.deviceName << "\n"
                  << "Device type: " << vk::to_string(properties.deviceType) << "\n";
        std::cout << "Device API version: "
                  << VK_API_VERSION_MAJOR(properties.apiVersion) << "."
                  << VK_API_VERSION_MINOR(properties.apiVersion) << "."
                  << VK_API_VERSION_PATCH(properties.apiVersion) << "\n";
        
        // 큐 패밀리의 인덱스와 패밀리에 속한 큐 개수 확인 
        const auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        std::cout << "Compute queue family index: " << computeQueueFamilyIndex << "\n"
                  << "Queues in selected family: " << queueFamilies[computeQueueFamilyIndex].queueCount << '\n';
        
        const auto features = physicalDevice.getFeatures();
        std::cout << "shaderFloat64 supported (not enabled): " << (features.shaderFloat64 ? "yes" : "no") << "\n";
        
        // 지원 여부 조회 & 크기 확인
        const auto& limits = properties.limits;
        std::cout <<"maxComputeWorkGroupCount: " 
                <<limits.maxComputeWorkGroupCount[0] << ", " // 한 번의 dispatch에서 제출할 수 있는 각 축의 그룹 수
                <<limits.maxComputeWorkGroupCount[1] << ", "
                <<limits.maxComputeWorkGroupCount[2] << "\n"
                << "maxComputeWorkGroupSize: " 
                << limits.maxComputeWorkGroupSize[0] << ", "
                <<limits.maxComputeWorkGroupSize[1] << ", " 
                <<limits.maxComputeWorkGroupSize[2] << "\n";

        // 실행 단위 수 제한 및 스토리지 버퍼 디스림터로 지정 가능한 최대 바이트 범위        
        std::cout <<"maxComputeWorkGroupInvocations: "<< limits.maxComputeWorkGroupInvocations << '\n'
          << "maxStorageBufferRange: " << limits.maxStorageBufferRange << " bytes\n";
    }


    bool initVulkan()
    {
        // TODO: pickPhysicalDevice()를 초기화 순서에 추가합니다.
        return createInstance() && setupDebugMessenger() && pickPhysicalDevice();
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
        appInfo.pApplicationName = "03-physical-devices";
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

    // TODO: findComputeQueueFamily(), pickPhysicalDevice(), printDeviceInfo()를 추가합니다.
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

    return 0;
}
