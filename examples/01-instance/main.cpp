#include <vulkan/vulkan_raii.hpp>

#include <iostream>

class ComputeApplication
{
public:
    void run()
    {
        initVulkan();
    }

private:
    // TODO: Context와 Instance 멤버를 추가합니다.
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    void initVulkan()
    {
        // TODO: createInstance()를 호출합니다.
        createInstance();
    }

    // TODO: createInstance() 함수를 추가하고 생성 정보를 채웁니다.
    void createInstance()
    {
        vk::ApplicationInfo appInfo{}; 
        appInfo.pApplicationName = "01-instance";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        
        // vulkan에 extension을 사용해서 기능을 추가함 
        // 시스템에서 제공하는 인스턴스 확장을 조회
        const auto extensions = context.enumerateInstanceExtensionProperties(); 
        std::cout << "Availble instance extensions:\n";
        for (const auto& extension :  extensions) 
        {
            std::cout << "  " << extension.extensionName <<'\n';
        }
        // 생성 정보에 applicaiton 정보 연결하기 (객체를 만들 때 필요한 설정을 생성 정보 구조체로 받음)
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        // context, createInfo 를 전달해서 instance 생성
        instance = vk::raii::Instance(context, createInfo); 
        std::cout << "Instance created.\n";
    }

};

int main()
{
    {
        ComputeApplication app;
        app.run();
    } // C API: vkDestroyInstance (app.instance 소멸자)

    std::cout << "Instance destroyed.\n";
    return 0;
}
