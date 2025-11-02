#include "events.hpp"
#include "renderer_ctx.hpp"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>


class ImguiRenderer {
private:
    GLFWwindow* window{nullptr};
    ImFont* font_regular;
    ImFont* font_title;
    ImVec2 contentEditSize {-FLT_MIN, 30};
public:
    /**
     * Setup for ImgUI: creates the window and sets up the fonts
     */
    int setup(const std::string& app_name) {
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

        // Build a custom font range that includes ASCII + arrows + triangles
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

    /**
     * teardown for ImgUI
     */
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
     * Draws a inputText for a given string in edit mode.
     */
    void editText(std::string& editedText, const NoteId& id, const std::string& labelSuffix, RenderCtx& ctx) {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // or a fixed width you like
        std::string label = "##edit_" + std::to_string(id) + "_" + labelSuffix; // '##' = no visible label, keeps ID
        bool submitted = ImGui::InputText(
            label.c_str(),
            &editedText,
            ImGuiInputTextFlags_EnterReturnsTrue);

        // Commit on Enter:
        if (submitted) {
            // add Action to action queue
            ctx.events.push({EventType::SubmitEdit, id});
        }
        // Optional cancel with Esc:
        else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ctx.events.push({EventType::CancelEdit, id});
        }
    }

    void editMultilineText(std::string& editedText, NoteId id, RenderCtx ctx) {
        // size of the input text block
        ImVec2 text_size = ImGui::CalcTextSize(editedText.c_str(), nullptr, false);
        ImVec2 size = ImVec2(contentEditSize.x, text_size.y + contentEditSize.y);

        std::string label = "##edit_" + std::to_string(id) + "_content"; // '##' = no visible label, keeps ID
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
                                    ImGuiInputTextFlags_WordWrap | 
                                    ImGuiInputTextFlags_NoHorizontalScroll |
                                    ImGuiInputTextFlags_EnterReturnsTrue;
        bool submitted = ImGui::InputTextMultiline(label.c_str(), &editedText, size, flags);

        // Commit on Enter:
        if (submitted) {
            ctx.events.push({EventType::SubmitEdit, id});
        }
        // Optional cancel with Esc:
        else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ctx.events.push({EventType::CancelEdit, id});
        }
    }

    void displayNormalNote(const NoteData& note, NoteId id, const RenderCtx& ctx) {
        // title
        ImGui::PushFont(font_title);
        ImGui::TextUnformatted(note.title.c_str());
        if (!ctx.view.getEditMode() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            LOG_INFO() << "double clicked on title: " << note.title;
            ctx.events.push({EventType::BeginEdit, id});

        }
        ImGui::PopFont();
        ImGui::SameLine();

        // move note up
        if (ImGui::Button("↑"))
            ctx.events.push({EventType::MoveUp, id});

        ImGui::SameLine();
        // move note down
        if (ImGui::Button("↓"))
            ctx.events.push({EventType::MoveDown, id});

        // tags
        ImGui::Text("Tags: "); ImGui::SameLine();
        for (const auto& tag : note.tags) {
            const auto& note = ctx.store.getNote(tag);
            if (ImGui::Button(note.title.c_str())) {
                LOG_DEBUG() << "clicked on tag: " << tag;
                ctx.events.push({EventType::OpenId, tag, id});
            }
            ImGui::SameLine();
        }
        ImGui::Spacing();

        // content
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("ChildR", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders, window_flags);
        ImGui::TextWrapped("%s", note.content.c_str());  // a 'printf' style function
        if (!ctx.view.getEditMode() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            LOG_INFO() << "double clicked on content: " << note.title;
            ctx.events.push({EventType::BeginEdit, id});
        }
        ImGui::EndChild();

        // children
        ImGui::Text("Tagged in: ");
        for (const auto& kid : note.kids) {
            const auto& note = ctx.store.getNote(kid);
            if (ImGui::Button(note.title.c_str())) {
                LOG_DEBUG() << "clicked on kid: " << kid;
                ctx.events.push({EventType::OpenId, kid, id});
            }
            ImGui::SameLine();
        }
    }

    void displayEditedNote(const NoteId& id, RenderCtx ctx) {
        // title + move controls
        ImGui::PushFont(font_title);
        editText(ctx.view.getEditNote().title, id, "title", ctx);
        ImGui::PopFont();

        // Tags line
        ImGui::Text("Tags: "); ImGui::SameLine();
        editText(ctx.view.getEditNote().tags, id, "tags", ctx);
        ImGui::Spacing();

        // content
        editMultilineText(ctx.view.getEditNote().content, id, ctx);

        // children
        ImGui::Text("Tagged in: ");
        editText(ctx.view.getEditNote().kids, id, "children", ctx);
    }

    void renderNotes(const RenderCtx& ctx) {
        ImGui::Begin("NoteWiki");

        for (auto note_view : ctx.view.view()) {
            const auto& note = ctx.store.getNote(note_view.id);
            ImGui::PushID(note_view.id);
            ImGui::BeginChild(note.title.c_str(), ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);

            if (note_view.edit) displayEditedNote(note_view.id, ctx);
            else displayNormalNote(note, note_view.id, ctx);

            ImGui::EndChild();
            ImGui::PopID();
        }

        ImGui::End();
    }

    int render(const RenderCtx& ctx) {
        // The active loop that is always running and re-rendering the UI
        if (glfwWindowShouldClose(window)) return false;

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderNotes(ctx);

        // ImGui::ShowDemoWindow();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        return true;
    }
};
