#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "../resource.h"
#include <windows.h>
#include <gl/GL.h>
#include <string>

// Forward declarations
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global variables
enum InstallerScreen { LICENSE, OPTIONS, INSTALLING };
InstallerScreen currentScreen = LICENSE;
int agreementChoice = 0; // 0 = reject, 1 = accept
bool createDesktopShortcut = true;
float installProgress = 0.0f;
bool isInstalling = false;
float installTimer = 0.0f;

// Color scheme
ImVec4 bgColor = ImVec4(0.114f, 0.208f, 0.361f, 1.0f);        // #1d355c
ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);            // #FFFFFF
ImVec4 lineColor = ImVec4(0.176f, 0.298f, 0.490f, 1.0f);      // #2d4c7d
ImVec4 buttonColor = ImVec4(0.988f, 0.761f, 0.078f, 1.0f);    // #fcc214
ImVec4 buttonTextColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);      // #000000

// Function to load font from embedded resource
ImFont* LoadFontFromResource(int resourceId, float fontSize) {
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hResource) return nullptr;

    HGLOBAL hMemory = LoadResource(NULL, hResource);
    if (!hMemory) return nullptr;

    DWORD dwSize = SizeofResource(NULL, hResource);
    LPVOID lpAddress = LockResource(hMemory);
    if (!lpAddress) return nullptr;

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;

    return io.Fonts->AddFontFromMemoryTTF(lpAddress, dwSize, fontSize, &fontConfig);
}

// Window procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create window class
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "InstallerWindow", NULL };
    RegisterClassExA(&wc);

    // Create window
    HWND hwnd = CreateWindowA(wc.lpszClassName, "Setup Wizard", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

    // Initialize OpenGL
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };

    HDC hdc = GetDC(hwnd);
    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);
    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    // Show window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;

    // Load custom fonts from embedded resources
    ImFont* fontRegular = LoadFontFromResource(IDR_FONT_REGULAR, 18.5f);
    ImFont* fontBold = LoadFontFromResource(IDR_FONT_BOLD, 18.5f);

    // If fonts failed to load, add default font as fallback
    if (!fontRegular && !fontBold) {
        io.Fonts->AddFontDefault();
    }

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 3.0f;
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = bgColor;
    style.Colors[ImGuiCol_Text] = textColor;
    style.Colors[ImGuiCol_Border] = lineColor;
    style.Colors[ImGuiCol_Separator] = lineColor;
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.3f, 0.4f, 0.5f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.4f, 0.5f, 0.6f);
    style.Colors[ImGuiCol_CheckMark] = buttonColor;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Main loop
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Setup", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // Use custom font if loaded
        if (fontRegular) ImGui::PushFont(fontRegular);

        // Screen 1: License Agreement
        if (currentScreen == LICENSE) {
            if (fontBold) ImGui::PushFont(fontBold);
            ImGui::TextWrapped("Please read the following important information before continuing.");
            if (fontBold) ImGui::PopFont();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("LicenseText", ImVec2(0, -120), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::TextWrapped(
                "THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.\n\n"
                "EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, "
                "EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                "THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.\n\n"
                "SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION."
            );
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::RadioButton("I accept the agreement", &agreementChoice, 1);
            ImGui::RadioButton("I do not accept the agreement", &agreementChoice, 0);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 120.0f;
            float spacing = 10.0f;
            float totalWidth = buttonWidth * 2 + spacing;
            float startX = ImGui::GetWindowWidth() - totalWidth - 20.0f;

            ImGui::SetCursorPosX(startX);
            if (agreementChoice == 1) {
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.7f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, buttonTextColor);
                if (ImGui::Button("Next", ImVec2(buttonWidth, 40))) {
                    currentScreen = OPTIONS;
                }
                ImGui::PopStyleColor(4);
            } else {
                ImGui::BeginDisabled();
                ImGui::Button("Next", ImVec2(buttonWidth, 40));
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 40))) {
                PostQuitMessage(0);
            }
        }

        // Screen 2: Installation Options
        else if (currentScreen == OPTIONS) {
            if (fontBold) ImGui::PushFont(fontBold);
            ImGui::Text("Select installation options:");
            if (fontBold) ImGui::PopFont();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Checkbox("Create a desktop shortcut", &createDesktopShortcut);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Disk space needed: 25 MB");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 120.0f;
            float spacing = 10.0f;
            float totalWidth = buttonWidth * 2 + spacing;
            float startX = ImGui::GetWindowWidth() - totalWidth - 20.0f;

            ImGui::SetCursorPosX(startX);
            ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.7f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, buttonTextColor);
            if (ImGui::Button("Install", ImVec2(buttonWidth, 40))) {
                currentScreen = INSTALLING;
                isInstalling = true;
                installProgress = 0.0f;
                installTimer = 0.0f;
            }
            ImGui::PopStyleColor(4);

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 40))) {
                PostQuitMessage(0);
            }
        }

        // Screen 3: Installing
        else if (currentScreen == INSTALLING) {
            if (fontBold) ImGui::PushFont(fontBold);
            ImGui::Text("Installing...");
            if (fontBold) ImGui::PopFont();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            // Progress bar
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, buttonColor);
            ImGui::ProgressBar(installProgress, ImVec2(-1, 40));
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Text("Progress: %.0f%%", installProgress * 100.0f);

            // Update progress
            if (isInstalling && installProgress < 1.0f) {
                installTimer += io.DeltaTime;
                installProgress = installTimer / 5.0f; // 5 second installation
                if (installProgress >= 1.0f) {
                    installProgress = 1.0f;
                    isInstalling = false;
                }
            }

            if (installProgress >= 1.0f) {
                ImGui::Spacing();
                ImGui::Spacing();

                if (fontBold) ImGui::PushFont(fontBold);
                ImGui::Text("Installation complete!");
                if (fontBold) ImGui::PopFont();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float buttonWidth = 120.0f;
                float startX = (ImGui::GetWindowWidth() - buttonWidth) / 2.0f;
                ImGui::SetCursorPosX(startX);

                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.7f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, buttonTextColor);
                if (ImGui::Button("Finish", ImVec2(buttonWidth, 40))) {
                    PostQuitMessage(0);
                }
                ImGui::PopStyleColor(4);
            }
        }

        if (fontRegular) ImGui::PopFont();
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SwapBuffers(hdc);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);

    return 0;
}
