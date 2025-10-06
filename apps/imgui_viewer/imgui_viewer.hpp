#include "note.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

class ImGuiViewer {
private:
    std::string app_name{"NoteWiki 0.0"};
    GLFWwindow* window{nullptr};
    NoteDataManager noteDataManager;
    std::map<uint32_t, Note> visible;
    std::unordered_set<std::string> in_visible;
    ImFont* font_regular;
    ImFont* font_title;
public:
    ImGuiViewer() {
        Note default_note = noteDataManager.get_note("default");
        int count = 0;
        for (const auto& kid : default_note.children) {
            // visible.emplace_back(noteDataManager.get_note(kid));
            visible[count * 8192] = noteDataManager.get_note(kid, true);
            in_visible.insert(kid);
            count++;
        }
    }

    int setup() {
        if (!glfwInit())
            return -1;

        const char* glsl_version = "#version 130";
        window = glfwCreateWindow(800, 600, app_name.c_str(), nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        //setup fonts
        std::string font_name = "fonts/ConsolateElf.ttf";
        io.Fonts->Clear();

        // Build a custom range that includes ASCII + arrows + triangles
        static ImVector<ImWchar> ranges;
        {
            ImFontGlyphRangesBuilder b;
            b.AddRanges(io.Fonts->GetGlyphRangesDefault()); // ASCII
            b.AddText("↑↓←→▲▼△▽");                         // any extra symbols you need
            b.BuildRanges(&ranges);
        }

        // regular text
        font_regular = io.Fonts->AddFontFromFileTTF(font_name.c_str(), 20.0f, nullptr, ranges.Data);
        // titles
        font_title = io.Fonts->AddFontFromFileTTF(font_name.c_str(), 35.0f, nullptr, ranges.Data);
        // Build font atlas
        io.Fonts->Build();
        // set regular as default
        // ImGui::PushFont(font_regular);

        return 0;
    }

    int tearDown() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();

        return 0;
    }

    /**
     * 'visible' is a container that doesn't care about strict order,
     *   but it does care that when a tag is clicked on, that the note
     *   that opens, opens below the note with the tag that was clicked.
     * Using a ordered map with a integer index, we can add 'spaces'
     *   between the indices.
     * unit32_t and 8192 gives us plenty of open indices between each 
     *   starting index.
     * 'in_visible' is a container to have a reference from note titles
     *   to 'visible' indices.
     */
    void add_visible_note(u_int32_t index_hint, std::string title) {
        if (in_visible.find(title) != in_visible.end())
            return;
        auto before = visible.find(index_hint);
        auto after = std::next(before);
        uint32_t new_index;
        if (after != visible.end())
            new_index = (after->first - before->first) / 2 + before->first;
        else
            new_index = before->first + 8192;
        visible[new_index] = noteDataManager.get_note(title);
        in_visible.insert(title);
    }

    void create_notes() {
        ImGui::Begin("NoteWiki");

        for (auto it = visible.begin(); it != visible.end(); ++it) {
            auto& [index, note] = *it;
            ImGui::PushID(index);
            ImGui::BeginChild(note.title.c_str(), ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);

            // header + move controls
            ImGui::PushFont(font_title);
            ImGui::TextUnformatted(note.title.c_str());
            ImGui::PopFont();
            ImGui::SameLine();
            if (ImGui::Button("↑") && it != visible.begin())
                std::swap(it->second, std::prev(it)->second);
            ImGui::SameLine();
            if (ImGui::Button("↓") && std::next(it) != visible.end())
                std::swap(it->second, std::next(it)->second);

            ImGui::Text("Tags: "); ImGui::SameLine();
            for (const auto& tag : note.tags) {
                if (ImGui::Button(tag.c_str())) {
                    add_visible_note(index, tag.c_str());
                }
                ImGui::SameLine();
            }
            ImGui::Spacing();

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
            ImGui::BeginChild("ChildR", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders, window_flags);
            ImGui::TextWrapped(note.content.c_str());
            ImGui::EndChild();

            ImGui::Text("Tagged: ");
            for (const auto& child : note.children) {
                if (ImGui::Button(child.c_str())) {
                    add_visible_note(index, child.c_str());
                }
                ImGui::SameLine();
            }
            ImGui::EndChild();
            ImGui::PopID();
        }

        ImGui::End();
    }

    int run() {
        if (setup() != 0) return -1;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            create_notes();

            ImGui::ShowDemoWindow();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }

        return tearDown();
    }
};
