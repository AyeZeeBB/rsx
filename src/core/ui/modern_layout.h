#pragma once
#include <thirdparty/imgui/imgui.h>
#include <thirdparty/imgui/imgui_internal.h>
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class CAsset;
class CGlobalAssetData;

namespace ModernUI
{
    // UI Panel Types
    enum class PanelType
    {
        AssetBrowser,
        AssetInspector,
        Viewport3D,
        AssetPreview,
        Properties,
        Console
    };

    // Asset tree node for hierarchical display
    struct AssetTreeNode
    {
        std::string name;
        std::string category;
        std::vector<CAsset*> assets;
        std::vector<AssetTreeNode> children;
        bool isExpanded = true;
        bool isCategory = false;
        
        AssetTreeNode() = default;
        AssetTreeNode(const std::string& n, bool cat = false) : name(n), isCategory(cat) {}
    };

    // Modern UI Layout Manager
    class LayoutManager
    {
    public:
        LayoutManager();
        ~LayoutManager();

        // Main render function
        void Render();
        
        // Setup the layout
        void Initialize();
        
        // Panel management
        void SetPanelVisible(PanelType panel, bool visible);
        bool IsPanelVisible(PanelType panel) const;
        
        // Asset selection
        void SetSelectedAssets(const std::vector<CAsset*>& assets);
        const std::vector<CAsset*>& GetSelectedAssets() const { return m_selectedAssets; }
        
        // Tree view management
        void RefreshAssetTree();
        
    private:
        // Panel rendering functions
        void RenderMenuBar();
        void RenderMainLayout();
        void RenderAssetBrowser();
        void RenderAssetInspector();
        void RenderViewport3D();
        void RenderAssetPreview();
        void RenderProperties();
        void RenderConsole();
        
        // Asset tree functions
        void BuildAssetTree();
        void RenderAssetTreeNode(AssetTreeNode& node);
        void RenderAssetTable(const std::vector<CAsset*>& assets);
        
        // Safe texture preview
        void RenderSafeTexturePreview(void* pakAsset);
        void RenderMinimalTextureImage(void* pakAsset, void* txtrAsset);
        void RenderTextureViewer(CAsset* textureAsset);
        void RenderModel3DViewport();
        
        // UI State
        bool m_panelVisible[6]; // One for each PanelType
        std::vector<CAsset*> m_selectedAssets;
        AssetTreeNode m_assetTreeRoot;
        bool m_showAssetTreeView = true;
        bool m_showAssetTableView = false;
        
        // Layout settings
        float m_leftPanelWidth = 350.0f;
        float m_rightPanelWidth = 400.0f;
        float m_bottomPanelHeight = 200.0f;
        
        // Search and filtering
        char m_searchBuffer[256] = {};
        std::string m_filterCategory;
        
        // UI Helper functions
        void ApplyModernStyle();
        ImVec4 GetAccentColor() const;
        ImVec4 GetSecondaryColor() const;
        void DrawSeparatorWithText(const char* text);
        bool TreeNodeWithIcon(const char* icon, const char* label, bool* p_open = nullptr);
        // GetAssetTypeIcon removed to prevent crashes
    };

    // Global instance
    extern LayoutManager* g_pModernLayout;
}
