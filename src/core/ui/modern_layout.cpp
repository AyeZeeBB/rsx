#include "pch.h"
#include "modern_layout.h"
#include <game/asset.h>
#include <core/render.h>
#include <core/window.h>
#include <core/utils/utils_general.h>
#include <thirdparty/imgui/misc/imgui_utility.h>
#include <algorithm>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <cctype>
#include <format>
#include <chrono>
#include <core/render/dx.h>
#include <core/filehandling/load.h>
#include <game/rtech/cpakfile.h>
#include <game/rtech/assets/texture.h>
#include <game/rtech/assets/material.h>
#include <core/mdl/modeldata.h>
#include <core/input/input.h>
#include <core/render/dxshader.h>

// External declarations
extern CDXParentHandler* g_dxHandler;
extern CInput* g_pInput;
extern std::atomic<bool> inJobAction;

// Global for 3D model rendering - this is used by the main render loop
extern CDXDrawData* previewDrawData;

// Preview settings structure
extern PreviewSettings_t g_PreviewSettings;
extern CDXParentHandler* g_dxHandler;

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
        for (int i = 0; i < 8; ++i)  // Updated for MaterialPreview
        {
            m_panelVisible[i] = true;
        }
        m_panelVisible[static_cast<int>(PanelType::Console)] = false;
        
        m_assetTreeRoot = AssetTreeNode("Assets", "", true);
    }
    
    LayoutManager::~LayoutManager()
    {
        // Clean up render target resources
        DestroyModelViewerRenderTarget();
        DestroyMaterialSphereRenderTarget();
        DestroySphereGeometry();
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
                ImGui::MenuItem("3D Model Viewer (Center)", nullptr, &m_panelVisible[static_cast<int>(PanelType::ModelViewer3D)]);
                ImGui::MenuItem("Asset Preview", nullptr, &m_panelVisible[static_cast<int>(PanelType::AssetPreview)]);
                ImGui::MenuItem("Material Preview", nullptr, &m_panelVisible[static_cast<int>(PanelType::MaterialPreview)]);
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
                // Left panel with padding
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
                    // Render Godot-style tabbed center panel
                    RenderTabbedCenterPanel();
                }
                ImGui::EndChild();

                // Right panel
                if (m_panelVisible[static_cast<int>(PanelType::AssetInspector)] || 
                    m_panelVisible[static_cast<int>(PanelType::Properties)])
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
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
        
        // Auto-refresh when PAK loading completes
        bool currentJobActionState = inJobAction.load();
        if (m_lastJobActionState && !currentJobActionState) {
            // PAK loading just finished - trigger tree rebuild
            m_treeNeedsRebuild = true;
        }
        m_lastJobActionState = currentJobActionState;
        
        // Rebuild tree if needed
        if (m_treeNeedsRebuild) {
            RefreshAssetTree();
            m_treeNeedsRebuild = false;
        }
        
        // Add manual padding with spacing and indentation
        ImGui::Spacing();
        ImGui::Indent(8.0f);
        
        // Header with title and controls
        ImGui::Text("Asset Browser");
        
        // Show loading indicator when PAKs are loading
        if (currentJobActionState) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Loading...");
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
        
        ImGui::Unindent(8.0f);
        ImGui::Spacing();
        ImGui::PopStyleVar();
    }

    void LayoutManager::RenderAssetInspector()
    {
        // Add manual padding for consistent layout
        ImGui::Spacing();
        ImGui::Indent(8.0f);

        // Modern header with better styling
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));

        // Header section
        ImGui::Text("Asset Inspector");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 60);
        if (ImGui::SmallButton("Refresh"))
        {
            // Force refresh inspector
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (m_selectedAssets.empty())
        {
            // Empty state with better visual design
            ImVec2 center = ImVec2(
                ImGui::GetContentRegionAvail().x * 0.5f - 80,
                ImGui::GetContentRegionAvail().y * 0.4f
            );
            ImGui::SetCursorPos(center);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text("No asset selected");
            ImGui::PopStyleColor();

            ImGui::SetCursorPosX(center.x - 30);
            ImGui::SetCursorPosY(center.y + 25);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::TextWrapped("Select an asset to view details");
            ImGui::PopStyleColor();

            ImGui::PopStyleVar();
            return;
        }

        CAsset* primaryAsset = m_selectedAssets[0];

        // Get asset info
        std::string assetName = primaryAsset->GetAssetName();
        std::string guidStr = std::format("{:016X}", primaryAsset->GetAssetGUID());
        uint32_t assetType = primaryAsset->GetAssetType();
        char typeBytes[5] = {0};
        memcpy(typeBytes, &assetType, 4);
        for (int i = 0; i < 4; ++i) {
            if (typeBytes[i] < 32 || typeBytes[i] > 126) typeBytes[i] = '?';
        }
        std::string containerName = primaryAsset->GetContainerFileName();

        // Extract filename from full path for cleaner display
        std::string fileName = assetName;
        size_t lastSlash = assetName.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            fileName = assetName.substr(lastSlash + 1);
        }

        // Modern card-style layout
        ImGui::BeginChild("AssetCard", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);

        // Asset header with icon and name
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font but can be changed to bold later
        ImGui::Text("Name: %s", fileName.c_str());
        ImGui::PopFont();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::Text("Type: %s", typeBytes);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Property section with better layout
        if (ImGui::BeginTable("AssetProperties", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            // File Name row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Name");
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-1);
            std::string nameDisplay = fileName;
            ImGui::InputText("##AssetName", nameDisplay.data(), nameDisplay.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            // GUID row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "GUID");
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-1);
            ImGui::InputText("##AssetGUID", guidStr.data(), guidStr.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            // Container row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Package");
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-1);
            // Show just the filename of the container for cleaner look
            std::string containerFile = containerName;
            size_t containerSlash = containerName.find_last_of("/\\");
            if (containerSlash != std::string::npos) {
                containerFile = containerName.substr(containerSlash + 1);
            }
            ImGui::InputText("##AssetContainer", containerFile.data(), containerFile.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            // Version info (if available)
            const auto& version = primaryAsset->GetAssetVersion();
            if (version.majorVer > 0 || version.minorVer > 0)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Version");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                std::string versionStr = version.ToString();
                ImGui::InputText("##AssetVersion", versionStr.data(), versionStr.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
            }

            ImGui::EndTable();
        }
        
        ImGui::Spacing();
        
        // Asset-specific details
        const char* assetTypeBytes = reinterpret_cast<const char*>(&assetType);
        if (strncmp(assetTypeBytes, "matl", 4) == 0) {
            // Cast to CPakAsset to access extraData()
            CPakAsset* pakAsset = static_cast<CPakAsset*>(primaryAsset);
            if (pakAsset && pakAsset->extraData()) {
                DrawSeparatorWithText("Material Details");
                
                const MaterialAsset* material = reinterpret_cast<const MaterialAsset*>(pakAsset->extraData());
            
                // Material shader info
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Shader:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1);
                static char shaderBuffer[256];
                std::string shaderName = material->shaderSetAsset ? material->shaderSetAsset->GetAssetName() : "Not loaded";
                strncpy_s(shaderBuffer, shaderName.c_str(), sizeof(shaderBuffer) - 1);
                ImGui::InputText("##MaterialShader", shaderBuffer, sizeof(shaderBuffer), ImGuiInputTextFlags_ReadOnly);
            
                // Texture count
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Textures:");
                ImGui::SameLine();
                int totalTextures = static_cast<int>(material->txtrAssets.size());
                int loadedTextures = 0;
                for (const auto& entry : material->txtrAssets) {
                    if (entry.asset) {
                        CPakAsset* texturePakAsset = static_cast<CPakAsset*>(entry.asset);
                        if (texturePakAsset && texturePakAsset->extraData()) {
                            TextureAsset* txtr = reinterpret_cast<TextureAsset*>(texturePakAsset->extraData());
                            if (!txtr->mipArray.empty()) {
                                loadedTextures++;
                            }
                        }
                    }
                }
            ImGui::Text("%d total (%d loaded)", totalTextures, loadedTextures);
            
                // List textures with details
                if (totalTextures > 0 && ImGui::TreeNode("Texture Details")) {
                    for (size_t i = 0; i < material->txtrAssets.size(); ++i) {
                        const auto& entry = material->txtrAssets[i];
                        if (entry.asset) {
                            CPakAsset* texturePakAsset = static_cast<CPakAsset*>(entry.asset);
                            bool hasData = texturePakAsset && texturePakAsset->extraData() != nullptr;
                            ImGui::PushStyleColor(ImGuiCol_Text, hasData ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(0.6f, 0.4f, 0.4f, 1.0f));
                            
                            ImGui::Text("Slot %d: %s", entry.index, entry.asset->GetAssetName().c_str());
                            
                            if (hasData) {
                                TextureAsset* txtr = reinterpret_cast<TextureAsset*>(texturePakAsset->extraData());
                                if (!txtr->mipArray.empty()) {
                                    ImGui::SameLine();
                                    ImGui::TextDisabled("(%dx%d, %zu mips)", txtr->width, txtr->height, txtr->mipArray.size());
                                    
                                    // Show loading status
                                    int loadedMips = 0;
                                    for (const auto& mip : txtr->mipArray) {
                                        if (mip.isLoaded) loadedMips++;
                                    }
                                    if (loadedMips < static_cast<int>(txtr->mipArray.size())) {
                                        ImGui::SameLine();
                                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "[Loading %d/%zu]", loadedMips, txtr->mipArray.size());
                                    }
                                }
                            } else {
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "[Not loaded]");
                            }
                            
                            ImGui::PopStyleColor();
                        }
                    }
                    ImGui::TreePop();
                }
                
                ImGui::Spacing();
            }
        }
        
        // Export status
        DrawSeparatorWithText("Export Status");
        ImGui::Spacing();

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

        // Action buttons section
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Actions");
        ImGui::Spacing();

        if (ImGui::Button("Copy Name", ImVec2(-1, 0)))
        {
            ImGui::SetClipboardText(fileName.c_str());
        }

        if (ImGui::Button("Copy GUID", ImVec2(-1, 0)))
        {
            ImGui::SetClipboardText(guidStr.c_str());
        }

        if (ImGui::Button("Open in Tab", ImVec2(-1, 0)))
        {
            OpenAssetInTab(primaryAsset);
        }

        ImGui::EndChild();

        ImGui::Unindent(8.0f);
        ImGui::Spacing();
        ImGui::PopStyleVar();
    }

    void LayoutManager::RenderViewport3D()
    {
        // Add manual padding
        ImGui::Spacing();
        ImGui::Indent(8.0f);
        
        // Check if we have a texture or material selected
        bool hasTextureSelected = false;
        bool hasMaterialSelected = false;
        CAsset* textureAsset = nullptr;
        CAsset* materialAsset = nullptr;
        
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
                    // Check for material type: 'matl' (0x6C74616D)
                    else if (assetType == 0x6C74616D) {
                        hasMaterialSelected = true;
                        materialAsset = firstAsset;
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
        
        if (hasMaterialSelected && materialAsset) {
            // Show material viewer instead of 3D viewport
            RenderMaterialViewer(materialAsset);
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
        
        ImGui::Unindent(8.0f);
        ImGui::Spacing();
    }

    void LayoutManager::RenderAssetPreview()
    {
        // Add manual padding
        ImGui::Spacing();
        ImGui::Indent(8.0f);
        
        if (m_selectedAssets.empty())
        {
            ImGui::TextDisabled("No asset selected");
            ImGui::Spacing();
            ImGui::TextWrapped("Select an asset from the Asset Browser to preview it here.");
            ImGui::Unindent(8.0f);
            ImGui::Spacing();
            return;
        }
        
        CAsset* asset = m_selectedAssets[0];
        static CAsset* prevPreviewAsset = nullptr;
        
        // Check if we have a preview function for this asset type
        uint32_t assetType = asset->GetAssetType();
        auto it = g_assetData.m_assetTypeBindings.find(assetType);
        
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
        
        ImGui::Unindent(8.0f);
        ImGui::Spacing();
    }

    void LayoutManager::RenderProperties()
    {
        // Add manual padding for consistent layout
        ImGui::Spacing();
        ImGui::Indent(8.0f);

        // Modern header with better styling
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));

        // Header section
        ImGui::Text("Application Settings");
        ImGui::Separator();
        ImGui::Spacing();

        // Create scrollable content area
        if (ImGui::BeginChild("PropertiesContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoBackground))
        {
            // Display Settings Section
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.3f, 1.0f), "Display Settings");
            ImGui::Spacing();

            static bool enableVSync = true;
            ImGui::Checkbox("Enable VSync", &enableVSync);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Synchronize rendering with display refresh rate");
            }

            static int targetFPS = 60;
            ImGui::SliderInt("Target FPS", &targetFPS, 30, 144);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Maximum frames per second when VSync is disabled");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Asset Browser Section
            ImGui::TextColored(ImVec4(0.3f, 0.7f, 0.9f, 1.0f), "Asset Browser");
            ImGui::Spacing();

            ImGui::Checkbox("Tree View by Default", &m_showAssetTreeView);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Show hierarchical tree view instead of flat list");
            }

            static bool showFileExtensions = true;
            ImGui::Checkbox("Show File Extensions", &showFileExtensions);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Display file extensions in asset names");
            }

            static bool autoRefresh = false;
            ImGui::Checkbox("Auto Refresh", &autoRefresh);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Automatically refresh asset list when files change");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Viewport Settings Section
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.7f, 1.0f), "3D Viewport");
            ImGui::Spacing();

            static bool showGrid = true;
            ImGui::Checkbox("Show Grid", &showGrid);

            static bool showAxes = true;
            ImGui::Checkbox("Show Axes", &showAxes);

            static float backgroundColor[3] = {0.05f, 0.05f, 0.05f};
            ImGui::ColorEdit3("Background Color", backgroundColor);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Material Preview Section
            ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Material Preview");
            ImGui::Spacing();

            ImGui::Checkbox("Auto-rotate Materials", &m_materialSphereState.autoRotate);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Automatically rotate material preview sphere");
            }

            ImGui::SliderFloat("Rotation Speed", &m_materialSphereState.rotationSpeed, 0.1f, 5.0f);
            ImGui::SliderFloat("Sphere Scale", &m_materialSphereState.sphereScale, 0.5f, 3.0f);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Export Settings Section
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.7f, 1.0f), "Export Settings");
            ImGui::Spacing();

            static bool preserveHierarchy = true;
            ImGui::Checkbox("Preserve Hierarchy", &preserveHierarchy);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Maintain folder structure when exporting");
            }

            static bool embedTextures = false;
            ImGui::Checkbox("Embed Textures", &embedTextures);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Include texture data in exported files");
            }

            static int compressionLevel = 5;
            ImGui::SliderInt("Compression Level", &compressionLevel, 0, 9);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Higher values = smaller files, slower export");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Advanced Settings Section
            ImGui::TextColored(ImVec4(0.9f, 0.5f, 0.3f, 1.0f), "Advanced");
            ImGui::Spacing();

            static int maxCacheSize = 512;
            ImGui::SliderInt("Cache Size (MB)", &maxCacheSize, 128, 2048);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Maximum memory for texture and model cache");
            }

            static int threadCount = 4;
            ImGui::SliderInt("Worker Threads", &threadCount, 1, 16);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Number of threads for asset processing");
            }

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
                maxCacheSize = 512;
                threadCount = 4;
            }

            ImGui::EndChild();
        }

        ImGui::Unindent(8.0f);
        ImGui::Spacing();
        ImGui::PopStyleVar();
    }

    void LayoutManager::RenderConsole()
    {
        // Add manual padding
        ImGui::Spacing();
        ImGui::Indent(8.0f);
        
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
        
        ImGui::Unindent(8.0f);
        ImGui::Spacing();
    }

    void LayoutManager::RenderMaterialPreview()
    {
        // Add manual padding
        ImGui::Spacing();
        ImGui::Indent(8.0f);

        // Check if we have a material selected
        bool hasMaterialSelected = false;
        CAsset* materialAsset = nullptr;

        if (!m_selectedAssets.empty()) {
            CAsset* firstAsset = m_selectedAssets[0];
            if (firstAsset) {
                try {
                    uint32_t assetType = firstAsset->GetAssetType();
                    // Check for material type: 'matl' (0x6C74616D)
                    if (assetType == 0x6C74616D) {
                        hasMaterialSelected = true;
                        materialAsset = firstAsset;
                    }
                } catch (...) {
                    // Asset type couldn't be read
                }
            }
        }

        if (!hasMaterialSelected || !materialAsset) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 0.6f), "Select a material to preview it here");
            ImGui::Unindent(8.0f);
            ImGui::Spacing();
            return;
        }

        // Split the area: Material details on top, sphere preview on bottom
        const float totalHeight = ImGui::GetContentRegionAvail().y;
        const float materialViewerHeight = totalHeight * 0.6f; // 60% for material details
        //const float spherePreviewHeight = totalHeight * 0.4f;  // 40% for sphere preview

        // Top section: Full material viewer
        if (ImGui::BeginChild("MaterialDetails", ImVec2(0, materialViewerHeight), true))
        {
            RenderMaterialViewer(materialAsset);
        }
        ImGui::EndChild();

        // Bottom section: Sphere preview
        if (ImGui::BeginChild("SpherePreview", ImVec2(0, 0), true))
        {
            try {
                CPakAsset* pakAsset = static_cast<CPakAsset*>(materialAsset);
                if (pakAsset && pakAsset->extraData()) {
                    const MaterialAsset* const material = reinterpret_cast<MaterialAsset*>(pakAsset->extraData());
                    if (material) {
                        RenderMaterialSpherePreview(material, materialAsset);
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to cast material data");
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Material data not loaded");
                }
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error loading material: %s", e.what());
            } catch (...) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unknown error loading material");
            }
        }
        ImGui::EndChild();

        ImGui::Unindent(8.0f);
        ImGui::Spacing();
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
            AssetTreeNode& emptyNode = m_assetTreeRoot.children.emplace_back("No assets loaded", "", true);
            emptyNode.isExpanded = false;
            return;
        }
        
        // Helper function to normalize path separators
        auto normalizePath = [](const std::string& path) {
            std::string normalized = path;
            std::replace(normalized.begin(), normalized.end(), '\\', '/');
            return normalized;
        };
        
        // Helper function to split path into components
        auto splitPath = [](const std::string& path) {
            std::vector<std::string> components;
            std::stringstream ss(path);
            std::string component;
            
            while (std::getline(ss, component, '/')) {
                if (!component.empty()) {
                    components.push_back(component);
                }
            }
            return components;
        };
        
        // Helper function to find or create a directory node
        std::function<AssetTreeNode*( AssetTreeNode&, const std::vector<std::string>&, size_t)> findOrCreatePath;
        findOrCreatePath = [&](AssetTreeNode& parent, const std::vector<std::string>& pathComponents, size_t depth) -> AssetTreeNode* {
            if (depth >= pathComponents.size()) {
                return &parent;
            }
            
            const std::string& dirName = pathComponents[depth];
            
            // Look for existing child with this name
            for (auto& child : parent.children) {
                if (child.name == dirName && child.isDirectory) {
                    return findOrCreatePath(child, pathComponents, depth + 1);
                }
            }
            
            // Create new directory node
            std::string fullPath = "";
            for (size_t i = 0; i <= depth; ++i) {
                if (i > 0) fullPath += "/";
                fullPath += pathComponents[i];
            }
            
            AssetTreeNode& newDir = parent.children.emplace_back(dirName, fullPath, true);
            newDir.isExpanded = false;
            return findOrCreatePath(newDir, pathComponents, depth + 1);
        };
        
        // Process all assets and organize by file path
        for (const auto& lookup : g_assetData.v_assets) {
            if (!lookup.m_asset) continue;
            
            CAsset* asset = lookup.m_asset;
            std::string assetName;
            
            try {
                assetName = asset->GetAssetName();
                if (assetName.empty()) {
                    continue; // Skip assets with no name
                }
            } catch (...) {
                continue; // Skip invalid assets
            }
            
            // Normalize the asset path
            std::string normalizedPath = normalizePath(assetName);
            std::vector<std::string> pathComponents = splitPath(normalizedPath);
            
            if (pathComponents.empty()) {
                continue; // Skip empty paths
            }
            
            // Find or create the directory structure for this asset
            AssetTreeNode* targetDir = &m_assetTreeRoot;
            if (pathComponents.size() > 1) {
                // All components except the last one are directories
                std::vector<std::string> dirComponents(pathComponents.begin(), pathComponents.end() - 1);
                targetDir = findOrCreatePath(m_assetTreeRoot, dirComponents, 0);
            }
            
            // Add the asset to the target directory
            targetDir->assets.push_back(asset);
        }
        
        // Recursive function to sort directories and remove empty ones
        std::function<void(AssetTreeNode&)> sortAndCleanup;
        sortAndCleanup = [&](AssetTreeNode& node) {
            // Remove empty directories
            node.children.erase(
                std::remove_if(node.children.begin(), node.children.end(),
                    [](const AssetTreeNode& child) {
                        return child.isDirectory && child.children.empty() && child.assets.empty();
                    }),
                node.children.end()
            );
            
            // Sort children alphabetically (directories first, then by name)
            std::sort(node.children.begin(), node.children.end(),
                [](const AssetTreeNode& a, const AssetTreeNode& b) {
                    if (a.isDirectory != b.isDirectory) {
                        return a.isDirectory; // Directories first
                    }
                    return a.name < b.name;
                });
            
            // Recursively process children
            for (auto& child : node.children) {
                sortAndCleanup(child);
            }
        };
        
        sortAndCleanup(m_assetTreeRoot);
    }

    void LayoutManager::RenderAssetTreeNode(AssetTreeNode& node)
    {
        bool hasChildren = !node.children.empty();
        bool hasAssets = !node.assets.empty();
        
        if (node.isDirectory)
        {
            // This is a directory node
            std::string nodeLabel;
            if (hasChildren && hasAssets) {
                nodeLabel = std::format("{} ({} folders, {} files)", node.name, node.children.size(), node.assets.size());
            } else if (hasChildren) {
                nodeLabel = std::format("{} ({} folders)", node.name, node.children.size());
            } else if (hasAssets) {
                nodeLabel = std::format("{} ({} files)", node.name, node.assets.size());
            } else {
                nodeLabel = std::format("{} (empty)", node.name);
            }
            
            // Use TreeNodeEx for better control over expansion state
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
            if (!hasChildren && !hasAssets) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }
            if (node.isExpanded) {
                flags |= ImGuiTreeNodeFlags_DefaultOpen;
            }
            
            bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), flags);
            
            if (nodeOpen)
            {
                // Update expansion state
                node.isExpanded = true;
                
                // Render child directories first
                for (auto& child : node.children)
                {
                    RenderAssetTreeNode(child);
                }
                
                // Then render individual assets in this directory
                if (hasAssets)
                {
                    for (CAsset* asset : node.assets)
                    {
                        if (!asset) continue;
                        
                        std::string displayName = "Asset";
                        std::string assetPath;
                        try {
                            assetPath = asset->GetAssetName();
                            // Extract just the filename from the path
                            size_t lastSlash = assetPath.find_last_of("/\\");
                            if (lastSlash != std::string::npos) {
                                displayName = assetPath.substr(lastSlash + 1);
                            } else {
                                displayName = assetPath;
                            }
                            
                            if (displayName.empty()) {
                                displayName = "Unnamed Asset";
                            }
                        } catch (...) {
                            displayName = "Invalid Asset";
                        }
                        
                        // Check if this asset is selected
                        bool isSelected = std::find(m_selectedAssets.begin(), m_selectedAssets.end(), asset) != m_selectedAssets.end();
                        
                        // Add some indentation for files to distinguish from folders
                        ImGui::Indent(16.0f);
                        
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

                        // Handle double-click to open in tab (Godot-style)
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            OpenAssetInTab(asset);
                        }

                        // Show tooltip with full path on hover
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("%s", assetPath.c_str());
                        }
                        
                        ImGui::Unindent(16.0f);
                    }
                }
                
                ImGui::TreePop();
            }
            else
            {
                // Update expansion state when collapsed
                node.isExpanded = false;
            }
        }
        else
        {
            // Individual asset (shouldn't happen with our new structure, but kept for safety)
            std::string displayName = node.name;
            
            bool isSelected = false; // We'd need to check if any assets match
            if (ImGui::Selectable(displayName.c_str(), isSelected))
            {
                // Handle individual asset selection if needed
            }
        }
    }

    void LayoutManager::RenderAssetTable(const std::vector<CAsset*>& assets)
    {
        // Add some padding and spacing
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 6.0f));
        
        if (ImGui::BeginTable("AssetTable", 2,
                             ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
                             ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV |
                             ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable |
                             ImGuiTableFlags_SizingStretchProp))
        {
            // Simplified column setup - just type and name for cleaner look
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            
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
                    
                    // Type column
                    if (ImGui::TableSetColumnIndex(0))
                    {
                        char typeStr[5] = {0};
                        uint32_t assetType = asset->GetAssetType();
                        memcpy(typeStr, &assetType, 4);
                        for (int i2 = 0; i2 < 4; ++i2) {
                            if (typeStr[i2] < 32 || typeStr[i2] > 126) typeStr[i2] = '?';
                        }
                        ImGui::Text("%s", typeStr);
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

                        // Handle double-click to open in tab (Godot-style)
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            OpenAssetInTab(asset);
                        }
                        
                        // Context menu for asset
                        if (ImGui::BeginPopupContextItem())
                        {
                            if (ImGui::MenuItem("Open in Tab"))
                            {
                                OpenAssetInTab(asset);
                            }
                            ImGui::Separator();
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
            
            ImGui::Text("Detailed Texture Information");
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
        ImGui::Text("Texture Viewer");
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
                ImGui::Text("%s", textureAsset->GetAssetName().c_str());
                
                // Quick info in one line
                ImGui::SameLine();
                ImGui::TextDisabled(" - ");
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
                
                ImGui::Text("%dx%d | %s | %d mips", 
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
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Cannot Display Texture");
                ImGui::Text("This texture format may not be supported");
                ImGui::Text("or the texture data may be corrupted.");
                ImGui::EndGroup();
            }
            
        } catch (...) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error reading texture asset");
        }
    }
    
    void LayoutManager::RenderModelViewer3D()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
        
        // Check if we have a model selected (declare at function scope)
        bool hasModelSelected = false;
        CAsset* modelAsset = nullptr;
        
        for (CAsset* asset : m_selectedAssets)
        {
            if (!asset) continue;
            
            try 
            {
                uint32_t assetType = asset->GetAssetType();
                // Check for model types: MDL_, ARIG, ASEQ, etc.
                if (assetType == 0x5F6C646D || // '_ldm' (MDL_)
                    assetType == 0x67697261 || // 'arig' (ARIG)
                    assetType == 0x71657361 || // 'aseq' (ASEQ)
                    assetType == 0x006C646D || // 'mdl\0' (MDL)
                    assetType == 0x6C646D72)   // 'rmdl' - Additional model type
                {
                    hasModelSelected = true;
                    modelAsset = asset;
                    break;
                }
            } 
            catch (...) 
            {
                continue;
            }
        }
        
        // Load the model for 3D preview if one is selected
        if (hasModelSelected && modelAsset)
        {
            try 
            {
                uint32_t assetType = modelAsset->GetAssetType();
                auto bindingIt = g_assetData.m_assetTypeBindings.find(assetType);
                
                if (bindingIt != g_assetData.m_assetTypeBindings.end() && bindingIt->second.previewFunc)
                {
                    static CAsset* lastPreviewedAsset = nullptr;
                    const bool isFirstFrame = (lastPreviewedAsset != modelAsset);
                    
                    // Force first frame periodically to reload textures when debugging
                    static int forceFirstFrameCounter = 0;
                    bool actualFirstFrame = isFirstFrame;
                    
                    lastPreviewedAsset = modelAsset;
                    
                    // Call the preview function to load the model
                    void* previewResult = bindingIt->second.previewFunc(modelAsset, actualFirstFrame);
                    if (previewResult) {
                        // Set the global previewDrawData for the main render loop
                        previewDrawData = reinterpret_cast<CDXDrawData*>(previewResult);
                    } else {
                        previewDrawData = nullptr;
                    }
                } 
                else 
                {
                    previewDrawData = nullptr;
                }
            } 
            catch (...) 
            {
                previewDrawData = nullptr;
            }
        } 
        else 
        {
            // No model selected - clear model rendering
            previewDrawData = nullptr;
        }
        
        // Compact header toolbar for center panel
        ImGui::AlignTextToFramePadding();
        ImGui::Text("3D Model Viewer");
        
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        
        // Quick controls in header
        ImGui::Text("View Controls:");
        ImGui::SameLine();
        
        ImGui::Checkbox("Skybox", &m_modelViewerState.showSkybox);
        ImGui::SameLine();
        
        ImGui::Checkbox("Default Texture", &m_modelViewerState.useDefaultTexture);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Replace all textures with a default white texture\n\nPress 'T' while right-clicking in the viewport to toggle texture debug info");
        }
        
        // Second row of controls
        ImGui::Checkbox("Shadows", &m_modelViewerState.showShadows);
        ImGui::SameLine();
        
        ImGui::Checkbox("Grid Floor", &m_modelViewerState.showGridFloor);
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(100);
        ImGui::SliderFloat("Speed", &m_modelViewerState.cameraSpeed, 0.1f, 10.0f, "%.1f");
        ImGui::SameLine();
        
        if (ImGui::Button("Reset Cam"))
        {
            if (g_dxHandler && g_dxHandler->GetCamera())
            {
                auto* camera = g_dxHandler->GetCamera();
                camera->position = {0, 0, -5};
                camera->rotation = {0, 0, 0};
            }
            m_modelViewerState.cameraPosition = {0, 0, -5};
            m_modelViewerState.cameraRotation = {0, 0, 0};
        }
        ImGui::SameLine();
        
        ImGui::Checkbox("Auto Rotate", &m_modelViewerState.autoRotate);
        if (m_modelViewerState.autoRotate) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::SliderFloat("##RotSpeed", &m_modelViewerState.autoRotateSpeed, 0.1f, 2.0f, "%.1f");
        }
        
        ImGui::Separator();
        
        // Model status line
        if (hasModelSelected && modelAsset)
        {
            std::string modelInfo = "Model: " + modelAsset->GetAssetName();
            bool modelLoaded = (previewDrawData != nullptr);
            
            if (modelLoaded) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", modelInfo.c_str());
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s (Loading...)", modelInfo.c_str());
            }
            
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " | Right-click + WASD: Move | Right-click + Mouse: Look");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No model selected - Choose a model asset (.mdl, .arig, .aseq) to view");
        }
        
        // Main 3D Viewport using render-to-texture
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        // Ensure minimum size
        int targetWidth = static_cast<int>(std::max(viewportSize.x, 400.0f));
        int targetHeight = static_cast<int>(std::max(viewportSize.y, 300.0f));
        
        // Create or resize render target if needed
        if (!m_modelViewerState.renderTargetView || 
            targetWidth != m_modelViewerState.renderWidth || 
            targetHeight != m_modelViewerState.renderHeight) 
        {
            if (!CreateModelViewerRenderTarget(targetWidth, targetHeight)) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to create render target");
                return;
            }
        }
        
        // Update camera speed
        if (g_dxHandler) {
            g_PreviewSettings.previewMovementSpeed = m_modelViewerState.cameraSpeed * 50.0f;
        }
        
        bool isViewportHovered = false;
        
        // Handle auto-rotation
        if (m_modelViewerState.autoRotate && g_dxHandler)
        {
            static float autoRotateTime = 0.0f;
            autoRotateTime += ImGui::GetIO().DeltaTime * m_modelViewerState.autoRotateSpeed;
            if (g_dxHandler->GetCamera())
            {
                g_dxHandler->GetCamera()->rotation.y = autoRotateTime * 0.5f;
            }
        }
        
        // Render the 3D model to our render target
        if (hasModelSelected && previewDrawData) {
            RenderModelToTexture();
        }
        
        // Display the rendered texture in ImGui
        if (m_modelViewerState.shaderResourceView)
        {
            // Create proper viewport child for the image
            if (ImGui::BeginChild("ModelViewer3DImage", viewportSize, false, 
                                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                // Display the rendered 3D scene as an image
                ImGui::Image(m_modelViewerState.shaderResourceView, 
                           ImVec2(static_cast<float>(targetWidth), static_cast<float>(targetHeight)));
                
                // Check if the image is hovered and handle input
                isViewportHovered = ImGui::IsItemHovered();
                
                if (isViewportHovered)
                {
                    // Handle camera input when right mouse button is held
                    bool rightMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

                    if (rightMouseDown && g_dxHandler && g_pInput)
                    {
                        CDXCamera* camera = g_dxHandler->GetCamera();
                        float deltaTime = ImGui::GetIO().DeltaTime;

                        // WASD movement using ImGui key detection (only when right mouse is held)
                        bool wPressed = ImGui::IsKeyDown(ImGuiKey_W);
                        bool sPressed = ImGui::IsKeyDown(ImGuiKey_S);
                        bool aPressed = ImGui::IsKeyDown(ImGuiKey_A);
                        bool dPressed = ImGui::IsKeyDown(ImGuiKey_D);
                        bool spacePressed = ImGui::IsKeyDown(ImGuiKey_Space);
                        bool shiftPressed = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);

                        // Manual WASD movement
                        Vector& pos = camera->position;
                        const float moveSpeed = g_PreviewSettings.previewMovementSpeed;
                        const float yaw = camera->rotation.y;

                        // Calculate movement vectors
                        const float x = sin(yaw);
                        const float z = cos(yaw);
                        const float nx = sin(DEG2RAD(90) - yaw);
                        const float nz = cos(DEG2RAD(90) - yaw);

                        // WASD movement
                        if (wPressed)
                        {
                            pos.x += moveSpeed * deltaTime * x;
                            pos.z += moveSpeed * deltaTime * z;
                        }
                        if (sPressed)
                        {
                            pos.x -= moveSpeed * deltaTime * x;
                            pos.z -= moveSpeed * deltaTime * z;
                        }
                        if (aPressed)
                        {
                            pos.x += moveSpeed * deltaTime * -nx;
                            pos.z += moveSpeed * deltaTime * nz;
                        }
                        if (dPressed)
                        {
                            pos.x -= moveSpeed * deltaTime * -nx;
                            pos.z -= moveSpeed * deltaTime * nz;
                        }
                        if (spacePressed)
                        {
                            pos.y += moveSpeed * deltaTime;
                        }
                        if (shiftPressed)
                        {
                            pos.y -= moveSpeed * deltaTime;
                        }

                        // Mouse look when right-clicking and dragging
                        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
                        {
                            ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
                            camera->AddRotation(mouseDelta.x * 0.002f, mouseDelta.y * 0.002f, 0);
                        }
                    }
                    else
                    {
                        // Show help tooltip when not navigating
                        ImGui::SetTooltip("Hold right-click and use WASD to navigate around the model");
                    }
                }
            }
            ImGui::EndChild();
        }
        else if (hasModelSelected)
        {
            // Model loading state
            const ImVec2 center = ImVec2(viewportSize.x * 0.5f, viewportSize.y * 0.5f);
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 windowPos = ImGui::GetCursorScreenPos();
            
            const char* message = "Initializing 3D renderer...\nPlease wait";
            const ImVec2 textSize = ImGui::CalcTextSize(message);
            const ImVec2 textPos = ImVec2(windowPos.x + center.x - textSize.x * 0.5f, 
                                         windowPos.y + center.y - textSize.y * 0.5f);
            
            drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.8f, 0.0f, 1.0f)), message);
        }
        else
        {
            // No model selected state
            const ImVec2 center = ImVec2(viewportSize.x * 0.5f, viewportSize.y * 0.5f);
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 windowPos = ImGui::GetCursorScreenPos();
            
            const char* message = "No Model Selected\n\nChoose a 3D model asset from the asset browser\nto view it in this viewport";
            const ImVec2 textSize = ImGui::CalcTextSize(message);
            const ImVec2 textPos = ImVec2(windowPos.x + center.x - textSize.x * 0.5f, 
                                         windowPos.y + center.y - textSize.y * 0.5f);
            
            drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), message);
        }
        
        ImGui::PopStyleVar();
    }

    bool LayoutManager::CreateModelViewerRenderTarget(int width, int height)
    {
        if (!g_dxHandler) return false;
        
        ID3D11Device* device = g_dxHandler->GetDevice();
        if (!device) return false;
        
        // Clean up existing resources
        DestroyModelViewerRenderTarget();
        
        // Create render target texture
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        
        HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &m_modelViewerState.renderTexture);
        if (FAILED(hr)) return false;
        
        // Create render target view
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = textureDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        
        hr = device->CreateRenderTargetView(m_modelViewerState.renderTexture, &rtvDesc, &m_modelViewerState.renderTargetView);
        if (FAILED(hr)) return false;
        
        // Create shader resource view for ImGui
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        
        hr = device->CreateShaderResourceView(m_modelViewerState.renderTexture, &srvDesc, &m_modelViewerState.shaderResourceView);
        if (FAILED(hr)) return false;
        
        // Create depth buffer
        D3D11_TEXTURE2D_DESC depthDesc = {};
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDesc.CPUAccessFlags = 0;
        depthDesc.MiscFlags = 0;
        
        hr = device->CreateTexture2D(&depthDesc, nullptr, &m_modelViewerState.depthTexture);
        if (FAILED(hr)) return false;
        
        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = depthDesc.Format;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        
        hr = device->CreateDepthStencilView(m_modelViewerState.depthTexture, &dsvDesc, &m_modelViewerState.depthStencilView);
        if (FAILED(hr)) return false;
        
        m_modelViewerState.renderWidth = width;
        m_modelViewerState.renderHeight = height;
        
        return true;
    }
    
    void LayoutManager::DestroyModelViewerRenderTarget()
    {
        if (m_modelViewerState.renderTargetView) {
            m_modelViewerState.renderTargetView->Release();
            m_modelViewerState.renderTargetView = nullptr;
        }
        if (m_modelViewerState.shaderResourceView) {
            m_modelViewerState.shaderResourceView->Release();
            m_modelViewerState.shaderResourceView = nullptr;
        }
        if (m_modelViewerState.depthStencilView) {
            m_modelViewerState.depthStencilView->Release();
            m_modelViewerState.depthStencilView = nullptr;
        }
        if (m_modelViewerState.renderTexture) {
            m_modelViewerState.renderTexture->Release();
            m_modelViewerState.renderTexture = nullptr;
        }
        if (m_modelViewerState.depthTexture) {
            m_modelViewerState.depthTexture->Release();
            m_modelViewerState.depthTexture = nullptr;
        }
    }
    
    void LayoutManager::ResizeModelViewerRenderTarget(int width, int height)
    {
        if (width != m_modelViewerState.renderWidth || height != m_modelViewerState.renderHeight) {
            CreateModelViewerRenderTarget(width, height);
        }
    }
    
    void LayoutManager::RenderModelToTexture()
    {
        if (!g_dxHandler || !m_modelViewerState.renderTargetView || !previewDrawData) {
            return;
        }
        
        ID3D11Device* device = g_dxHandler->GetDevice();
        ID3D11DeviceContext* context = g_dxHandler->GetDeviceContext();
        if (!context || !device) return;
        
        // Save current render targets and viewport
        ID3D11RenderTargetView* originalRTV = nullptr;
        ID3D11DepthStencilView* originalDSV = nullptr;
        context->OMGetRenderTargets(1, &originalRTV, &originalDSV);
        
        UINT numViewports = 1;
        D3D11_VIEWPORT originalViewport;
        context->RSGetViewports(&numViewports, &originalViewport);
        
        // Set our render target
        context->OMSetRenderTargets(1, &m_modelViewerState.renderTargetView, m_modelViewerState.depthStencilView);
        
        // Clear render target with skybox, ground plane, or default color
        float clearColor[4];
        if (m_modelViewerState.showSkybox) {
            // Sky blue gradient color
            clearColor[0] = 0.4f;  // R
            clearColor[1] = 0.6f;  // G  
            clearColor[2] = 0.9f;  // B
            clearColor[3] = 1.0f;  // A
        } else if (m_modelViewerState.showGridFloor) {
            // Studio lighting background - light gray
            clearColor[0] = 0.15f;
            clearColor[1] = 0.15f;
            clearColor[2] = 0.15f;
            clearColor[3] = 1.0f;
        } else {
            // Dark background
            clearColor[0] = 0.02f;
            clearColor[1] = 0.02f;
            clearColor[2] = 0.02f;
            clearColor[3] = 1.0f;
        }
        context->ClearRenderTargetView(m_modelViewerState.renderTargetView, clearColor);
        context->ClearDepthStencilView(m_modelViewerState.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        // Set our viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(m_modelViewerState.renderWidth);
        viewport.Height = static_cast<float>(m_modelViewerState.renderHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context->RSSetViewports(1, &viewport);
        
        // Set render states (matching original render loop)
        context->RSSetState(g_dxHandler->GetRasterizerState());
        context->OMSetDepthStencilState(g_dxHandler->GetDepthStencilState(), 1u);
        
        // Render the 3D model (copied from main render loop)
        CDXCamera* camera = g_dxHandler->GetCamera();
        
        // Update camera data
        camera->CommitCameraDataBufferUpdates();
        
        // Render grid floor if enabled
        if (m_modelViewerState.showGridFloor) {
            RenderGridFloor(context, camera);
        }
        
        // Setup enhanced lighting system
        if (m_modelViewerState.showLighting) {
            CDXScene& scene = g_dxHandler->GetScene();
            
            // Clear existing lights and set up a better lighting rig
            scene.globalLights.clear();
            
            if (m_modelViewerState.showShadows) {
                // Dramatic lighting setup for shadows
                
                // Strong key light from above for sharp shadows
                HardwareLight& keyLight = scene.globalLights.emplace_back();
                keyLight.pos = { 10.f, 50.f, -30.f };
                keyLight.rcpMaxRadius = 1 / 300.f;
                keyLight.rcpMaxRadiusSq = 1 / (keyLight.rcpMaxRadius * keyLight.rcpMaxRadius);
                keyLight.attenLinear = -0.5f;
                keyLight.attenQuadratic = 0.2f;
                keyLight.specularIntensity = 1.5f;
                keyLight.color = { 1.2f, 1.1f, 1.0f }; // Bright warm white
                
                // Soft fill light to prevent pure black shadows
                HardwareLight& fillLight = scene.globalLights.emplace_back();
                fillLight.pos = { -40.f, 20.f, 20.f };
                fillLight.rcpMaxRadius = 1 / 200.f;
                fillLight.rcpMaxRadiusSq = 1 / (fillLight.rcpMaxRadius * fillLight.rcpMaxRadius);
                fillLight.attenLinear = -1.8f;
                fillLight.attenQuadratic = 0.9f;
                fillLight.specularIntensity = 0.3f;
                fillLight.color = { 0.4f, 0.45f, 0.6f }; // Soft cool fill
                
            } else {
                // Even lighting setup without dramatic shadows
                
                // Main key light (from above-front)
                HardwareLight& keyLight = scene.globalLights.emplace_back();
                keyLight.pos = { 20.f, 30.f, -20.f };
                keyLight.rcpMaxRadius = 1 / 200.f;
                keyLight.rcpMaxRadiusSq = 1 / (keyLight.rcpMaxRadius * keyLight.rcpMaxRadius);
                keyLight.attenLinear = -0.95238f;
                keyLight.attenQuadratic = 0.45238f;
                keyLight.specularIntensity = 1.0f;
                keyLight.color = { 1.0f, 0.95f, 0.9f }; // Warm white
                
                // Fill light (from the left)
                HardwareLight& fillLight = scene.globalLights.emplace_back();
                fillLight.pos = { -30.f, 10.f, 10.f };
                fillLight.rcpMaxRadius = 1 / 150.f;
                fillLight.rcpMaxRadiusSq = 1 / (fillLight.rcpMaxRadius * fillLight.rcpMaxRadius);
                fillLight.attenLinear = -1.2f;
                fillLight.attenQuadratic = 0.6f;
                fillLight.specularIntensity = 0.6f;
                fillLight.color = { 0.8f, 0.85f, 1.0f }; // Cool white
                
                // Rim light (from behind-right)
                HardwareLight& rimLight = scene.globalLights.emplace_back();
                rimLight.pos = { 15.f, 5.f, 40.f };
                rimLight.rcpMaxRadius = 1 / 120.f;
                rimLight.rcpMaxRadiusSq = 1 / (rimLight.rcpMaxRadius * rimLight.rcpMaxRadius);
                rimLight.attenLinear = -1.5f;
                rimLight.attenQuadratic = 0.8f;
                rimLight.specularIntensity = 0.8f;
                rimLight.color = { 1.0f, 1.0f, 0.9f }; // Slightly warm
            }
            
            if (scene.NeedsLightingUpdate())
                g_dxHandler->GetScene().CreateOrUpdateLights(device, context);
        }
        
        if (previewDrawData->vertexShader && previewDrawData->pixelShader)
        {
            CShader* vertexShader = previewDrawData->vertexShader;
            CShader* pixelShader = previewDrawData->pixelShader;
            
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->IASetInputLayout(vertexShader->GetInputLayout());
            
            if (vertexShader)
                context->VSSetShader(vertexShader->Get<ID3D11VertexShader>(), nullptr, 0u);
            
            if (pixelShader)
                context->PSSetShader(pixelShader->Get<ID3D11PixelShader>(), nullptr, 0u);
            
            // CRITICAL: Set the global transforms buffer that was missing!
            ID3D11Buffer* const transformsBuffer = previewDrawData->transformsBuffer;
            context->VSSetConstantBuffers(0u, 1u, &transformsBuffer);
            
            UINT offset = 0u;
            
            for (size_t i = 0; i < previewDrawData->meshBuffers.size(); ++i)
            {
                const DXMeshDrawData_t& meshDrawData = previewDrawData->meshBuffers[i];
                
                // Skip invisible meshes
                if (!meshDrawData.visible) continue;
                
                const bool useAdvancedModelPreview = meshDrawData.vertexShader && meshDrawData.pixelShader;
                
                ID3D11Buffer* sharedConstBuffers[] = {
                    camera->bufCommonPerCamera,
                    previewDrawData->transformsBuffer,
                };
                
                if (meshDrawData.vertexShader)
                {
                    context->IASetInputLayout(meshDrawData.inputLayout);
                    context->VSSetShader(meshDrawData.vertexShader, nullptr, 0u);
                }
                else
                    context->VSSetShader(vertexShader->Get<ID3D11VertexShader>(), nullptr, 0u);
                
                if (useAdvancedModelPreview)
                {
                    context->VSSetConstantBuffers(2u, 2, sharedConstBuffers);
                    context->VSSetShaderResources(60u, 1u, &previewDrawData->boneMatrixSRV);
                    context->VSSetShaderResources(62u, 1u, &previewDrawData->boneMatrixSRV);
                }
                
                for (auto& rsrc : previewDrawData->vertexShaderResources)
                {
                    context->VSSetShaderResources(rsrc.bindPoint, 1u, &rsrc.resourceView);
                }
                
                context->IASetVertexBuffers(0u, 1u, &meshDrawData.vertexBuffer, &meshDrawData.vertexStride, &offset);
                
                if (meshDrawData.pixelShader)
                    context->PSSetShader(meshDrawData.pixelShader, nullptr, 0u);
                else
                    context->PSSetShader(pixelShader->Get<ID3D11PixelShader>(), nullptr, 0u);
                
                ID3D11SamplerState* const samplerState = g_dxHandler->GetSamplerState();
                
                if (useAdvancedModelPreview)
                {
                    ID3D11SamplerState* samplers[] = {
                        g_dxHandler->GetSamplerComparisonState(),
                        samplerState,
                        samplerState,
                    };
                    context->PSSetSamplers(0, 3, samplers);
                    
                    if (meshDrawData.uberStaticBuf)
                        context->PSSetConstantBuffers(0u, 1u, &meshDrawData.uberStaticBuf);
                    context->PSSetConstantBuffers(2u, 2, sharedConstBuffers);
                    
                    #if defined(ADVANCED_MODEL_PREVIEW)
                    scene.BindLightsSRV(context);
                    #endif
                }
                else
                    context->PSSetSamplers(0, 1, &samplerState);
                
                for (auto& tex : meshDrawData.textures)
                {
                    ID3D11ShaderResourceView* textureSRV = nullptr;
                    
                    if (m_modelViewerState.useDefaultTexture) {
                        // Use default texture when enabled - set to nullptr for white/default
                        textureSRV = nullptr;
                    } else {
                        // Use the actual texture exactly like the original code
                        textureSRV = tex.texture ? tex.texture.get()->GetSRV() : nullptr;
                    }
                    
                    context->PSSetShaderResources(tex.resourceBindPoint, 1u, &textureSRV);
                }
                
                for (auto& rsrc : previewDrawData->pixelShaderResources)
                {
                    context->PSSetShaderResources(rsrc.bindPoint, 1u, &rsrc.resourceView);
                }
                
                context->IASetIndexBuffer(meshDrawData.indexBuffer, meshDrawData.indexFormat, 0u);
                context->DrawIndexed(static_cast<UINT>(meshDrawData.numIndices), 0u, 0u);
            }
        }
        
        // Restore original render targets and viewport
        context->OMSetRenderTargets(1, &originalRTV, originalDSV);
        context->RSSetViewports(1, &originalViewport);
        
        if (originalRTV) originalRTV->Release();
        if (originalDSV) originalDSV->Release();
    }
    
    void LayoutManager::RenderGridFloor(ID3D11DeviceContext* context, CDXCamera* camera)
    {
        //Temp to avoid warning
        context = context;
        camera = camera;

        // Simple ground plane effect using environment settings
        // A real implementation would render actual grid geometry
        
        // For now, we simulate a ground plane effect by:
        // 1. Using the light gray background color (already set above)
        // 2. Adding additional ambient lighting to simulate ground reflection
        
        // Add subtle ambient lighting to simulate ground plane reflection
        // This creates a more grounded feeling even without actual geometry
        
        // TODO: Future implementation could include:
        // - Actual floor quad geometry at Y = -10 or similar
        // - Grid texture with UV coordinates  
        // - Reflection/shadow mapping
        // - Procedural grid shader
        
        // For now, the effect is achieved through background color and lighting changes
    }

    void LayoutManager::RenderModel3DViewport()
    {
        // Header with controls
        ImGui::Text("3D Viewport");
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
                    ImGui::Text("Selected: %s", firstAsset->GetAssetName().c_str());
                    
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
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "3D Model - rendering below");
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
                camera->position = {0, 0, -100}; // Reset position - start further back to see full model
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
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "3D Model Ready - Look in viewport below");
                    } else {
                        previewDrawData = nullptr;
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Model failed to load");
                    }
                } catch (...) {
                    previewDrawData = nullptr;
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error loading model");
                }
            } else {
                previewDrawData = nullptr;
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No preview support for this model type");
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
                            drawList->AddText(ImVec2(center.x - 80, center.y), IM_COL32(255, 200, 0, 255), "Loading 3D Model...");
                        } else {
                            // Model should be rendering here!
                            drawList->AddText(ImVec2(center.x - 120, center.y - 30), IM_COL32(100, 255, 100, 255), "3D MODEL SHOULD BE HERE");
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
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a 3D Model");
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

    void LayoutManager::RenderMaterialViewer(CAsset* materialAsset)
    {
        if (!materialAsset) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid material asset");
            return;
        }

        try {
            CPakAsset* pakAsset = static_cast<CPakAsset*>(materialAsset);
            if (!pakAsset || !pakAsset->extraData()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Material data not loaded");
                return;
            }

            // Get the MaterialAsset data
            const MaterialAsset* const material = reinterpret_cast<MaterialAsset*>(pakAsset->extraData());
            if (!material) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to cast material data");
                return;
            }

            // Display material header
            ImGui::Text("Material: %s", materialAsset->GetAssetName().c_str());
            ImGui::Separator();

            // Basic material info
            if (ImGui::BeginChild("MaterialInfo", ImVec2(0, 0), false, ImGuiWindowFlags_NoBackground))
            {
                ImGui::Text("GUID: 0x%016llX", material->guid);
                ImGui::Text("Name: %s", material->name ? material->name : "N/A");
                
                if (material->surfaceProp)
                    ImGui::Text("Surface Property: %s", material->surfaceProp);
                if (material->surfaceProp2)
                    ImGui::Text("Surface Property 2: %s", material->surfaceProp2);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                // Safe material preview implementation to avoid crashes
                if (ImGui::BeginTabBar("MaterialTabs"))
                {
                    if (ImGui::BeginTabItem("Textures"))
                    {
                        // Popup state variables
                        static bool showTexturePopup = false;
                        static CAsset* popupTexture = nullptr;
                        
                        if (material->txtrAssets.empty()) {
                            ImGui::TextDisabled("No textures found in this material");
                        } else {

                            // Table of textures
                            if (ImGui::BeginTable("MaterialTextures", 7, 
                                ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | 
                                ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
                                ImGuiTableFlags_ScrollY))
                            {
                                ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                                ImGui::TableSetupColumn("GUID", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                                ImGui::TableSetupColumn("Dimensions", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                                ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                                ImGui::TableHeadersRow();

                                for (size_t i = 0; i < material->txtrAssets.size(); ++i) {
                                    const TextureAssetEntry_t& entry = material->txtrAssets[i];
                                    
                                    ImGui::PushID(static_cast<int>(i));
                                    ImGui::TableNextRow();

                                    if (ImGui::TableSetColumnIndex(0)) {
                                        ImGui::Text("%d", entry.index);
                                    }

                                    if (ImGui::TableSetColumnIndex(1)) {
                                        uint64_t guid = reinterpret_cast<const uint64_t*>(material->textureHandles)[entry.index];
                                        ImGui::Text("0x%016llX", guid);
                                    }

                                    if (ImGui::TableSetColumnIndex(2)) {
                                        if (entry.asset) {
                                            std::string filename = std::filesystem::path(entry.asset->GetAssetName()).filename().string();
                                            ImGui::TextUnformatted(filename.c_str());
                                        } else {
                                            uint64_t guid = reinterpret_cast<const uint64_t*>(material->textureHandles)[entry.index];
                                            ImGui::Text("0x%016llX", guid);
                                        }
                                    }

                                    if (ImGui::TableSetColumnIndex(3)) {
                                        if (material->resourceBindings.count(entry.index)) {
                                            ImGui::TextUnformatted(material->resourceBindings.at(entry.index).name);
                                        } else {
                                            ImGui::TextDisabled("[unknown]");
                                        }
                                    }

                                    if (ImGui::TableSetColumnIndex(4)) {
                                        if (entry.asset) {
                                            try {
                                                const TextureAsset* texAsset = reinterpret_cast<const TextureAsset*>(entry.asset->extraData());
                                                if (texAsset) {
                                                    ImGui::Text("%d x %d", texAsset->width, texAsset->height);
                                                } else {
                                                    ImGui::TextDisabled("N/A");
                                                }
                                            } catch (...) {
                                                ImGui::TextDisabled("Error");
                                            }
                                        } else {
                                            ImGui::TextDisabled("N/A");
                                        }
                                    }

                                    if (ImGui::TableSetColumnIndex(5)) {
                                        if (entry.asset) {
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                                            ImGui::TextUnformatted("Loaded");
                                            ImGui::PopStyleColor();
                                        } else {
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                                            ImGui::TextUnformatted("Not Loaded");
                                            ImGui::PopStyleColor();
                                        }
                                    }

                                    if (ImGui::TableSetColumnIndex(6)) {
                                        if (entry.asset) {
                                            std::string buttonId = std::format("View##texture_{}", i);
                                            if (ImGui::Button(buttonId.c_str(), ImVec2(50, 0))) {
                                                // Debug output
                                                printf("View button clicked for texture: %s\n", entry.asset->GetAssetName().c_str());
                                                
                                                // Open texture popup
                                                popupTexture = entry.asset;
                                                showTexturePopup = true;
                                                
                                                printf("Popup state set: showTexturePopup=%d, popupTexture=%p\n", 
                                                    showTexturePopup, popupTexture);
                                            }
                                        } else {
                                            ImGui::TextDisabled("N/A");
                                        }
                                    }

                                    ImGui::PopID();
                                }

                                ImGui::EndTable();
                            }
 
                            // Clean texture preview window with proper layout
                            if (showTexturePopup) {
                                ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);
                                ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(1200, 800));
                                
                                if (ImGui::Begin("Texture Preview", &showTexturePopup)) {
                                    if (popupTexture) {
                                        try {
                                            CPakAsset* pakTexture = static_cast<CPakAsset*>(popupTexture);
                                            if (pakTexture && pakTexture->extraData()) {
                                                const TextureAsset* texAsset = reinterpret_cast<const TextureAsset*>(pakTexture->extraData());
                                                if (texAsset) {
                                                    // Header
                                                    ImGui::Text("Texture: %s", popupTexture->GetAssetName().c_str());
                                                    ImGui::Separator();
                                                    
                                                    // Content area with reserved space for close button
                                                    const float footerHeight = 40.0f;
                                                    if (ImGui::BeginChild("Content", ImVec2(0, -footerHeight))) {
                                                        // Basic info in a compact format
                                                        ImGui::Text("Size: %dx%d | Format: %u | Mips: %u | Arrays: %u", 
                                                            texAsset->width, texAsset->height,
                                                            static_cast<uint32_t>(texAsset->imgFormat),
                                                            texAsset->totalMipLevels, texAsset->arraySize);
                                                        
                                                        ImGui::Spacing();
                                                        
                                                        // Simple texture display - just call the safe function without complex layout
                                                        RenderMinimalTextureImage(pakTexture, const_cast<void*>(static_cast<const void*>(texAsset)));
                                                    }
                                                    ImGui::EndChild();
                                                    
                                                    // Footer with close button
                                                    ImGui::Separator();
                                                    float buttonWidth = 80.0f;
                                                    float windowWidth = ImGui::GetWindowWidth();
                                                    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
                                                    
                                                    if (ImGui::Button("Close", ImVec2(buttonWidth, 0))) {
                                                        showTexturePopup = false;
                                                        popupTexture = nullptr;
                                                    }
                                                } else {
                                                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Invalid texture data");
                                                }
                                            } else {
                                                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Texture not loaded");
                                            }
                                        } catch (...) {
                                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error loading texture");
                                        }
                                    }
                                    ImGui::End();
                                }
                            }
                            
                            // Handle popup closing via X button or Escape
                            if (!showTexturePopup && popupTexture) {
                                popupTexture = nullptr;
                            }
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Properties"))
                    {
                        if (ImGui::TreeNode("Flags"))
                        {
                            ImGui::Text("Glue Flags: 0x%08X", material->glueFlags);
                            ImGui::Text("Glue Flags2: 0x%08X", material->glueFlags2);
                            ImGui::Text("Uber Buffer Flags: 0x%02X", material->uberBufferFlags);
                            ImGui::TreePop();
                        }

                        if (ImGui::TreeNode("Samplers"))
                        {
                            for (int i = 0; i < 4; ++i) {
                                ImGui::Text("Sampler[%d]: 0x%02X", i, static_cast<unsigned char>(material->samplers[i]));
                            }
                            ImGui::TreePop();
                        }

                        ImGui::EndTabItem();
                    }


                    ImGui::EndTabBar();
                }
            }
            ImGui::EndChild();

        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error viewing material: %s", e.what());
        } catch (...) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unknown error viewing material");
        }
    }

void LayoutManager::RenderMaterialSpherePreview(const void* materialData, CAsset* materialAsset)
{
    materialAsset = materialAsset;
    const MaterialAsset* material = reinterpret_cast<const MaterialAsset*>(materialData);
    
    if (!material) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid material data");
        return;
    }

    // Clean header for sphere preview - always show
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "3D Material Preview");
    
    // Quick texture status in a compact format
    int validTextures = 0;
    int totalTextures = static_cast<int>(material->txtrAssets.size());
    
    for (const auto& entry : material->txtrAssets) {
        if (entry.asset && entry.asset->extraData()) {
            TextureAsset* txtr = reinterpret_cast<TextureAsset*>(entry.asset->extraData());
            if (!txtr->mipArray.empty()) {
                validTextures++;
            }
        }
    }
    
    ImGui::SameLine();
    if (validTextures > 0) {
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "(%d/%d textures)", validTextures, totalTextures);
    } else if (totalTextures > 0) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "(loading %d textures...)", totalTextures);
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(no textures)");
    }
        
        // Compact controls in a nice layout
        ImGui::Spacing();
        
        // First row: Auto rotate + speed
        ImGui::Checkbox("Auto Rotate", &m_materialSphereState.autoRotate);
        if (m_materialSphereState.autoRotate) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::SliderFloat("##RotSpeed", &m_materialSphereState.rotationSpeed, 0.1f, 5.0f);
            ImGui::SameLine();
            ImGui::TextDisabled("Speed");
        }
        
        // Second row: Scale + rendering options + reset
        ImGui::SetNextItemWidth(120);
        ImGui::SliderFloat("Scale", &m_materialSphereState.sphereScale, 0.5f, 3.0f);
        ImGui::SameLine();
        ImGui::Checkbox("Wireframe", &m_materialSphereState.showWireframe);
        ImGui::SameLine();
        ImGui::Checkbox("Lighting", &m_materialSphereState.showLighting);
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset")) {
            m_materialSphereState.sphereScale = 1.0f;
            m_materialSphereState.rotationSpeed = 1.0f;
            m_materialSphereState.currentRotation = 0.0f;
        }
        
        // Create sphere geometry if needed
        CreateSphereGeometry();
        
        // Create or resize render target if needed
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        canvasSize.y = std::min(canvasSize.y, 400.0f); // Max height
        canvasSize.x = std::min(canvasSize.x, 400.0f); // Max width
        
        if (canvasSize.x < 200 || canvasSize.y < 200) {
            canvasSize = ImVec2(300, 300); // Minimum size
        }
        
        int targetWidth = static_cast<int>(canvasSize.x);
        int targetHeight = static_cast<int>(canvasSize.y);
        
        if (!m_materialSphereState.renderTargetView || 
            m_materialSphereState.renderWidth != targetWidth || 
            m_materialSphereState.renderHeight != targetHeight) {
            CreateMaterialSphereRenderTarget(targetWidth, targetHeight);
        }
        
        // Render the sphere to texture
        if (m_materialSphereState.renderTargetView) {
            RenderMaterialSphereToTexture(material);
            
            // Display the rendered texture
            if (m_materialSphereState.shaderResourceView) {
                ImGui::Image(reinterpret_cast<void*>(m_materialSphereState.shaderResourceView), canvasSize);
            }
        }
        
    }

    bool LayoutManager::CreateMaterialSphereRenderTarget(int width, int height)
    {
        DestroyMaterialSphereRenderTarget();
        
        if (!g_dxHandler || !g_dxHandler->GetDevice()) {
            return false;
        }
        
        ID3D11Device* device = g_dxHandler->GetDevice();
        
        // Create render texture
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        
        HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &m_materialSphereState.renderTexture);
        if (FAILED(hr)) return false;
        
        // Create render target view
        hr = device->CreateRenderTargetView(m_materialSphereState.renderTexture, nullptr, &m_materialSphereState.renderTargetView);
        if (FAILED(hr)) return false;
        
        // Create shader resource view
        hr = device->CreateShaderResourceView(m_materialSphereState.renderTexture, nullptr, &m_materialSphereState.shaderResourceView);
        if (FAILED(hr)) return false;
        
        // Create depth texture
        D3D11_TEXTURE2D_DESC depthDesc = textureDesc;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        
        hr = device->CreateTexture2D(&depthDesc, nullptr, &m_materialSphereState.depthTexture);
        if (FAILED(hr)) return false;
        
        // Create depth stencil view
        hr = device->CreateDepthStencilView(m_materialSphereState.depthTexture, nullptr, &m_materialSphereState.depthStencilView);
        if (FAILED(hr)) return false;
        
        m_materialSphereState.renderWidth = width;
        m_materialSphereState.renderHeight = height;
        
        return true;
    }

    void LayoutManager::DestroyMaterialSphereRenderTarget()
    {
        if (m_materialSphereState.depthStencilView) {
            m_materialSphereState.depthStencilView->Release();
            m_materialSphereState.depthStencilView = nullptr;
        }
        if (m_materialSphereState.depthTexture) {
            m_materialSphereState.depthTexture->Release();
            m_materialSphereState.depthTexture = nullptr;
        }
        if (m_materialSphereState.shaderResourceView) {
            m_materialSphereState.shaderResourceView->Release();
            m_materialSphereState.shaderResourceView = nullptr;
        }
        if (m_materialSphereState.renderTargetView) {
            m_materialSphereState.renderTargetView->Release();
            m_materialSphereState.renderTargetView = nullptr;
        }
        if (m_materialSphereState.renderTexture) {
            m_materialSphereState.renderTexture->Release();
            m_materialSphereState.renderTexture = nullptr;
        }
    }

    void LayoutManager::CreateSphereGeometry()
    {
        if (m_materialSphereState.vertexBuffer) {
            return; // Already created
        }
        
        if (!g_dxHandler || !g_dxHandler->GetDevice()) {
            return;
        }
        
        ID3D11Device* device = g_dxHandler->GetDevice();
        
        // Create sphere geometry (UV sphere)
        const int rings = 16;
        const int sectors = 32;
        const float radius = 1.0f;
        
        std::vector<Vertex_t> vertices;
        std::vector<uint16_t> indices;
        
        // Generate vertices
        for (int r = 0; r <= rings; ++r) {
            float phi = static_cast<float>(r) * XM_PI / rings;
            float y = radius * cosf(phi);
            float ringRadius = radius * sinf(phi);
            
            for (int s = 0; s <= sectors; ++s) {
                float theta = static_cast<float>(s) * 2.0f * XM_PI / sectors;
                float x = ringRadius * cosf(theta);
                float z = ringRadius * sinf(theta);
                
                Vertex_t vertex;
                vertex.position = Vector(x, y, z);
                
                // Pack normal properly - for now, use a simple encoding
                Vector normalVec = Vector(x / radius, y / radius, z / radius);
                vertex.normalPacked = Normal32(); // Initialize with zero
                vertex.normalPacked.PackNormal(normalVec, Vector4D(1, 0, 0, 1)); // Pack with tangent
                
                vertex.texcoord = Vector2D(static_cast<float>(s) / sectors, static_cast<float>(r) / rings);
                vertex.color = Color32(255, 255, 255, 255); // White
                vertex.weightCount = 0;
                vertex.weightIndex = 0;
                
                vertices.push_back(vertex);
            }
        }
        
        // Generate indices
        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                int current = r * (sectors + 1) + s;
                int next = current + sectors + 1;
                
                // First triangle
                indices.push_back(static_cast<uint16_t>(current));
                indices.push_back(static_cast<uint16_t>(next));
                indices.push_back(static_cast<uint16_t>(current + 1));
                
                // Second triangle
                indices.push_back(static_cast<uint16_t>(current + 1));
                indices.push_back(static_cast<uint16_t>(next));
                indices.push_back(static_cast<uint16_t>(next + 1));
            }
        }
        
        // Create vertex buffer
        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex_t));
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbDesc.CPUAccessFlags = 0;
        
        D3D11_SUBRESOURCE_DATA vbData = {};
        vbData.pSysMem = vertices.data();
        
        HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &m_materialSphereState.vertexBuffer);
        if (FAILED(hr)) return;
        
        // Create index buffer
        D3D11_BUFFER_DESC ibDesc = {};
        ibDesc.Usage = D3D11_USAGE_DEFAULT;
        ibDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint16_t));
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibDesc.CPUAccessFlags = 0;
        
        D3D11_SUBRESOURCE_DATA ibData = {};
        ibData.pSysMem = indices.data();
        
        hr = device->CreateBuffer(&ibDesc, &ibData, &m_materialSphereState.indexBuffer);
        if (FAILED(hr)) return;
        
        m_materialSphereState.indexCount = static_cast<UINT>(indices.size());
        m_materialSphereState.vertexStride = sizeof(Vertex_t);
    }

    void LayoutManager::DestroySphereGeometry()
    {
        if (m_materialSphereState.indexBuffer) {
            m_materialSphereState.indexBuffer->Release();
            m_materialSphereState.indexBuffer = nullptr;
        }
        if (m_materialSphereState.vertexBuffer) {
            m_materialSphereState.vertexBuffer->Release();
            m_materialSphereState.vertexBuffer = nullptr;
        }
        m_materialSphereState.indexCount = 0;
        m_materialSphereState.vertexStride = 0;
    }

    void LayoutManager::RenderMaterialSphereToTexture(const void* materialData)
    {
        const MaterialAsset* material = reinterpret_cast<const MaterialAsset*>(materialData);
        
        if (!material || !g_dxHandler || !m_materialSphereState.renderTargetView) {
            return;
        }
        
        ID3D11DeviceContext* context = g_dxHandler->GetDeviceContext();
        CDXCamera* camera = g_dxHandler->GetCamera();
        
        // Save original render targets
        ID3D11RenderTargetView* originalRTV = nullptr;
        ID3D11DepthStencilView* originalDSV = nullptr;
        context->OMGetRenderTargets(1, &originalRTV, &originalDSV);
        
        // Save original viewport
        UINT numViewports = 1;
        D3D11_VIEWPORT originalViewport;
        context->RSGetViewports(&numViewports, &originalViewport);
        
        // Set our render target
        context->OMSetRenderTargets(1, &m_materialSphereState.renderTargetView, m_materialSphereState.depthStencilView);
        
        // Clear render target (dark background)
        float clearColor[4] = {0.1f, 0.1f, 0.15f, 1.0f};
        context->ClearRenderTargetView(m_materialSphereState.renderTargetView, clearColor);
        context->ClearDepthStencilView(m_materialSphereState.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        // Set viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = static_cast<float>(m_materialSphereState.renderWidth);
        viewport.Height = static_cast<float>(m_materialSphereState.renderHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context->RSSetViewports(1, &viewport);
        
        // Set render states
        context->RSSetState(g_dxHandler->GetRasterizerState());
        context->OMSetDepthStencilState(g_dxHandler->GetDepthStencilState(), 1u);
        
        // Update rotation
        if (m_materialSphereState.autoRotate) {
            m_materialSphereState.currentRotation += m_materialSphereState.rotationSpeed * 0.016f; // Assume ~60fps
            if (m_materialSphereState.currentRotation > 2.0f * XM_PI) {
                m_materialSphereState.currentRotation -= 2.0f * XM_PI;
            }
        }
        
        // Setup camera for sphere (looking at origin)
        camera->CommitCameraDataBufferUpdates();
        
        // Create world matrix with rotation and scale
        XMMATRIX world = XMMatrixScaling(m_materialSphereState.sphereScale, m_materialSphereState.sphereScale, m_materialSphereState.sphereScale) *
                        XMMatrixRotationY(m_materialSphereState.currentRotation);
        
        // Create view matrix (camera looking at sphere from distance)
        XMMATRIX view = XMMatrixLookAtLH(
            XMVectorSet(0, 0, -3, 1),  // Camera position
            XMVectorSet(0, 0, 0, 1),   // Look at origin
            XMVectorSet(0, 1, 0, 0)    // Up vector
        );
        
        // Create projection matrix
        float aspectRatio = static_cast<float>(m_materialSphereState.renderWidth) / static_cast<float>(m_materialSphereState.renderHeight);
        XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
        
        // Setup lighting (similar to model viewer)
        CDXScene& scene = g_dxHandler->GetScene();
        
        if (scene.globalLights.size() == 0) {
            HardwareLight& keyLight = scene.globalLights.emplace_back();
            keyLight.pos = {2, 3, -2};
            keyLight.rcpMaxRadius = 1 / 50.f;
            keyLight.rcpMaxRadiusSq = 1 / (keyLight.rcpMaxRadius * keyLight.rcpMaxRadius);
            keyLight.attenLinear = -1.95238f;
            keyLight.attenQuadratic = 0.95238f;
            keyLight.specularIntensity = 1.f;
            keyLight.color = {1.0f, 0.95f, 0.9f}; // Warm key light
            
            HardwareLight& fillLight = scene.globalLights.emplace_back();
            fillLight.pos = {-1, 1, 2};
            fillLight.rcpMaxRadius = 1 / 30.f;
            fillLight.rcpMaxRadiusSq = 1 / (fillLight.rcpMaxRadius * fillLight.rcpMaxRadius);
            fillLight.attenLinear = -1.95238f;
            fillLight.attenQuadratic = 0.95238f;
            fillLight.specularIntensity = 0.3f;
            fillLight.color = {0.7f, 0.8f, 1.0f}; // Cool fill light
        }
        
        if (scene.NeedsLightingUpdate()) {
            scene.CreateOrUpdateLights(g_dxHandler->GetDevice(), context);
        }
        
        // Create working shader source code for the material sphere
        static const char* sphereVertexShader = 
            "struct VS_Input\n"
            "{\n"
            "    float3 position : POSITION;\n"
            "    uint normal : NORMAL;\n"
            "    uint color : COLOR;\n"
            "    float2 uv : TEXCOORD;\n"
            "};\n"
            "struct VS_Output\n"
            "{\n"
            "    float4 position : SV_POSITION;\n"
            "    float3 worldPosition : POSITION;\n"
            "    float4 color : COLOR;\n"
            "    float3 normal : NORMAL;\n"
            "    float2 uv : TEXCOORD;\n"
            "};\n"
            "cbuffer VS_TransformConstants : register(b0)\n"
            "{\n"
            "    float4x4 modelMatrix;\n"
            "    float4x4 viewMatrix;\n"
            "    float4x4 projectionMatrix;\n"
            "};\n"
            "VS_Output vs_main(VS_Input input)\n"
            "{\n"
            "    VS_Output output;\n"
            "    float3 pos = input.position;\n"
            "    output.position = mul(float4(pos, 1.f), modelMatrix);\n"
            "    output.position = mul(output.position, viewMatrix);\n"
            "    output.position = mul(output.position, projectionMatrix);\n"
            "    output.worldPosition = mul(float4(input.position, 1.f), modelMatrix).xyz;\n"
            "    output.color = float4(1.0, 1.0, 1.0, 1.0);\n"
            "    output.normal = normalize(input.position);\n"
            "    output.uv = input.uv;\n"
            "    return output;\n"
            "}";
            
        static const char* spherePixelShader = 
            "struct VS_Output\n"
            "{\n"
            "    float4 position : SV_POSITION;\n"
            "    float3 worldPosition : POSITION;\n"
            "    float4 color : COLOR;\n"
            "    float3 normal : NORMAL;\n"
            "    float2 uv : TEXCOORD;\n"
            "};\n"
            "Texture2D baseTexture : register(t0);\n"
            "SamplerState texSampler : register(s0);\n"
            "float4 ps_main(VS_Output input) : SV_Target\n"
            "{\n"
            "    float3 lightDir = normalize(float3(1, 1, -1));\n"
            "    float3 normal = normalize(input.normal);\n"
            "    float ndotl = max(0.0, dot(normal, lightDir));\n"
            "    \n"
            "    // Sample the texture\n"
            "    float4 texColor = baseTexture.Sample(texSampler, input.uv);\n"
            "    \n"
            "    // If texture sampling failed or returns near-black, use debug pattern\n"
            "    if (length(texColor.rgb) < 0.1) {\n"
            "        // UV-based checker pattern for debugging\n"
            "        float checker = step(0.5, frac(input.uv.x * 8.0)) * step(0.5, frac(input.uv.y * 8.0));\n"
            "        checker += (1.0 - step(0.5, frac(input.uv.x * 8.0))) * (1.0 - step(0.5, frac(input.uv.y * 8.0)));\n"
            "        texColor = float4(checker * 0.8 + 0.2, 0.2, 0.8, 1.0);\n"
            "    }\n"
            "    \n"
            "    float3 finalColor = texColor.rgb * (0.3 + 0.7 * ndotl);\n"
            "    return float4(finalColor, texColor.a);\n"
            "}";
        
        // Use the corrected shader source code
        CShader* vertexShader = g_dxHandler->GetShaderManager()->LoadShaderFromString("sphere_vs", sphereVertexShader, eShaderType::Vertex);
        CShader* pixelShader = g_dxHandler->GetShaderManager()->LoadShaderFromString("sphere_ps", spherePixelShader, eShaderType::Pixel);
        
        if (vertexShader && pixelShader && m_materialSphereState.vertexBuffer && m_materialSphereState.indexBuffer) {
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->IASetInputLayout(vertexShader->GetInputLayout());
            
            context->VSSetShader(vertexShader->Get<ID3D11VertexShader>(), nullptr, 0u);
            context->PSSetShader(pixelShader->Get<ID3D11PixelShader>(), nullptr, 0u);
            
            // Create and bind transform buffer
            ID3D11Buffer* transformBuffer = nullptr;
            D3D11_BUFFER_DESC desc = {};
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = sizeof(VS_TransformConstants);
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            
            VS_TransformConstants transforms;
            transforms.modelMatrix = XMMatrixTranspose(world);
            transforms.viewMatrix = XMMatrixTranspose(view);
            transforms.projectionMatrix = XMMatrixTranspose(projection);
            
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = &transforms;
            
            if (SUCCEEDED(g_dxHandler->GetDevice()->CreateBuffer(&desc, &initData, &transformBuffer))) {
                context->VSSetConstantBuffers(0u, 1u, &transformBuffer);
                
                // Set vertex and index buffers
                UINT offset = 0;
                context->IASetVertexBuffers(0u, 1u, &m_materialSphereState.vertexBuffer, &m_materialSphereState.vertexStride, &offset);
                context->IASetIndexBuffer(m_materialSphereState.indexBuffer, DXGI_FORMAT_R16_UINT, 0u);
                
                // Bind material textures
                ID3D11SamplerState* samplerState = g_dxHandler->GetSamplerState();
                context->PSSetSamplers(0, 1, &samplerState);
                
                // Bind lighting
                scene.BindLightsSRV(context);
                
                // Bind material textures to appropriate slots
                int texturesBound = 0;
                for (const auto& entry : material->txtrAssets) {
                    if (entry.asset && entry.asset->extraData()) {
                        TextureAsset* txtr = reinterpret_cast<TextureAsset*>(entry.asset->extraData());
                        if (!txtr->mipArray.empty()) {
                            // Try different mip levels if the highest quality one fails
                            std::shared_ptr<CTexture> texture = nullptr;
                            
                            // Try highest quality first (last mip)
                            const auto& highestMip = txtr->mipArray[txtr->mipArray.size() - 1];
                            if (highestMip.isLoaded) {
                                texture = CreateTextureFromMip(entry.asset, &highestMip, s_PakToDxgiFormat[txtr->imgFormat]);
                            }
                            
                            // If that failed, try other mips
                            if (!texture && txtr->mipArray.size() > 1) {
                                for (int mipIdx = static_cast<int>(txtr->mipArray.size()) - 2; mipIdx >= 0; --mipIdx) {
                                    const auto& mip = txtr->mipArray[mipIdx];
                                    if (mip.isLoaded) {
                                        texture = CreateTextureFromMip(entry.asset, &mip, s_PakToDxgiFormat[txtr->imgFormat]);
                                        if (texture) break;
                                    }
                                }
                            }
                            
                            if (texture) {
                                ID3D11ShaderResourceView* srv = texture->GetSRV();
                                if (srv) {
                                    // Always bind to register t0 for the main texture (albedo/diffuse)
                                    context->PSSetShaderResources(0, 1u, &srv);
                                    texturesBound++;
                                    break; // Use the first valid texture for now
                                }
                            }
                        }
                    }
                }
                
                // If no textures bound, bind a white dummy texture or let shader handle it
                if (texturesBound == 0) {
                    // Create a simple 1x1 white texture as fallback
                    D3D11_TEXTURE2D_DESC texDesc = {};
                    texDesc.Width = 1;
                    texDesc.Height = 1;
                    texDesc.MipLevels = 1;
                    texDesc.ArraySize = 1;
                    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    texDesc.SampleDesc.Count = 1;
                    texDesc.Usage = D3D11_USAGE_DEFAULT;
                    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                    
                    uint32_t whitePixel = 0xFFFFFFFF; // White pixel
                    initData = {};
                    initData.pSysMem = &whitePixel;
                    initData.SysMemPitch = 4;
                    
                    ID3D11Texture2D* whiteTex = nullptr;
                    ID3D11ShaderResourceView* whiteSRV = nullptr;
                    
                    if (SUCCEEDED(g_dxHandler->GetDevice()->CreateTexture2D(&texDesc, &initData, &whiteTex))) {
                        if (SUCCEEDED(g_dxHandler->GetDevice()->CreateShaderResourceView(whiteTex, nullptr, &whiteSRV))) {
                            context->PSSetShaderResources(0, 1u, &whiteSRV);
                        }
                        if (whiteSRV) whiteSRV->Release();
                        if (whiteTex) whiteTex->Release();
                    }
                }
                
                // Draw the sphere
                context->DrawIndexed(m_materialSphereState.indexCount, 0u, 0u);
                
                transformBuffer->Release();
            }
        }
        
        // Restore original render targets and viewport
        context->OMSetRenderTargets(1, &originalRTV, originalDSV);
        context->RSSetViewports(1, &originalViewport);

        if (originalRTV) originalRTV->Release();
        if (originalDSV) originalDSV->Release();
    }

    // ================================================================================================
    // Tab Management (Godot-style)
    // ================================================================================================

    void LayoutManager::OpenAssetInTab(CAsset* asset)
    {
        if (!asset) return;

        // Check if asset is already open in a tab
        for (size_t i = 0; i < m_openTabs.size(); ++i)
        {
            if (m_openTabs[i].asset == asset)
            {
                SetActiveTab(i);
                return;
            }
        }

        // Determine tab type based on asset type
        TabType tabType = TabType::Generic;
        try {
            uint32_t assetType = asset->GetAssetType();

            // Check for 3D model types (match what RenderModelViewer3D expects)
            if (assetType == 0x5F6C646D || // '_ldm' (MDL_)
                assetType == 0x67697261 || // 'arig' (ARIG)
                assetType == 0x71657361 || // 'aseq' (ASEQ)
                assetType == 0x006C646D || // 'mdl\0' (MDL)
                assetType == 0x6C646D72)   // 'rmdl' - Additional model type
            {
                tabType = TabType::Model3D;
            }
            // Check for texture types: 'rtxt' or 'txtr'
            else if (assetType == 0x72747874 || assetType == 0x74787472) {
                tabType = TabType::Texture;
            }
            // Check for material type: 'matl'
            else if (assetType == 0x6C74616D) {
                tabType = TabType::Material;
            }
        } catch (...) {
            // Default to generic if we can't determine type
        }

        // Create new tab with just the filename
        std::string fullPath = asset->GetAssetName();
        std::string tabName;

        if (!fullPath.empty()) {
            // Extract filename from full path
            size_t lastSlash = fullPath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                tabName = fullPath.substr(lastSlash + 1);
            } else {
                tabName = fullPath; // No path separators, use as-is
            }
        }

        if (tabName.empty()) {
            tabName = "Unnamed Asset";
        }

        m_openTabs.emplace_back(tabName, asset, tabType);
        size_t newTabIndex = m_openTabs.size() - 1;
        SetActiveTab(newTabIndex);
        m_forceSelectTabIndex = newTabIndex; // Force ImGui to select this specific tab on next render
    }

    void LayoutManager::CloseTab(size_t tabIndex)
    {
        if (tabIndex >= m_openTabs.size()) return;

        m_openTabs.erase(m_openTabs.begin() + tabIndex);

        // Adjust active tab index
        if (m_openTabs.empty()) {
            m_activeTabIndex = 0;
        } else if (m_activeTabIndex >= m_openTabs.size()) {
            m_activeTabIndex = m_activeTabIndex - 1;
        } else if (tabIndex <= m_activeTabIndex && m_activeTabIndex > 0) {
            m_activeTabIndex--;
        }
    }

    void LayoutManager::SetActiveTab(size_t tabIndex)
    {
        if (tabIndex >= m_openTabs.size()) return;

        // Set all tabs to inactive first
        for (auto& tab : m_openTabs) {
            tab.isActive = false;
        }

        // Set the selected tab as active
        m_openTabs[tabIndex].isActive = true;
        m_activeTabIndex = tabIndex;

        // Update selected assets to match active tab
        std::vector<CAsset*> newSelection = { m_openTabs[tabIndex].asset };
        SetSelectedAssets(newSelection);
    }

    size_t LayoutManager::GetActiveTabIndex() const
    {
        return m_activeTabIndex;
    }

    void LayoutManager::RenderTabbedCenterPanel()
    {
        // Show message if no tabs are open
        if (m_openTabs.empty())
        {
            ImVec2 centerPos = ImVec2(
                ImGui::GetContentRegionAvail().x * 0.5f,
                ImGui::GetContentRegionAvail().y * 0.5f
            );
            ImGui::SetCursorPos(centerPos);
            ImGui::PushStyleColor(ImGuiCol_Text, GetSecondaryColor());
            ImGui::Text("No assets open");
            ImGui::PopStyleColor();

            ImGui::SetCursorPosY(centerPos.y + 30);
            ImGui::SetCursorPosX(centerPos.x - 80);
            ImGui::Text("Double-click an asset to open it in a tab");
            return;
        }

        // Render tab bar
        if (ImGui::BeginTabBar("AssetTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_TabListPopupButton))
        {
            for (size_t i = 0; i < m_openTabs.size(); ++i)
            {
                OpenTab& tab = m_openTabs[i];
                bool isOpen = true;

                // Create tab name with close button
                std::string tabLabel = tab.name;

                // Add type icon
                switch (tab.type)
                {
                    case TabType::Model3D:
                        tabLabel = "[mdl]" + tabLabel;
                        break;
                    case TabType::Texture:
                        tabLabel = "[txtr]" + tabLabel;
                        break;
                    case TabType::Material:
                        tabLabel = "[mat]" + tabLabel;
                        break;
                }

                // Use ImGui flags to set the selected tab
                ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
                if (i == m_forceSelectTabIndex) {
                    flags = ImGuiTabItemFlags_SetSelected;
                }

                if (ImGui::BeginTabItem(tabLabel.c_str(), &isOpen, flags))
                {
                    if (m_activeTabIndex != i) {
                        SetActiveTab(i);
                    }

                    // Render the content for this tab
                    RenderTabContent(tab);

                    ImGui::EndTabItem();
                }

                // Handle tab close
                if (!isOpen) {
                    CloseTab(i);
                    break; // Exit loop since we modified the vector
                }
            }

            ImGui::EndTabBar();
        }

        // Reset the force selection index after rendering
        m_forceSelectTabIndex = SIZE_MAX;
    }

    void LayoutManager::RenderTabContent(const OpenTab& tab)
    {
        if (!tab.asset) {
            ImGui::Text("Invalid asset");
            return;
        }

        // Render content based on tab type
        switch (tab.type)
        {
            case TabType::Model3D:
                RenderModelViewer3D();
                break;

            case TabType::Texture:
                RenderTextureViewer(tab.asset);
                break;

            case TabType::Material:
                {
                    // Temporarily set the asset as selected so RenderMaterialPreview can find it
                    std::vector<CAsset*> originalSelection = m_selectedAssets;
                    m_selectedAssets = { tab.asset };
                    RenderMaterialPreview();
                    m_selectedAssets = originalSelection;
                }
                break;

            case TabType::Generic:
            default:
                {
                    // Show basic asset info
                    ImGui::Text("Asset: %s", tab.asset->GetAssetName().c_str());
                    ImGui::Text("Type: 0x%08X", tab.asset->GetAssetType());

                    // Try to show some content if possible
                    try {
                        uint32_t assetType = tab.asset->GetAssetType();

                        // Try texture viewer for unknown texture-like assets
                        if (assetType == 0x72747874 || assetType == 0x74787472) {
                            RenderTextureViewer(tab.asset);
                        }
                        // Try material preview for material-like assets
                        else if (assetType == 0x6C74616D) {
                            std::vector<CAsset*> savedSelection = m_selectedAssets;
                            m_selectedAssets = { tab.asset };
                            RenderMaterialPreview();
                            m_selectedAssets = savedSelection;
                        }
                        else {
                            ImGui::Separator();
                            ImGui::Text("Preview not available for this asset type");
                        }
                    } catch (...) {
                        ImGui::Text("Unable to preview this asset");
                    }
                }
                break;
        }
    }
}

