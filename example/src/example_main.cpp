// Modified/Simplified version of (https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui_console.h"

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

csys::ItemLog &operator<<(csys::ItemLog &log, ImVec4 &vec)
{
    log << "ImVec4: [" << vec.x << ", "
        << vec.y << ", "
        << vec.z << ", "
        << vec.w << "]";
    return log;
}

static void imvec4_setter(ImVec4 & my_type, std::vector<int> vec)
{
    if (vec.size() < 4) return;

    my_type.x = vec[0]/255.f;
    my_type.y = vec[1]/255.f;
    my_type.z = vec[2]/255.f;
    my_type.w = vec[3]/255.f;
}

int main(int, char **)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "ImGui Console Basic Example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gladLoadGL() == 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

    ///////////////////////////////////////////////////////////////////////////
    // IMGUI CONSOLE //////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    // Create ImGui Console
    ImGuiConsole console;

    // Register variables
    console.System().RegisterVariable("background_color", clear_color, imvec4_setter);

    // Register scripts
    console.System().RegisterScript("test_script", "./console.script");

    // Register custom commands
    console.System().RegisterCommand("random_background_color", "Assigns a random color to the background application",
                                     [&clear_color]()
                                     {
                                         clear_color.x = (rand() % 256) / 256.f;
                                         clear_color.y = (rand() % 256) / 256.f;
                                         clear_color.z = (rand() % 256) / 256.f;
                                         clear_color.w = (rand() % 256) / 256.f;
                                     });
    console.System().RegisterCommand("reset_background_color", "Reset background color to its original value",
                                     [&clear_color, val = clear_color]()
                                     {
                                         clear_color = val;
                                     });

    // Log example information:
    console.System().Log(csys::ItemType::INFO) << "Welcome to the imgui-console example!" << csys::endl;
    console.System().Log(csys::ItemType::INFO) << "The following variables have been exposed to the console:" << csys::endl << csys::endl;
    console.System().Log(csys::ItemType::INFO) << "\tbackground_color - set: [int int int int]" << csys::endl;
    console.System().Log(csys::ItemType::INFO) << csys::endl << "Try running the following command:" << csys::endl;
    console.System().Log(csys::ItemType::INFO) << "\tset background_color [255 0 0 255]" << csys::endl << csys::endl;

    ///////////////////////////////////////////////////////////////////////////

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui Console
        console.Draw();

        // Show the big demo window
        ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
