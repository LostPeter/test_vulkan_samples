// author: LostPeter
// time:   2022-11-05

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Application
{
public:
    void run() 
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

 private:
    GLFWwindow* window;
    VkInstance instance;

    void initWindow() 
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        this->window = glfwCreateWindow(WIDTH, HEIGHT, "LostPeter - test_vulkan_0000_base_code", nullptr, nullptr);
    }

    void initVulkan() 
    {
        
    }

    void mainLoop() 
    {
        while (!glfwWindowShouldClose(this->window)) 
        {
            glfwPollEvents();

        }
    }

    void cleanup() 
    {
        glfwDestroyWindow(this->window);
        
        glfwTerminate();
    }

};

int main()
{
    Application app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}