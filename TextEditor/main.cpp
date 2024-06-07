#include "TextEditor.h"
#include "PredictiveText.h"
#include <commdlg.h>
#include <string>
#include <codecvt>
#include <locale>
#include <fstream>
#include <vector>
#include <iostream> // Include for std::cout and std::cerr

std::wstring ConvertToWideString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

bool OpenFileDialog(std::string& filePath)
{
    wchar_t filename[MAX_PATH] = L""; // Initialize the buffer

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = L"txt";

    if (GetOpenFileName(&ofn))
    {
        filePath = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(filename);
        return true;
    }
    else
    {
        std::cerr << "Failed to open file dialog. Error code: " << CommDlgExtendedError() << std::endl;
        return false;
    }
}

bool SaveFileDialog(std::string& filePath)
{
    wchar_t filename[MAX_PATH] = L""; // Initialize the buffer

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";

    if (GetSaveFileName(&ofn))
    {
        filePath = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(filename);
        return true;
    }
    else
    {
        std::cerr << "Failed to open save file dialog. Error code: " << CommDlgExtendedError() << std::endl;
        return false;
    }
}

void ApplyCustomFont(ImGuiIO& io) {
    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 3;
    config.PixelSnapH = true;

    // Update the path to your font file
    const char* fontPath = "font/Roboto-Regular.ttf";
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &config);
    if (!font) {
        std::cerr << "Could not load font file: " << fontPath << std::endl;
        exit(1);
    }

    // Smaller font for buttons and status bar
    ImFont* smallFont = io.Fonts->AddFontFromFileTTF(fontPath, 14.0f, &config);
    if (!smallFont) {
        std::cerr << "Could not load small font file: " << fontPath << std::endl;
        exit(1);
    }
}

void UpdateWindowTitle(HWND hwnd, const std::string& filename, bool isUnsaved) {
    std::string title = "NHText - ";
    title += (filename.empty() ? "Untitled" : filename);
    if (isUnsaved) {
        title += " *";
    }
    SetWindowTextA(hwnd, title.c_str());
}

void LoadTrainingData(PredictiveText& predictor) {
    std::ifstream file("training/clean_data.txt");
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        predictor.Train(buffer.str());
        file.close();
    }
    else {
        std::cerr << "Failed to load training data." << std::endl;
    }
}

int main(int, char**)
{
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Text Editor", nullptr };
    RegisterClassEx(&wc);

    // Create window with a default size similar to a terminal window size (800x600)
    HWND hwnd = CreateWindow(wc.lpszClassName, L"NHText - Untitled", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Apply custom font
    ApplyCustomFont(io);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Change background and text color
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

    // Change the text editor background color
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Text buffer and current file path
    std::vector<char> textBuffer(1024 * 1024); // 1 MB buffer
    std::string currentFile;
    bool isUnsaved = false;

    // Main loop
    PredictiveText predictor;
    LoadTrainingData(predictor);
    MSG msg;
    bool done = false;
    while (!done)
    {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
            continue;
        }

        // Start the ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Handle Ctrl+S for saving the file
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) {
            if (ImGui::IsKeyPressed(ImGuiKey_S)) {
                if (currentFile.empty()) {
                    if (SaveFileDialog(currentFile)) {
                        SaveFile(currentFile, textBuffer);
                        isUnsaved = false;
                        UpdateWindowTitle(hwnd, currentFile, isUnsaved);
                    }
                }
                else {
                    SaveFile(currentFile, textBuffer);
                    isUnsaved = false;
                    UpdateWindowTitle(hwnd, currentFile, isUnsaved);
                }
            }
        }

        // Create your GUI here
        ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2((float)ImGui::GetIO().DisplaySize.x, (float)ImGui::GetIO().DisplaySize.y - 40)); // Adjust for bottom bar

            
        // Display buttons at the top
        ImGui::BeginChild("TopBar", ImVec2(ImGui::GetContentRegionAvail().x, 30), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);
        ImGui::PushFont(io.Fonts->Fonts[1]); // Use smaller font for buttons
        if (ImGui::Button("Open", ImVec2(60, 25))) {
            if (OpenFileDialog(currentFile)) {
                OpenFile(currentFile, textBuffer);
                isUnsaved = false;
                UpdateWindowTitle(hwnd, currentFile, isUnsaved);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(60, 25))) {
            if (currentFile.empty()) {
                if (SaveFileDialog(currentFile)) {
                    SaveFile(currentFile, textBuffer);
                    isUnsaved = false;
                    UpdateWindowTitle(hwnd, currentFile, isUnsaved);
                }
            }
            else {
                SaveFile(currentFile, textBuffer);
                isUnsaved = false;
                UpdateWindowTitle(hwnd, currentFile, isUnsaved);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save As", ImVec2(80, 25))) {
            if (SaveFileDialog(currentFile)) {
                SaveFile(currentFile, textBuffer);
                isUnsaved = false;
                UpdateWindowTitle(hwnd, currentFile, isUnsaved);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("New File", ImVec2(80, 25))) {
            std::fill(textBuffer.begin(), textBuffer.end(), 0);
            currentFile.clear();
            isUnsaved = false;
            UpdateWindowTitle(hwnd, currentFile, isUnsaved);
        }
        ImGui::PopFont();
        ImGui::EndChild();

        ImGui::Separator(); // Add a separator to create a border

        // Display text content using ImGui::InputTextMultiline
        if (ImGui::InputTextMultiline("##source", textBuffer.data(), textBuffer.size(), ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput)) {
            isUnsaved = true;
            UpdateWindowTitle(hwnd, currentFile, isUnsaved);
        }

        ImGui::End(); // End of text editor window 

        // Predictive text
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 40));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40));
        ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        std::string currentText(textBuffer.data());
        int wordCount = PredictiveText::CountWords(currentText);
        std::string lastWord = currentText.substr(currentText.find_last_of(" \t\n") + 1);
        std::string predictedWord = predictor.Predict(lastWord);

        ImGui::PushFont(io.Fonts->Fonts[1]); // Use smaller font for status bar
        ImGui::Text("Predicted Word: %s | 'Shit Tab' to complete", predictedWord.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 100); // Adjust this value if needed
        ImGui::Text("Word Count: %d", wordCount);
        ImGui::PopFont();
        ImGui::End();

        // Tab key for auto-completion
        if (ImGui::IsKeyPressed(ImGuiKey_Tab) && !predictedWord.empty()) {
            // Replace the last word with the predicted word when Tab is pressed
            size_t pos = currentText.find_last_of(" \t\n");
            currentText = currentText.substr(0, pos + 1) + predictedWord + " ";
            strcpy_s(textBuffer.data(), textBuffer.size(), currentText.c_str());
            isUnsaved = true;
            UpdateWindowTitle(hwnd, currentFile, isUnsaved);
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // White background
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
