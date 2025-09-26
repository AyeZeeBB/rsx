#include "pch.h"
#include "modern_layout.h"
#include <game/asset.h>
#include <core/render.h>
#include <core/window.h>
#include <core/utils/utils_general.h>
#include <thirdparty/imgui/misc/imgui_utility.h>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include <chrono>
#include <core/render/dx.h>
#include <core/filehandling/load.h>
#include <game/rtech/cpakfile.h>
#include <game/rtech/assets/texture.h>
#include <core/input/input.h>

// External declarations
extern CDXParentHandler* g_dxHandler;
extern CInput* g_pInput;
extern std::atomic<bool> inJobAction;

// Global for 3D model rendering - this is used by the main render loop
extern CDXDrawData* previewDrawData;

// Preview settings structure
extern PreviewSettings_t g_PreviewSettings;

// Forward declarations
extern CGlobalAssetData g_assetData;


// Icon constants for modern UI
#define ICON_FOLDER "📁"
#define ICON_FILE "📄"

namespace ModernUI
{
    LayoutManager* g_pModernLayout = nullptr;

    LayoutManager::LayoutManager()
    {
        // Initialize all panels as visible except console
        for (int i = 0; i < 6; ++i)
        {
            m_panelVisible[i] = true;
        }
        m_panelVisible[static_cast<int>(PanelType::Console)] = false;
        
        m_assetTreeRoot = AssetTreeNode("Assets", true);
    }

    LayoutManager::~LayoutManager()
    {
    }

    void LayoutManager::Initialize()
    {
        ApplyModernStyle();
        RefreshAssetTree();
    }

    void LayoutManager::Render()
    {
        RenderMenuBar();
        RenderMainLayout();
    }

    void LayoutManager::ApplyModernStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Modern color scheme - dark theme with blue accents
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 0.52f, 0.88f, 0.50f);
        
        // Backgrounds
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
        
        // Borders
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        
        // Frames
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        
        // Title bars
        colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        
        // Menu bar
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        
        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        
        // Interactive elements with blue accent
        const ImVec4 accentColor = GetAccentColor();
        colors[ImGuiCol_CheckMark] = accentColor;
        colors[ImGuiCol_SliderGrab] = accentColor;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(accentColor.x * 1.2f, accentColor.y * 1.2f, accentColor.z * 1.2f, 1.0f);
        
        // Buttons
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ButtonActive] = accentColor;
        
        // Headers
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_HeaderActive] = accentColor;
        
        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        
        // Tables
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.15f, 0.15f, 0.15f, 0.30f);
        
        // Separators
        colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = accentColor;
        colors[ImGuiCol_SeparatorActive] = accentColor;
        
        // Resize grips
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered] = accentColor;
        colors[ImGuiCol_ResizeGripActive] = accentColor;
        
        // Modern styling values
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.ChildRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;
        
        style.WindowPadding = ImVec2(12.0f, 12.0f);
        style.FramePadding = ImVec2(8.0f, 6.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.IndentSpacing = 20.0f;
        style.ScrollbarSize = 16.0f;
        style.GrabMinSize = 12.0f;
    }

    ImVec4 LayoutManager::GetAccentColor() const
    {
        return ImVec4(0.25f, 0.52f, 0.88f, 1.00f); // Modern blue
    }

    ImVec4 LayoutManager::GetSecondaryColor() const
    {
        return ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // Light gray
    }

    void LayoutManager::RenderMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    if (!inJobAction)
                    {
                        // Reset selected assets to avoid crash
                        m_selectedAssets.clear();
                        g_assetData.ClearAssetData();
                        
                        // Clear the tree immediately
                        m_assetTreeRoot.children.clear();

                        // Open file dialog in separate thread
                        CThread(HandleOpenFileDialog, g_dxHandler->GetWindowHandle()).detach();
                    }
                }
                if (ImGui::MenuItem("Unload Files"))
                {
                    if (!inJobAction)
                    {
                        m_selectedAssets.clear();
                        g_assetData.ClearAssetData();
                        RefreshAssetTree();
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    PostQuitMessage(0);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Asset Browser", nullptr, &m_panelVisible[static_cast<int>(PanelType::AssetBrowser)]);
                ImGui::MenuItem("Asset Inspector", nullptr, &m_panelVisible[static_cast<int>(PanelType::AssetInspector)]);
                ImGui::MenuItem("3D Viewport", nullptr, &m_panelVisible[static_cast<int>(PanelType::Viewport3D)]);
                ImGui::MenuItem("Asset Preview", nullptr, &m_panelVisible[static_cast<int>(PanelType::AssetPreview)]);
                ImGui::MenuItem("Properties", nullptr, &m_panelVisible[static_cast<int>(PanelType::Properties)]);
                ImGui::MenuItem("Console", nullptr, &m_panelVisible[static_cast<int>(PanelType::Console)]);
                ImGui::Separator();
                
                extern bool g_useModernLayout;
                if (ImGui::MenuItem("Toggle Layout Mode"))
                {
                    g_useModernLayout = !g_useModernLayout;
                }
                ImGui::Text("Current: %s", g_useModernLayout ? "Modern" : "Legacy");
                
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout"))
                {
                    // Reset to default layout
                    for (int i = 0; i < 5; ++i)
                        m_panelVisible[i] = true;
                    m_panelVisible[static_cast<int>(PanelType::Console)] = false;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Settings"))
                {
                    // Open settings
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About"))
                {
                    // Show about dialog
                }
                ImGui::EndMenu();
            }

            // Right-aligned FPS counter
            const ImGuiIO& io = ImGui::GetIO();
            const float fps = io.Framerate;
            const std::string fpsText = "FPS: " + std::to_string((int)fps);
            const float textWidth = ImGui::CalcTextSize(fpsText.c_str()).x;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textWidth - 10.0f);
            ImGui::Text("%s", fpsText.c_str());

            ImGui::EndMainMenuBar();
        }
    }

    void LayoutManager::RenderMainLayout()
    {
        // Get the main viewport
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | 
                                      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::Begin("ModernLayout_MainWindow", nullptr, windowFlags))
        {
            //const float menuHeight = ImGui::GetFrameHeight();
            const float availableHeight = ImGui::GetContentRegionAvail().y;
            const float bottomPanelHeight = m_panelVisible[static_cast<int>(PanelType::Console)] ? m_bottomPanelHeight : 0.0f;
            const float mainAreaHeight = availableHeight - bottomPanelHeight;

            // Create horizontal splitter for main area
            if (ImGui::BeginChild("MainArea", ImVec2(0, mainAreaHeight), false, ImGuiWindowFlags_NoScrollbar))
            {
                // Left panel
                if (m_panelVisible[static_cast<int>(PanelType::AssetBrowser)])
                {
                    if (ImGui::BeginChild("LeftPanel", ImVec2(m_leftPanelWidth, 0), true))
                    {
                        RenderAssetBrowser();
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();
                }

                // Center area
                const float rightPanelWidth = (m_panelVisible[static_cast<int>(PanelType::AssetInspector)] || 
                                              m_panelVisible[static_cast<int>(PanelType::Properties)]) ? m_rightPanelWidth : 0.0f;
                //const float leftPanelWidth = m_panelVisible[static_cast<int>(PanelType::AssetBrowser)] ? m_leftPanelWidth : 0.0f;
                const float centerWidth = ImGui::GetContentRegionAvail().x - rightPanelWidth;

                if (ImGui::BeginChild("CenterArea", ImVec2(centerWidth, 0), false))
                {
                    // Split center area vertically if both viewport and preview are visible
                    if (m_panelVisible[static_cast<int>(PanelType::Viewport3D)] && m_panelVisible[static_cast<int>(PanelType::AssetPreview)])
                    {
                        const float centerHeight = ImGui::GetContentRegionAvail().y;
                        if (ImGui::BeginChild("ViewportArea", ImVec2(0, centerHeight * 0.6f), true))
                        {
                            RenderViewport3D();
                        }
                        ImGui::EndChild();

                        if (ImGui::BeginChild("PreviewArea", ImVec2(0, 0), true))
                        {
                            RenderAssetPreview();
                        }
                        ImGui::EndChild();
                    }
                    else if (m_panelVisible[static_cast<int>(PanelType::Viewport3D)])
                    {
                        RenderViewport3D();
                    }
                    else if (m_panelVisible[static_cast<int>(PanelType::AssetPreview)])
                    {
                        RenderAssetPreview();
                    }
                }
                ImGui::EndChild();

                // Right panel
                if (m_panelVisible[static_cast<int>(PanelType::AssetInspector)] || m_panelVisible[static_cast<int>(PanelType::Properties)])
                {
                    ImGui::SameLine();
                    if (ImGui::BeginChild("RightPanel", ImVec2(m_rightPanelWidth, 0), true))
                    {
                        if (m_panelVisible[static_cast<int>(PanelType::AssetInspector)] && m_panelVisible[static_cast<int>(PanelType::Properties)])
                        {
                            // Split right panel with tabs
                            if (ImGui::BeginTabBar("RightPanelTabs"))
                            {
                                if (ImGui::BeginTabItem("Inspector"))
                                {
                                    RenderAssetInspector();
                                    ImGui::EndTabItem();
                                }
                                if (ImGui::BeginTabItem("Properties"))
                                {
                                    RenderProperties();
                                    ImGui::EndTabItem();
                                }
                                ImGui::EndTabBar();
                            }
                        }
                        else if (m_panelVisible[static_cast<int>(PanelType::AssetInspector)])
                        {
                            RenderAssetInspector();
                        }
                        else if (m_panelVisible[static_cast<int>(PanelType::Properties)])
                        {
                            RenderProperties();
                        }
                    }
                    ImGui::EndChild();
                }
            }
            ImGui::EndChild();

            // Bottom panel (console)
            if (m_panelVisible[static_cast<int>(PanelType::Console)])
            {
                if (ImGui::BeginChild("BottomPanel", ImVec2(0, bottomPanelHeight), true))
                {
                    RenderConsole();
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(3);
    }

    void LayoutManager::DrawSeparatorWithText(const char* text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, GetSecondaryColor());
        
        const float textWidth = ImGui::CalcTextSize(text).x;
        const float availWidth = ImGui::GetContentRegionAvail().x;
        const float separatorWidth = (availWidth - textWidth - 20.0f) * 0.5f;
        
        ImGui::Separator();
        ImGui::SameLine(separatorWidth);
        ImGui::Text("%s", text);
        ImGui::SameLine(separatorWidth + textWidth + 10.0f);
        
        ImGui::PopStyleColor();
    }

    bool LayoutManager::TreeNodeWithIcon(const char* icon, const char* label, bool* p_open)
    {
        p_open = p_open;
        ImGui::PushStyleColor(ImGuiCol_Text, GetAccentColor());
        ImGui::Text("%s", icon);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        return ImGui::TreeNode(label);
    }

    void LayoutManager::SetPanelVisible(PanelType panel, bool visible)
    {
        m_panelVisible[static_cast<int>(panel)] = visible;
    }

    bool LayoutManager::IsPanelVisible(PanelType panel) const
    {
        return m_panelVisible[static_cast<int>(panel)];
    }

    void LayoutManager::SetSelectedAssets(const std::vector<CAsset*>& assets)
    {
        m_selectedAssets = assets;
    }

    void LayoutManager::RefreshAssetTree()
    {
        BuildAssetTree();
    }

    void LayoutManager::RenderAssetBrowser()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
        
        // Header with title and controls
        ImGui::Text("Asset Browser");
        ImGui::SameLine();
        if (ImGui::SmallButton("Refresh"))
        {
            RefreshAssetTree();
        }
        
        ImGui::SameLine();
        if (ImGui::SmallButton("Force Rebuild"))
        {
            m_assetTreeRoot.children.clear();
            BuildAssetTree();
        }
        
        ImGui::Separator();
        
        // Auto-refresh disabled to prevent freezing
        // Use manual refresh buttons only
        
        // Search box
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputTextWithHint("##AssetSearch", "Search assets...", m_searchBuffer, sizeof(m_searchBuffer)))
        {
            // Filter will be applied during rendering
        }
        
        ImGui::Spacing();
        
        // View mode toggle
        if (ImGui::RadioButton("Tree View", m_showAssetTreeView))
        {
            m_showAssetTreeView = true;
            m_showAssetTableView = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("List View", m_showAssetTableView))
        {
            m_showAssetTreeView = false;
            m_showAssetTableView = true;
        }
        
        ImGui::Separator();
        
        // Asset count with safety check
        size_t currentAssetCount = 0;
        try {
            currentAssetCount = g_assetData.v_assets.size();
        } catch (...) {
            currentAssetCount = 0;
        }
        ImGui::Text("Assets: %zu", currentAssetCount);
        
        if (inJobAction)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(Loading...)");
        }
        
        ImGui::Spacing();
        
        // Main content area
        if (ImGui::BeginChild("AssetBrowserContent", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (m_showAssetTreeView)
            {
                // Tree view with additional safety
                if (inJobAction)
                {
                    ImGui::TextDisabled("Loading assets, please wait...");
                }
                else if (!m_assetTreeRoot.children.empty())
                {
                    try {
                        for (auto& categoryNode : m_assetTreeRoot.children)
                        {
                            RenderAssetTreeNode(categoryNode);
                        }
                    } catch (...) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error displaying assets. Please click Refresh.");
                    }
                }
                else
                {
                    ImGui::TextDisabled("No assets loaded. Use File -> Open to load a PAK file.");
                    ImGui::Spacing();
                    if (ImGui::Button("Load PAK File"))
                    {
                        // Trigger file open dialog
                        if (!inJobAction)
                        {
                            m_selectedAssets.clear();
                            g_assetData.ClearAssetData();
                            m_assetTreeRoot.children.clear();
                            CThread(HandleOpenFileDialog, g_dxHandler->GetWindowHandle()).detach();
                        }
                    }
                }
            }
            else if (m_showAssetTableView)
            {
                // Table view - show all assets
                if (inJobAction)
                {
                    ImGui::TextDisabled("Loading assets, please wait...");
                }
                else
                {
                    std::vector<CAsset*> allAssets;
                    for (const auto& lookup : g_assetData.v_assets)
                    {
                        if (lookup.m_asset != nullptr)
                        {
                            allAssets.push_back(lookup.m_asset);
                        }
                    }
                    RenderAssetTable(allAssets);
                }
            }
        }
        ImGui::EndChild();
        
        ImGui::PopStyleVar();
    }

    void LayoutManager::RenderAssetInspector()
    {
        ImGui::Text("Asset Inspector");
        
        if (ImGui::SmallButton("Refresh"))
        {
            // Force refresh inspector
        }
        
        ImGui::Separator();
        
        if (m_selectedAssets.empty())
        {
            ImGui::TextDisabled("No asset selected");
            ImGui::Spacing();
            ImGui::TextWrapped("Select an asset from the Asset Browser to view its details here.");
            return;
        }
        
        CAsset* primaryAsset = m_selectedAssets[0];
        
        // Asset basic info
        DrawSeparatorWithText("Basic Information");
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        static char assetNameBuffer[512];
        strncpy_s(assetNameBuffer, primaryAsset->GetAssetName().c_str(), sizeof(assetNameBuffer) - 1);
        if (ImGui::InputText("##AssetName", assetNameBuffer, sizeof(assetNameBuffer), ImGuiInputTextFlags_ReadOnly))
        {
            // Read-only for now
        }
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("GUID:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        static char guidBuffer[32];
        std::string guidStr = std::format("{:016X}", primaryAsset->GetAssetGUID());
        strncpy_s(guidBuffer, guidStr.c_str(), sizeof(guidBuffer) - 1);
        ImGui::InputText("##AssetGUID", guidBuffer, sizeof(guidBuffer), ImGuiInputTextFlags_ReadOnly);
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Type:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        static char typeBuffer[128];
        uint32_t assetType = primaryAsset->GetAssetType();
        char typeBytes[5] = {0};
        memcpy(typeBytes, &assetType, 4);
        for (int i = 0; i < 4; ++i) {
            if (typeBytes[i] < 32 || typeBytes[i] > 126) typeBytes[i] = '?';
        }
        std::string typeStr = std::format("Type: {}", typeBytes);
        strncpy_s(typeBuffer, typeStr.c_str(), sizeof(typeBuffer) - 1);
        ImGui::InputText("##AssetType", typeBuffer, sizeof(typeBuffer), ImGuiInputTextFlags_ReadOnly);
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Container:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        static char containerBuffer[512];
        strncpy_s(containerBuffer, primaryAsset->GetContainerFileName().c_str(), sizeof(containerBuffer) - 1);
        ImGui::InputText("##AssetContainer", containerBuffer, sizeof(containerBuffer), ImGuiInputTextFlags_ReadOnly);
        
        // Version info
        const auto& version = primaryAsset->GetAssetVersion();
        if (version.majorVer > 0 || version.minorVer > 0)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Version:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            static char versionBuffer[64];
            strncpy_s(versionBuffer, version.ToString().c_str(), sizeof(versionBuffer) - 1);
            ImGui::InputText("##AssetVersion", versionBuffer, sizeof(versionBuffer), ImGuiInputTextFlags_ReadOnly);
        }
        
        ImGui::Spacing();
        
        // Export status
        DrawSeparatorWithText("Export Status");
        
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Exported:");
        ImGui::SameLine();
        bool exported = primaryAsset->GetExportedStatus();
        if (exported)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("Yes");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("No");
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        
        // Multiple selection info
        if (m_selectedAssets.size() > 1)
        {
            DrawSeparatorWithText("Multiple Selection");
            ImGui::Text("Selected Assets: %zu", m_selectedAssets.size());
            
            // Count by type
            std::unordered_map<uint32_t, int> typeCounts;
            for (CAsset* asset : m_selectedAssets)
            {
                typeCounts[asset->GetAssetType()]++;
            }
            
            ImGui::Text("Asset Types:");
            for (const auto& [type, count] : typeCounts)
            {
                char typeStr2[5] = {0};
                memcpy(typeStr2, &type, 4);
                for (int i = 0; i < 4; ++i) {
                    if (typeStr2[i] < 32 || typeStr2[i] > 126) typeStr2[i] = '?';
                }
                ImGui::BulletText("%s: %d", typeStr2, count);
            }
        }
        
        ImGui::Spacing();
        
        // Action buttons
        DrawSeparatorWithText("Actions");
        
        if (ImGui::Button("Export Selected", ImVec2(-1, 0)))
        {
            // TODO: Export selected assets
        }
        
        if (ImGui::Button("Copy Name to Clipboard", ImVec2(-1, 0)))
        {
            ImGui::SetClipboardText(primaryAsset->GetAssetName().c_str());
        }
        
        if (ImGui::Button("Copy GUID to Clipboard", ImVec2(-1, 0)))
        {
            std::string guidStr2 = std::format("{:016X}", primaryAsset->GetAssetGUID());
            ImGui::SetClipboardText(guidStr2.c_str());
        }
    }

    void LayoutManager::RenderViewport3D()
    {
        // Check if we have a texture selected
        bool hasTextureSelected = false;
        CAsset* textureAsset = nullptr;
        
        if (!m_selectedAssets.empty()) {
            CAsset* firstAsset = m_selectedAssets[0];
            if (firstAsset) {
                try {
                    uint32_t assetType = firstAsset->GetAssetType();
                    // Check for texture types: 'rtxt' (0x72747874) or 'txtr' (0x74787472)
                    if (assetType == 0x72747874 || assetType == 0x74787472) { 
                        hasTextureSelected = true;
                        textureAsset = firstAsset;
                    }
                } catch (...) {
                    // Asset type couldn't be read
                }
            }
        }
        
        if (hasTextureSelected && textureAsset) {
            // Show texture viewer instead of 3D viewport
            RenderTextureViewer(textureAsset);
            return;
        }
        
        // Normal 3D viewport for non-texture assets
        RenderModel3DViewport();
        
        // Main viewport area
        const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        if (viewportSize.x > 0 && viewportSize.y > 0)
        {
            // Create a child window for the 3D content
            if (ImGui::BeginChild("Viewport3DContent", viewportSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
                const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
                
                // Draw background
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                const ImU32 bgColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
                drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), bgColor);
                
                // Draw grid
                const ImU32 gridColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                const float gridStep = 50.0f;
                
                for (float x = fmod(canvasPos.x, gridStep); x < canvasPos.x + canvasSize.x; x += gridStep)
                {
                    drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + canvasSize.y), gridColor);
                }
                for (float y = fmod(canvasPos.y, gridStep); y < canvasPos.y + canvasSize.y; y += gridStep)
                {
                    drawList->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x + canvasSize.x, y), gridColor);
                }
                
                // Center crosshair
                const ImVec2 center = ImVec2(canvasPos.x + canvasSize.x * 0.5f, canvasPos.y + canvasSize.y * 0.5f);
                const ImU32 crosshairColor = ImGui::ColorConvertFloat4ToU32(GetAccentColor());
                drawList->AddLine(ImVec2(center.x - 10, center.y), ImVec2(center.x + 10, center.y), crosshairColor, 2.0f);
                drawList->AddLine(ImVec2(center.x, center.y - 10), ImVec2(center.x, center.y + 10), crosshairColor, 2.0f);
                
                // Mouse interaction for camera control
                if (ImGui::IsWindowHovered())
                {
                    //const ImGuiIO& io = ImGui::GetIO();
                    
                    // Instructions text
                    const char* instructions = "Right-click and drag to orbit\nMiddle-click to pan\nScroll to zoom";
                    const ImVec2 textSize = ImGui::CalcTextSize(instructions);
                    const ImVec2 textPos = ImVec2(canvasPos.x + 10, canvasPos.y + 10);
                    
                    drawList->AddRectFilled(textPos, ImVec2(textPos.x + textSize.x + 10, textPos.y + textSize.y + 10),
                                          ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.7f)));
                    drawList->AddText(ImVec2(textPos.x + 5, textPos.y + 5), 
                                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f)), instructions);
                }
                
                // Render 3D content here
                // This would normally render the actual 3D scene
                if (!m_selectedAssets.empty())
                {
                    // Show selected asset info in viewport
                    CAsset* asset = m_selectedAssets[0];
                    char typeStr[5] = {0};
                    uint32_t assetType = asset->GetAssetType();
                    memcpy(typeStr, &assetType, 4);
                    for (int i = 0; i < 4; ++i) {
                        if (typeStr[i] < 32 || typeStr[i] > 126) typeStr[i] = '?';
                    }
                    std::string modelInfo = std::format("Previewing: {}\nType: {}", 
                                                       asset->GetAssetName(), 
                                                       typeStr);
                    
                    const ImVec2 infoTextSize = ImGui::CalcTextSize(modelInfo.c_str());
                    const ImVec2 infoPos = ImVec2(center.x - infoTextSize.x * 0.5f, center.y + 50);
                    
                    drawList->AddRectFilled(ImVec2(infoPos.x - 5, infoPos.y - 5), 
                                          ImVec2(infoPos.x + infoTextSize.x + 5, infoPos.y + infoTextSize.y + 5),
                                          ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.8f)));
                    drawList->AddText(infoPos, ImGui::ColorConvertFloat4ToU32(GetAccentColor()), modelInfo.c_str());
                }
                else
                {
                    // No asset selected message
                    const char* noAssetText = "No 3D asset selected\n\nSelect a model, texture, or other\n3D asset to preview it here";
                    const ImVec2 noAssetTextSize = ImGui::CalcTextSize(noAssetText);
                    const ImVec2 noAssetPos = ImVec2(center.x - noAssetTextSize.x * 0.5f, center.y - noAssetTextSize.y * 0.5f);
                    
                    drawList->AddText(noAssetPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), noAssetText);
                }
                
                // Invisible button to capture mouse input for the entire viewport
                ImGui::SetCursorScreenPos(canvasPos);
                ImGui::InvisibleButton("ViewportInteraction", canvasSize);
            }
            ImGui::EndChild();
        }
    }

    void LayoutManager::RenderAssetPreview()
    {
        ImGui::Text("Asset Preview");
        ImGui::Separator();
        
        if (m_selectedAssets.empty())
        {
            ImGui::TextDisabled("No asset selected");
            ImGui::Spacing();
            ImGui::TextWrapped("Select an asset from the Asset Browser to preview it here.");
            return;
        }
        
        CAsset* asset = m_selectedAssets[0];
        static CAsset* prevPreviewAsset = nullptr;
        
        // Check if we have a preview function for this asset type
        uint32_t assetType = asset->GetAssetType();
        auto it = g_assetData.m_assetTypeBindings.find(assetType);
        
        // TEXTURE PREVIEW DISABLED - CAUSING CRASHES
        // TODO: Investigate texture preview crashes and re-enable safely
        //if (false && it != g_assetData.m_assetTypeBindings.end() && it->second.previewFunc)
        //{
            // Preview functions completely disabled until crash source is identified
        //}
        //else
        {
            // Safe asset info display with minimal operations
            ImGui::Text("Asset Information");
            ImGui::Separator();
            
            try {
                // Basic asset name (safest operation)
                std::string assetName = asset->GetAssetName();
                ImGui::Text("Name: %s", assetName.empty() ? "Unknown" : assetName.c_str());
            } catch (...) {
                ImGui::Text("Name: <Error reading name>");
            }
            
            try {
                // Asset type with safety
                char typeStr[5] = {0};
                memcpy(typeStr, &assetType, 4);
                for (int i = 0; i < 4; ++i) {
                    if (typeStr[i] < 32 || typeStr[i] > 126) typeStr[i] = '?';
                }
                ImGui::Text("Type: %s", typeStr);
            } catch (...) {
                ImGui::Text("Type: <Error reading type>");
            }
            
            try {
                // GUID with safety
                uint64_t guid = asset->GetAssetGUID();
                ImGui::Text("GUID: %016llX", guid);
            } catch (...) {
                ImGui::Text("GUID: <Error reading GUID>");
            }
            
            try {
                // Container with safety
                std::string container = asset->GetContainerFileName();
                ImGui::Text("Container: %s", container.empty() ? "Unknown" : container.c_str());
            } catch (...) {
                ImGui::Text("Container: <Error reading container>");
            }
            
            // Safe texture preview (no DirectX operations)
            if (assetType == 0x72747874) { // 'rtxt'
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Texture Information");
                
                try {
                    CPakAsset* pakAsset = static_cast<CPakAsset*>(asset);
                    if (pakAsset && pakAsset->extraData()) {
                        // Use the actual TextureAsset class (properly included)
                        TextureAsset* txtrAsset = reinterpret_cast<TextureAsset*>(pakAsset->extraData());
                        if (txtrAsset) {
                            ImGui::Text("Dimensions: %dx%d", txtrAsset->width, txtrAsset->height);
                            ImGui::Text("Mip Levels: %d", txtrAsset->totalMipLevels);
                            ImGui::Text("Array Size: %d", txtrAsset->arraySize);
                            ImGui::Text("Format: %u", static_cast<uint32_t>(txtrAsset->imgFormat));
                            
                            if (txtrAsset->name) {
                                ImGui::Text("Internal Name: %s", txtrAsset->name);
                            }
                        }
                    }
                } catch (...) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Could not read texture data safely");
                }
            }
        }
    }

    void LayoutManager::RenderProperties()
    {
        ImGui::Text("Properties");
        ImGui::Separator();
        
        // Application settings and properties
        DrawSeparatorWithText("Display Settings");
        
        static bool enableVSync = true;
        ImGui::Checkbox("Enable VSync", &enableVSync);
        
        static int targetFPS = 60;
        ImGui::SliderInt("Target FPS", &targetFPS, 30, 144);
        
        DrawSeparatorWithText("Asset Browser");
        
        ImGui::Checkbox("Tree View by Default", &m_showAssetTreeView);
        
        static bool showFileExtensions = true;
        ImGui::Checkbox("Show File Extensions", &showFileExtensions);
        
        static bool autoRefresh = false;
        ImGui::Checkbox("Auto Refresh", &autoRefresh);
        
        DrawSeparatorWithText("Viewport");
        
        static bool showGrid = true;
        ImGui::Checkbox("Show Grid", &showGrid);
        
        static bool showAxes = true;
        ImGui::Checkbox("Show Axes", &showAxes);
        
        static float backgroundColor[3] = {0.05f, 0.05f, 0.05f};
        ImGui::ColorEdit3("Background Color", backgroundColor);
        
        DrawSeparatorWithText("Export Settings");
        
        static bool preserveHierarchy = true;
        ImGui::Checkbox("Preserve Hierarchy", &preserveHierarchy);
        
        static bool embedTextures = false;
        ImGui::Checkbox("Embed Textures", &embedTextures);
        
        static int compressionLevel = 5;
        ImGui::SliderInt("Compression Level", &compressionLevel, 0, 9);
        
        ImGui::Spacing();
        
        if (ImGui::Button("Reset to Defaults", ImVec2(-1, 0)))
        {
            // Reset all settings to defaults
            enableVSync = true;
            targetFPS = 60;
            showFileExtensions = true;
            autoRefresh = false;
            showGrid = true;
            showAxes = true;
            backgroundColor[0] = backgroundColor[1] = backgroundColor[2] = 0.05f;
            preserveHierarchy = true;
            embedTextures = false;
            compressionLevel = 5;
        }
    }

    void LayoutManager::RenderConsole()
    {
        ImGui::Text("Console");
        
        // Console controls
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear"))
        {
            // Clear console
        }
        ImGui::SameLine();
        
        static bool autoScroll = true;
        ImGui::Checkbox("Auto Scroll", &autoScroll);
        
        ImGui::Separator();
        
        // Console output area
        if (ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true))
        {
            // Placeholder console messages
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::TextUnformatted("[INFO] Modern UI Layout initialized successfully");
            ImGui::PopStyleColor();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::TextUnformatted("[SUCCESS] Asset browser tree view enabled");
            ImGui::PopStyleColor();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
            ImGui::TextUnformatted("[WARNING] No assets loaded - open a PAK file to begin");
            ImGui::PopStyleColor();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::TextUnformatted("[INFO] Ready for asset extraction and preview");
            ImGui::PopStyleColor();
            
            // Auto-scroll to bottom
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        
        // Command input
        static char commandBuffer[256] = "";
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputTextWithHint("##ConsoleInput", "Enter command...", commandBuffer, sizeof(commandBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            // Process command
            if (strlen(commandBuffer) > 0)
            {
                // Add command to console output (placeholder)
                // In a real implementation, this would execute the command
                memset(commandBuffer, 0, sizeof(commandBuffer));
            }
        }
    }

    void LayoutManager::BuildAssetTree()
    {
        // Clear existing tree
        m_assetTreeRoot.children.clear();
        
        // Don't build tree if loading
        if (inJobAction) {
            return;
        }
        
        if (g_assetData.v_assets.empty()) {
            AssetTreeNode& emptyNode = m_assetTreeRoot.children.emplace_back("No assets loaded", true);
            emptyNode.isExpanded = false;
            return;
        }
        
        // Create hierarchical tree: All Assets -> Asset Types -> Individual Assets
        AssetTreeNode& allNode = m_assetTreeRoot.children.emplace_back("All Assets", true);
        allNode.isExpanded = true;
        
        // Map to organize assets by type
        std::unordered_map<uint32_t, std::vector<CAsset*>> assetsByType;
        std::unordered_map<uint32_t, std::string> typeNames;
        
        // Group all assets by type
        for (const auto& lookup : g_assetData.v_assets) {
            if (!lookup.m_asset) continue;
            
            CAsset* asset = lookup.m_asset;
            uint32_t assetType = 0;
            
            try {
                assetType = asset->GetAssetType();
            } catch (...) {
                assetType = 0; // Unknown type
            }
            
            assetsByType[assetType].push_back(asset);
            
            // Create readable type name if not already done
            if (typeNames.find(assetType) == typeNames.end()) {
                char typeStr[5] = {0};
                memcpy(typeStr, &assetType, 4);
                
                // Sanitize characters
                for (int i = 0; i < 4; ++i) {
                    if (typeStr[i] < 32 || typeStr[i] > 126) {
                        typeStr[i] = '?';
                    }
                }
                
                // Create user-friendly names for known types
                if (assetType == 0x72747874) { // 'rtxt'
                    typeNames[assetType] = "Textures";
                } else if (assetType == 0x6C646D5F) { // '_ldm'
                    typeNames[assetType] = "Models";
                } else if (assetType == 0x6C74616D) { // 'ltam'
                    typeNames[assetType] = "Materials";
                } else if (assetType == 0x73697573) { // 'sius'
                    typeNames[assetType] = "Audio";
                } else if (assetType == 0x61676972) { // 'gira'
                    typeNames[assetType] = "Animation Rigs";
                } else if (assetType == 0x61736571) { // 'qesa'
                    typeNames[assetType] = "Animation Sequences";
                } else if (assetType == 0x72646873) { // 'rdhs'
                    typeNames[assetType] = "Shaders";
                } else if (assetType == 0x676D6975) { // 'gmiu'
                    typeNames[assetType] = "UI Images";
                } else {
                    typeNames[assetType] = std::format("Type [{}]", typeStr);
                }
            }
        }
        
        // Create category nodes under "All Assets"
        for (const auto& [type, assets] : assetsByType) {
            if (assets.empty()) continue;
            
            AssetTreeNode& typeNode = allNode.children.emplace_back(typeNames[type], true);
            typeNode.isExpanded = false;
            typeNode.assets = assets; // Copy all assets of this type
        }
        
        // Sort type categories alphabetically
        std::sort(allNode.children.begin(), allNode.children.end(),
                  [](const AssetTreeNode& a, const AssetTreeNode& b) {
                      return a.name < b.name;
                  });
    }

    void LayoutManager::RenderAssetTreeNode(AssetTreeNode& node)
    {
        // Check if this node has child categories or assets
        bool hasChildren = !node.children.empty();
        bool hasAssets = !node.assets.empty();
        
        if (hasChildren)
        {
            // This is a category node (like "All Assets" or "Textures")
            std::string nodeLabel = std::format("{} ({} types)", node.name, node.children.size());
            bool nodeOpen = ImGui::TreeNode(nodeLabel.c_str());
            
            if (nodeOpen)
            {
                // Render child category nodes
                for (auto& child : node.children)
                {
                    RenderAssetTreeNode(child);
                }
                ImGui::TreePop();
            }
        }
        else if (hasAssets)
        {
            // This is an asset type category (like "Textures" with actual assets)
            std::string nodeLabel = std::format("{} ({} assets)", node.name, node.assets.size());
            bool nodeOpen = ImGui::TreeNode(nodeLabel.c_str());
            
            if (nodeOpen)
            {
                // Render individual assets
                for (CAsset* asset : node.assets)
                {
                    if (!asset) continue;
                    
                    std::string displayName = "Asset";
                    try {
                        displayName = asset->GetAssetName();
                        if (displayName.empty()) {
                            displayName = "Unnamed Asset";
                        }
                    } catch (...) {
                        displayName = "Invalid Asset";
                    }
                    
                    // Check if this asset is selected
                    bool isSelected = std::find(m_selectedAssets.begin(), m_selectedAssets.end(), asset) != m_selectedAssets.end();
                    
                    if (ImGui::Selectable(displayName.c_str(), isSelected))
                    {
                        // Handle selection with Ctrl for multi-select
                        if (!ImGui::GetIO().KeyCtrl)
                        {
                            m_selectedAssets.clear();
                        }
                        
                        auto it = std::find(m_selectedAssets.begin(), m_selectedAssets.end(), asset);
                        if (it != m_selectedAssets.end())
                        {
                            m_selectedAssets.erase(it); // Deselect
                        }
                        else
                        {
                            m_selectedAssets.push_back(asset); // Select
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
        else
        {
            // Empty node
            ImGui::Text("%s (empty)", node.name.c_str());
        }
    }

    void LayoutManager::RenderAssetTable(const std::vector<CAsset*>& assets)
    {
        // Add some padding and spacing
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 6.0f));
        
        if (ImGui::BeginTable("AssetTable", 4, 
                             ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | 
                             ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV | 
                             ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable |
                             ImGuiTableFlags_SizingStretchProp))
        {
            // Better column setup with more appropriate widths
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableSetupColumn("GUID", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableSetupColumn("Container", ImGuiTableColumnFlags_WidthStretch, 0.3f);
            
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
            
            ImGuiListClipper clipper;
            std::vector<CAsset*> filteredAssets;
            
            // Apply search filter
            if (strlen(m_searchBuffer) > 0)
            {
                std::string searchLower = m_searchBuffer;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                
                for (CAsset* asset : assets)
                {
                    std::string assetLower = asset->GetAssetName();
                    std::transform(assetLower.begin(), assetLower.end(), assetLower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    
                    if (assetLower.find(searchLower) != std::string::npos)
                    {
                        filteredAssets.push_back(asset);
                    }
                }
            }
            else
            {
                filteredAssets = assets;
            }
            
            clipper.Begin(static_cast<int>(filteredAssets.size()));
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    CAsset* asset = filteredAssets[i];
                    
                    ImGui::PushID(asset);
                    ImGui::TableNextRow();
                    
                    // Type column with icon and text
                    if (ImGui::TableSetColumnIndex(0))
                    {
                        char typeStr[5] = {0};
                        uint32_t assetType = asset->GetAssetType();
                        memcpy(typeStr, &assetType, 4);
                         for (int i2 = 0; i2 < 4; ++i2) {
                             if (typeStr[i2] < 32 || typeStr[i2] > 126) typeStr[i2] = '?';
                         }
                        ImGui::Text("Type: %s", typeStr);
                    }
                    
                    // Name column
                    if (ImGui::TableSetColumnIndex(1))
                    {
                        bool isSelected = std::find(m_selectedAssets.begin(), m_selectedAssets.end(), asset) != m_selectedAssets.end();
                        
                        // Add padding for better visual separation
                        ImGui::AlignTextToFramePadding();
                        
                        if (ImGui::Selectable(asset->GetAssetName().c_str(), isSelected, 
                                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
                        {
                            if (!ImGui::GetIO().KeyCtrl)
                            {
                                m_selectedAssets.clear();
                            }
                            
                            auto it = std::find(m_selectedAssets.begin(), m_selectedAssets.end(), asset);
                            if (it != m_selectedAssets.end())
                            {
                                m_selectedAssets.erase(it);
                            }
                            else
                            {
                                m_selectedAssets.push_back(asset);
                            }
                        }
                        
                        // Context menu for asset
                        if (ImGui::BeginPopupContextItem())
                        {
                            if (ImGui::MenuItem("Export Asset"))
                            {
                                // TODO: Export this asset
                            }
                            if (ImGui::MenuItem("Copy Name"))
                            {
                                ImGui::SetClipboardText(asset->GetAssetName().c_str());
                            }
                            if (ImGui::MenuItem("Copy GUID"))
                            {
                                std::string guidStr = std::format("{:016X}", asset->GetAssetGUID());
                                ImGui::SetClipboardText(guidStr.c_str());
                            }
                            ImGui::EndPopup();
                        }
                    }
                    
                    // GUID column
                    if (ImGui::TableSetColumnIndex(2))
                    {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("%016llX", asset->GetAssetGUID());
                    }
                    
                    // Container column
                    if (ImGui::TableSetColumnIndex(3))
                    {
                        ImGui::AlignTextToFramePadding();
                        std::string containerName = asset->GetContainerFileName();
                        
                        // Shorten long container names
                        if (containerName.length() > 30)
                        {
                            containerName = containerName.substr(0, 27) + "...";
                        }
                        
                        ImGui::Text("%s", containerName.c_str());
                        
                        // Tooltip with full name
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("%s", asset->GetContainerFileName().c_str());
                        }
                    }
                    
                    ImGui::PopID();
                }
            }
            
            ImGui::EndTable();
        }
        
        ImGui::PopStyleVar(); // CellPadding
    }

    // GetAssetTypeIcon function removed to prevent crashes from emoji rendering
    // Using simple text display instead of icons
    
    void LayoutManager::RenderSafeTexturePreview(void* pakAssetPtr)
    {
        try {
            CPakAsset* pakAsset = static_cast<CPakAsset*>(pakAssetPtr);
            if (!pakAsset || !pakAsset->extraData()) {
                ImGui::Text("Invalid texture data");
                return;
            }
            
            // Safe approach - just show detailed texture information without DirectX operations
            struct TextureAsset {
                uint64_t guid;
                char* name;
                uint32_t imgFormat;
                uint16_t width;
                uint16_t height;
                uint8_t arraySize;
                uint8_t totalMipLevels;
                // Don't try to access more complex data that might cause crashes
            };
            
            TextureAsset* txtrAsset = reinterpret_cast<TextureAsset*>(pakAsset->extraData());
            if (!txtrAsset) {
                ImGui::Text("Could not read texture asset");
                return;
            }
            
            ImGui::Text("📋 Detailed Texture Information");
            ImGui::Separator();
            
            ImGui::Text("Dimensions: %d x %d pixels", txtrAsset->width, txtrAsset->height);
            ImGui::Text("Format ID: %u", txtrAsset->imgFormat);
            ImGui::Text("Mip Levels: %d", txtrAsset->totalMipLevels);
            ImGui::Text("Array Size: %d", txtrAsset->arraySize);
            
            if (txtrAsset->name) {
                ImGui::Text("Internal Name: %s", txtrAsset->name);
            }
            
            ImGui::Spacing();
            
            // Calculate texture size info
            uint64_t pixelCount = (uint64_t)txtrAsset->width * txtrAsset->height;
            ImGui::Text("Total Pixels: %llu", pixelCount);
            
            // Try to identify common formats
            std::string formatName = "Unknown";
            uint32_t formatId = static_cast<uint32_t>(txtrAsset->imgFormat);
            switch (formatId) {
                case 28: formatName = "RGBA8"; break;
                case 71: formatName = "BC1/DXT1"; break;
                case 74: formatName = "BC2/DXT3"; break;
                case 77: formatName = "BC3/DXT5"; break;
                case 80: formatName = "BC4"; break;
                case 83: formatName = "BC5"; break;
                case 98: formatName = "BC7"; break;
                default: formatName = std::format("Format {}", formatId); break;
            }
            ImGui::Text("Format: %s", formatName.c_str());
            
            ImGui::Spacing();
            
            // Try to render the actual texture image (minimal approach)
            try {
                RenderMinimalTextureImage(pakAsset, txtrAsset);
            } catch (...) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Could not display texture image");
                ImGui::Text("Format may be unsupported or corrupted");
            }
            
        } catch (...) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error reading texture details");
        }
    }
    
    void LayoutManager::RenderMinimalTextureImage(void* pakAssetPtr, void* txtrAssetPtr)
    {
        // Very minimal texture rendering - only try to show the highest quality mip
        CPakAsset* pakAsset = static_cast<CPakAsset*>(pakAssetPtr);
        
        // Use the actual TextureAsset class
        TextureAsset* txtrAsset = static_cast<TextureAsset*>(txtrAssetPtr);
        
        // Static cache for textures to avoid recreating them
        static std::unordered_map<void*, std::shared_ptr<void>> textureCache;
        static void* lastAsset = nullptr;
        
        ImGui::Text("🖼️ Texture Image:");
        
        // Try to create and display the actual texture
        static std::unordered_map<void*, std::shared_ptr<CTexture>> renderedTextures;
        
        auto textureIt = renderedTextures.find(pakAsset);
        std::shared_ptr<CTexture> displayTexture = nullptr;
        
        if (textureIt == renderedTextures.end()) {
            // First time rendering this texture - try to create it
            try {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "Loading texture...");
                
                // Check if we have mip data
                if (txtrAsset->mipArray.empty()) {
                    throw std::runtime_error("No mip data available");
                }
                
                // Find the highest quality mip (largest resolution)
                const TextureMip_t* bestMip = nullptr;
                for (const auto& mip : txtrAsset->mipArray) {
                    if (mip.isLoaded && (!bestMip || (mip.width >= bestMip->width && mip.height >= bestMip->height))) {
                        bestMip = &mip;
                    }
                }
                
                if (!bestMip) {
                    throw std::runtime_error("No loaded mips available");
                }
                
                ImGui::Text("Found mip: %dx%d", bestMip->width, bestMip->height);
                
                // Get the DXGI format
                //extern DXGI_FORMAT s_PakToDxgiFormat[]; // Declare the external array
                DXGI_FORMAT dxgiFormat = s_PakToDxgiFormat[static_cast<uint32_t>(txtrAsset->imgFormat)];
                
                if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
                    throw std::runtime_error("Unsupported texture format");
                }
                
                // Try to create the texture using the existing safe function
                displayTexture = CreateTextureFromMip(pakAsset, bestMip, dxgiFormat, 0);
                
                if (displayTexture) {
                    renderedTextures[pakAsset] = displayTexture;
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Texture loaded successfully!");
                } else {
                    throw std::runtime_error("CreateTextureFromMip returned null");
                }
                
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Failed to load texture: %s", e.what());
                renderedTextures[pakAsset] = nullptr; // Cache the failure
            } catch (...) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Failed to load texture: Unknown error");
                renderedTextures[pakAsset] = nullptr; // Cache the failure
            }
        } else {
            displayTexture = textureIt->second;
        }
        
        // Display the texture if we have one
        if (displayTexture) {
            ImGui::Spacing();
            ImGui::Text("🖼️ Texture Preview:");
            
            // Get the texture SRV for rendering
            void* srv = displayTexture->GetSRV();
            if (srv) {
                // Scale down large textures for preview
                float displayWidth = std::min((float)displayTexture->GetWidth(), 512.0f);
                float displayHeight = std::min((float)displayTexture->GetHeight(), 512.0f);
                
                // Maintain aspect ratio
                float aspectRatio = (float)displayTexture->GetWidth() / (float)displayTexture->GetHeight();
                if (displayWidth / displayHeight > aspectRatio) {
                    displayWidth = displayHeight * aspectRatio;
                } else {
                    displayHeight = displayWidth / aspectRatio;
                }
                
                // Render the actual texture!
                ImGui::Image(srv, ImVec2(displayWidth, displayHeight));
                
                ImGui::Text("Displaying: %dx%d (scaled from %dx%d)", 
                    (int)displayWidth, (int)displayHeight,
                    displayTexture->GetWidth(), displayTexture->GetHeight());
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Texture loaded but SRV is null");
            }
        }
    }
    
    void LayoutManager::RenderTextureViewer(CAsset* textureAsset)
    {
        // Dedicated texture viewer panel
        ImGui::Text("🖼️ Texture Viewer");
        ImGui::Separator();
        
        try {
            CPakAsset* pakAsset = static_cast<CPakAsset*>(textureAsset);
            if (!pakAsset || !pakAsset->extraData()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid texture asset");
                return;
            }
            
            TextureAsset* txtrAsset = reinterpret_cast<TextureAsset*>(pakAsset->extraData());
            if (!txtrAsset) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Could not read texture data");
                return;
            }
            
            // Top toolbar with texture info and controls
            ImGui::BeginGroup();
            {
                // Asset name
                ImGui::Text("📄 %s", textureAsset->GetAssetName().c_str());
                
                // Quick info in one line
                ImGui::SameLine();
                ImGui::TextDisabled(" • ");
                ImGui::SameLine();
                
                uint32_t formatId = static_cast<uint32_t>(txtrAsset->imgFormat);
                std::string formatName = "Unknown";
                switch (formatId) {
                    case 28: formatName = "RGBA8"; break;
                    case 71: formatName = "BC1/DXT1"; break;
                    case 74: formatName = "BC2/DXT3"; break;
                    case 77: formatName = "BC3/DXT5"; break;
                    case 80: formatName = "BC4"; break;
                    case 83: formatName = "BC5"; break;
                    case 98: formatName = "BC7"; break;
                    default: formatName = std::format("Format {}", formatId); break;
                }
                
                ImGui::Text("%dx%d • %s • %d mips", 
                    txtrAsset->width, txtrAsset->height, 
                    formatName.c_str(), txtrAsset->totalMipLevels);
            }
            ImGui::EndGroup();
            
            ImGui::Separator();
            
            // Main texture display area
            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            
            // Create render texture and display it
            static std::unordered_map<void*, std::shared_ptr<CTexture>> textureCache;
            static std::unordered_map<void*, float> zoomLevels;
            
            auto textureIt = textureCache.find(pakAsset);
            std::shared_ptr<CTexture> displayTexture = nullptr;
            
            if (textureIt == textureCache.end()) {
                // Create texture for first time
                try {
                    if (!txtrAsset->mipArray.empty()) {
                        // Find best mip
                        const TextureMip_t* bestMip = nullptr;
                        for (const auto& mip : txtrAsset->mipArray) {
                            if (mip.isLoaded && (!bestMip || (mip.width >= bestMip->width && mip.height >= bestMip->height))) {
                                bestMip = &mip;
                            }
                        }
                        
                        if (bestMip) {
                            DXGI_FORMAT dxgiFormat = s_PakToDxgiFormat[static_cast<uint32_t>(txtrAsset->imgFormat)];
                            if (dxgiFormat != DXGI_FORMAT_UNKNOWN) {
                                displayTexture = CreateTextureFromMip(pakAsset, bestMip, dxgiFormat, 0);
                                if (displayTexture) {
                                    textureCache[pakAsset] = displayTexture;
                                    zoomLevels[pakAsset] = 1.0f; // Default zoom
                                }
                            }
                        }
                    }
                } catch (...) {
                    // Failed to create texture
                }
            } else {
                displayTexture = textureIt->second;
            }
            
            if (displayTexture) {
                // Zoom controls
                float& zoom = zoomLevels[pakAsset];
                
                ImGui::Text("Zoom:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                ImGui::SliderFloat("##zoom", &zoom, 0.1f, 4.0f, "%.1fx");
                
                ImGui::SameLine();
                if (ImGui::SmallButton("1:1")) zoom = 1.0f;
                ImGui::SameLine();
                if (ImGui::SmallButton("Fit")) {
                    // Calculate zoom to fit
                    float scaleX = availableSize.x / displayTexture->GetWidth();
                    float scaleY = (availableSize.y - 60) / displayTexture->GetHeight(); // Reserve space for controls
                    zoom = std::min(scaleX, scaleY);
                }
                
                ImGui::Separator();
                
                // Create scrollable texture display
                if (ImGui::BeginChild("TextureDisplay", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) {
                    void* srv = displayTexture->GetSRV();
                    if (srv) {
                        float displayWidth = displayTexture->GetWidth() * zoom;
                        float displayHeight = displayTexture->GetHeight() * zoom;
                        
                        // Center the image if it's smaller than the available space
                        ImVec2 childSize = ImGui::GetContentRegionAvail();
                        if (displayWidth < childSize.x) {
                            ImGui::SetCursorPosX((childSize.x - displayWidth) * 0.5f);
                        }
                        if (displayHeight < childSize.y) {
                            ImGui::SetCursorPosY((childSize.y - displayHeight) * 0.5f);
                        }
                        
                        // Draw the texture
                        ImGui::Image(srv, ImVec2(displayWidth, displayHeight));
                        
                        // Mouse wheel zoom when hovering
                        if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0.0f) {
                            float wheelDelta = ImGui::GetIO().MouseWheel * 0.1f;
                            zoom = std::clamp(zoom + wheelDelta, 0.1f, 4.0f);
                        }
                        
                        // Show pixel info on hover
                        if (ImGui::IsItemHovered()) {
                            ImVec2 mousePos = ImGui::GetMousePos();
                            ImVec2 itemMin = ImGui::GetItemRectMin();
                            ImVec2 relativePos = ImVec2(mousePos.x - itemMin.x, mousePos.y - itemMin.y);
                            
                            int pixelX = (int)(relativePos.x / zoom);
                            int pixelY = (int)(relativePos.y / zoom);
                            
                            if (pixelX >= 0 && pixelX < displayTexture->GetWidth() && 
                                pixelY >= 0 && pixelY < displayTexture->GetHeight()) {
                                ImGui::SetTooltip("Pixel: %d, %d", pixelX, pixelY);
                            }
                        }
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Texture loaded but cannot display");
                    }
                }
                ImGui::EndChild();
                
            } else {
                // Texture failed to load or no texture available
                ImVec2 centerPos = ImVec2(availableSize.x * 0.5f, availableSize.y * 0.5f);
                ImGui::SetCursorPos(ImVec2(centerPos.x - 100, centerPos.y - 50));
                
                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠️ Cannot Display Texture");
                ImGui::Text("This texture format may not be supported");
                ImGui::Text("or the texture data may be corrupted.");
                ImGui::EndGroup();
            }
            
        } catch (...) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error reading texture asset");
        }
    }
    
    void LayoutManager::RenderModel3DViewport()
    {
        // Header with controls
        ImGui::Text("🎮 3D Viewport");
        ImGui::SameLine();
        
        // Check if we have a model selected
        bool hasModelSelected = false;
        CAsset* modelAsset = nullptr;
        void* previewResult = nullptr;
        
        if (!m_selectedAssets.empty()) {
            CAsset* firstAsset = m_selectedAssets[0];
            if (firstAsset) {
                try {
                    uint32_t assetType = firstAsset->GetAssetType();
                    
                    // Show current asset info
                    ImGui::Text("📦 Selected: %s", firstAsset->GetAssetName().c_str());
                    
                    // Check for model types: 'MDL_', 'MDL', 'ARIG', 'ASEQ', 'ASQD', 'ANIR', 'SEQ', 'rmdl'
                    if (assetType == 0x5F6C646D || // 'mdl_'  
                        assetType == 0x006C646D || // 'mdl'
                        assetType == 0x67697261 || // 'arig'
                        assetType == 0x71657361 || // 'aseq'
                        assetType == 0x64717361 || // 'asqd'
                        assetType == 0x72696E61 || // 'anir'
                        assetType == 0x00716573 || // 'seq'
                        assetType == 0x6C646D72) { // 'rmdl'
                        hasModelSelected = true;
                        modelAsset = firstAsset;
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ 3D Model - rendering below");
                    } else {
                        // Debug info for non-model assets
                        char fourccStr[5] = {0};
                        fourccStr[0] = (assetType >> 0) & 0xFF;
                        fourccStr[1] = (assetType >> 8) & 0xFF;
                        fourccStr[2] = (assetType >> 16) & 0xFF;
                        fourccStr[3] = (assetType >> 24) & 0xFF;
                        ImGui::Text("Asset Type: 0x%08X (%s)", assetType, fourccStr);
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Not a recognized model type");
                    }
                } catch (...) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error reading asset type");
                }
            }
        } else {
            // No assets selected - clear model rendering
            previewDrawData = nullptr;
        }
        
        // Camera and viewport controls
        static bool showWireframe = false;
        static bool showBounds = false;
        static float cameraSpeed = 1.0f;
        static bool autoRotate = false;
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
        
        if (ImGui::SmallButton("Reset Camera"))
        {
            if (g_dxHandler && g_dxHandler->GetCamera()) {
                CDXCamera* camera = g_dxHandler->GetCamera();
                camera->position = {0, 0, -10}; // Reset position
                camera->rotation = {0, 0, 0};   // Reset rotation
            }
        }
        ImGui::SameLine();
        
        ImGui::Checkbox("Wireframe", &showWireframe);
        ImGui::SameLine();
        
        ImGui::Checkbox("Bounds", &showBounds);
        ImGui::SameLine();
        
        ImGui::Checkbox("Auto Rotate", &autoRotate);
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(80);
        ImGui::SliderFloat("Speed", &cameraSpeed, 0.1f, 5.0f, "%.1fx");
        
        ImGui::PopStyleVar();
        
        // Show instructions for camera controls
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "3D Camera Controls:\n"
                "• Right Mouse: Look around\n" 
                "• WASD: Move camera\n"
                "• Space/Shift: Up/Down\n"
                "• Ctrl: Move faster"
            );
        }
        
        ImGui::Separator();
        
        // Model preview processing (minimal output)
        if (hasModelSelected && modelAsset) {
            // Try to get the asset type binding and call the preview function
            auto bindingIt = g_assetData.m_assetTypeBindings.find(modelAsset->GetAssetType());
            
            if (bindingIt != g_assetData.m_assetTypeBindings.end() && bindingIt->second.previewFunc) {
                // Use the existing preview system
                static std::unordered_map<CAsset*, bool> firstFrameMap;
                bool isFirstFrame = (firstFrameMap.find(modelAsset) == firstFrameMap.end());
                if (isFirstFrame) {
                    firstFrameMap[modelAsset] = true;
                }
                
                try {
                    previewResult = bindingIt->second.previewFunc(modelAsset, isFirstFrame);
                    if (previewResult) {
                        // Set the global previewDrawData for the main render loop
                        previewDrawData = reinterpret_cast<CDXDrawData*>(previewResult);
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ 3D Model Ready - Look in viewport below");
                    } else {
                        previewDrawData = nullptr;
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠ Model failed to load");
                    }
                } catch (...) {
                    previewDrawData = nullptr;
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "❌ Error loading model");
                }
            } else {
                previewDrawData = nullptr;
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "❌ No preview support for this model type");
            }
            
            ImGui::Separator();
        }
        
        // Main viewport area - render the 3D scene
        const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        if (viewportSize.x > 0 && viewportSize.y > 0) {
            // Create a child window for the 3D viewport
            if (ImGui::BeginChild("3DViewport", viewportSize, true)) {
                
                // Get the current render target from DirectX
                if (g_dxHandler) {
                    
                    // Handle camera movement and input
                    if (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard) {
                        if (g_pInput) {
                            // Update camera movement speed
                            
                            g_PreviewSettings.previewMovementSpeed = cameraSpeed * 50.0f;
                            
                            // Let the existing camera system handle input
                            g_pInput->Frame(ImGui::GetIO().DeltaTime);
                            g_dxHandler->GetCamera()->Move(ImGui::GetIO().DeltaTime);
                        }
                    }
                    
                    // Auto-rotate the model if enabled
                    if (autoRotate && hasModelSelected) {
                        static float autoRotateTime = 0.0f;
                        autoRotateTime += ImGui::GetIO().DeltaTime;
                        // Rotate around Y axis
                        if (g_dxHandler->GetCamera()) {
                            g_dxHandler->GetCamera()->rotation.y = autoRotateTime * 0.5f; // Slow rotation
                        }
                    }
                    
                    // The actual 3D rendering happens in the main render loop
                    // We display the model preview info and controls here
                    if (hasModelSelected) {
                        // Get the viewport dimensions
                        ImVec2 pos = ImGui::GetCursorScreenPos();
                        ImVec2 size = ImGui::GetContentRegionAvail();
                        
                        // Make the 3D viewport transparent to see DirectX rendering underneath
                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        // Don't fill the background - let the 3D scene show through
                        
                        // 3D Viewport border - this area should show the 3D model
                        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 255, 100, 180), 0.0f, 0, 3.0f);
                        
                        // Status overlay
                        ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
                        if (!previewResult) {
                            drawList->AddText(ImVec2(center.x - 80, center.y), IM_COL32(255, 200, 0, 255), "⏳ Loading 3D Model...");
                        } else {
                            // Model should be rendering here!
                            drawList->AddText(ImVec2(center.x - 120, center.y - 30), IM_COL32(100, 255, 100, 255), "🎮 3D MODEL SHOULD BE HERE");
                            drawList->AddText(ImVec2(center.x - 100, center.y - 10), IM_COL32(200, 200, 200, 255), "Right-click to activate camera");
                            drawList->AddText(ImVec2(center.x - 80, center.y + 10), IM_COL32(150, 150, 150, 255), "WASD to move, mouse to look");
                        }
                        
                        // Camera status indicator
                        if (g_pInput && g_pInput->mouseCaptured) {
                            ImVec2 statusPos = ImVec2(pos.x + 10, pos.y + 10);
                            drawList->AddRectFilled(statusPos, ImVec2(statusPos.x + 120, statusPos.y + 20), IM_COL32(0, 0, 0, 150));
                            drawList->AddText(ImVec2(statusPos.x + 5, statusPos.y + 5), IM_COL32(0, 255, 0, 255), "🎮 Camera Active");
                        }
                        
                        // Move cursor past the rendered area
                        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
                        
                    } else {
                        // No model selected - show placeholder
                        ImVec2 center = ImVec2(viewportSize.x * 0.5f, viewportSize.y * 0.5f);
                        ImGui::SetCursorPos(ImVec2(center.x - 100, center.y - 30));
                        
                        ImGui::BeginGroup();
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "🎯 Select a 3D Model");
                        ImGui::Text("Choose a model asset to view it here");
                        ImGui::TextDisabled("Supported: .mdl, .arig, .aseq files");
                        ImGui::EndGroup();
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DirectX handler not available");
                }
            }
            ImGui::EndChild();
        }
    }
}

