#pragma once
#include <thirdparty/imgui/imgui.h>
#include <thirdparty/imgui/imgui_internal.h>
#include <vector>
#include <string>
#include <functional>
#include <core/math/vector.h>
#include <core/math/mathlib.h>
#include <d3d11.h>

// Forward declarations
class CAsset;
class CGlobalAssetData;
class CDXCamera;
struct ID3D11DeviceContext;

namespace ModernUI
{
    // UI Panel Types
    enum class PanelType
    {
        AssetBrowser,
        AssetInspector,
        Viewport3D,
        ModelViewer3D,      // New dedicated 3D model viewer
        AssetPreview,
        Properties,
        Console
    };

    // Asset tree node for hierarchical display
    struct AssetTreeNode
    {
        std::string name;           // Display name for this node (e.g., "barriers", "shooting_range_target_02_stand.rmdl")
        std::string fullPath;       // Full path from root (e.g., "mdl/barriers")
        std::vector<CAsset*> assets; // Assets directly in this folder (files)
        std::vector<AssetTreeNode> children; // Subdirectories
        bool isExpanded = false;    // Default to collapsed for better performance
        bool isDirectory = true;    // true for directories, false for individual assets
        
        AssetTreeNode() = default;
        AssetTreeNode(const std::string& n, const std::string& path = "", bool isDir = true) 
            : name(n), fullPath(path), isDirectory(isDir) {}
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
        void RenderModelViewer3D();    // New dedicated 3D model viewer
        void RenderAssetPreview();
        void RenderProperties();
        void RenderConsole();
        
        // Asset tree functions
        void BuildAssetTree();
        void RenderAssetTreeNode(AssetTreeNode& node);
        void RenderAssetTable(const std::vector<CAsset*>& assets);
        
        // 3D Model Viewer render-to-texture functions
        bool CreateModelViewerRenderTarget(int width, int height);
        void DestroyModelViewerRenderTarget();
        void RenderModelToTexture();
        void ResizeModelViewerRenderTarget(int width, int height);
        void RenderGridFloor(ID3D11DeviceContext* context, CDXCamera* camera);
        
        // Safe texture preview
        void RenderSafeTexturePreview(void* pakAsset);
        void RenderMinimalTextureImage(void* pakAsset, void* txtrAsset);
        void RenderTextureViewer(CAsset* textureAsset);
        void RenderMaterialViewer(CAsset* materialAsset);
        void RenderModel3DViewport();
        
        // UI State
        bool m_panelVisible[7]; // One for each PanelType (updated for ModelViewer3D)
        std::vector<CAsset*> m_selectedAssets;
        AssetTreeNode m_assetTreeRoot;
        bool m_showAssetTreeView = true;
        bool m_showAssetTableView = false;
        
        // 3D Model Viewer State
        struct ModelViewerState {
            bool freecamEnabled = false;
            float cameraSpeed = 1.0f;
            bool showWireframe = false;
            bool showLighting = true;
            bool showSkybox = true;
            bool useDefaultTexture = false;
            bool showShadows = true;
            bool showGridFloor = true;
            bool autoRotate = false;
            float autoRotateSpeed = 0.5f;
            Vector cameraPosition = {0, 0, -5};
            Vector cameraRotation = {0, 0, 0};
            
            // Render-to-texture resources
            ID3D11Texture2D* renderTexture = nullptr;
            ID3D11RenderTargetView* renderTargetView = nullptr;
            ID3D11ShaderResourceView* shaderResourceView = nullptr;
            ID3D11Texture2D* depthTexture = nullptr;
            ID3D11DepthStencilView* depthStencilView = nullptr;
            int renderWidth = 800;
            int renderHeight = 600;
        } m_modelViewerState;
        
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
