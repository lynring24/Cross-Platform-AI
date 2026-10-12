#include <vulkan/vulkan_raii.hpp>

#include <iostream>
#include <cstring>

class ComputeApplication
{
public:
    bool run()
    {
        if (!initVulkan()) 
        {
            return false;            
        }
        // TODO: 초기화에 성공하면 확인용 메시지를 제출합니다.
        submitDebugMessage();
        return true;
    }

private:
    // TODO: validation layer 이름과 debug messenger 멤버를 추가합니다.

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    
    static constexpr const char* validationLayer = "VK_LAYER_KHRONOS_validation";

    bool initVulkan()
    {
        return createInstance() && setupDebugMessenger();
    }
    // TODO: 레이어 지원 확인, 콜백, messenger 생성, 메시지 제출 함수를 추가합니다.

    bool checkValidationLayerSupport()
    {
        const auto layers = context.enumerateInstanceLayerProperties();
        for (const auto& layer : layers)
        {
            // 사용할 레이어와 이름이 같은지 확인함 
            if (std::strcmp(layer.layerName, validationLayer) == 0)
            {
                return true;
            }
        }
        std::cerr << "Required layer not found: " << validationLayer << "\n";
        return false;
    }

    // 콜백 함수 
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity, 
        VkDebugUtilsMessageTypeFlagsEXT type, 
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, 
        void*
    )
    {
        std::cerr << "[debug " 
        << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity))
        << " " <<
        vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(type)) << "] ";
        std::cerr << (callbackData->pMessageIdName ? callbackData->pMessageIdName : "unnamed")
         << ": " << callbackData->pMessage << '\n';
        return VK_FALSE; 
    }

    bool setupDebugMessenger()
    {
        debugMessenger = instance.createDebugUtilsMessengerEXT(makeDebugMessengerCreateInfo());
        std::cout << "Debug messenger created.\n";
        return true;
    }

    // 콜백으로 확인용 메시지 보내기 
    void submitDebugMessage()
    {
        vk::DebugUtilsMessengerCallbackDataEXT callbackData{};
        callbackData.pMessageIdName = "tutorial.callback-check";
        callbackData.pMessage = "Application-injected callback check; this is not a validation error.";
        instance.submitDebugUtilsMessageEXT(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning, 
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral, 
            callbackData
        );
    }



    // 어떤 메시지를 받을지 설정하기 
    static vk::DebugUtilsMessengerCreateInfoEXT makeDebugMessengerCreateInfo()
    {
        // 생성 정보 구조체를 만들고 경고와 오류를 받고록 지정함 
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{}; 
        createInfo.messageSeverity = 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning 
        | 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        // 받을 메시지 종류 
        createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance; 
        //debugCallback을 지정하고 생성 정보를 반환한다. 

        createInfo.setPfnUserCallback(debugCallback);
        return createInfo;
    }

    bool createInstance()
    {
        // TODO: 레이어 지원 여부를 확인합니다.(인스턴스 생성 전에 검토)
        if (!checkValidationLayerSupport())
        {
            return false;
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "02-validation-layers";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // TODO: 확장 목록에서 VK_EXT_debug_utils 지원 여부를 확인합니다.
        bool debugUtilsAvailable = false;
        const auto extensions = context.enumerateInstanceExtensionProperties(); // C API: vkEnumerateInstanceExtensionProperties
        for (const auto& extension : extensions)
        {
            // TODO: VK_EXT_debug_utils와 이름이 일치하는지 확인합니다.
            if (std::strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)==0)
            {
                debugUtilsAvailable = true;
            }
        }

        if(!debugUtilsAvailable)
        {
            std::cerr << "Required instance extension not found : VK_EXT_debug_utils\n";
            return false;
        }

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        // TODO: 레이어와 확장을 활성화하고 콜백 생성 정보를 연결합니다.

        createInfo.enabledLayerCount = 1; 
        createInfo.ppEnabledLayerNames = &validationLayer; 
        
        // 이름 목록과 개수를 전달

        const char* requiredExtensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        createInfo.enabledExtensionCount = 1 ; 
        createInfo.ppEnabledExtensionNames = requiredExtensions;

        // 인스턴스를 생성하거나 해제하는 과정의 메시지를 받기위해 콜백 설정을 준비한다. 
        auto debugCreateInfo = makeDebugMessengerCreateInfo();
        createInfo.pNext = &debugCreateInfo;

        instance = vk::raii::Instance(context, createInfo); // C API: vkCreateInstance
        std::cout << "Validation layer enabled: " << validationLayer <<"\n";
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
    } // C API: vkDestroyInstance (app.instance 소멸자)

    std::cout << "Debug messenger and instance destroyed.\n";
    return 0;
}
