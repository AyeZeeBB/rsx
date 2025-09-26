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
        MaterialPreview,    // New dedicated material sphere preview
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

    // Tab types for center panel (Godot-style)
    enum class TabType
    {
        Model3D,
        Texture,
        Material,
        Generic
    };

    // Open tab structure
    struct OpenTab
    {
        std::string name;           // Display name for the tab
        CAsset* asset;              // Associated asset
        TabType type;               // Type of viewer to use
        bool isActive;              // Currently active tab

        OpenTab(const std::string& n, CAsset* a, TabType t)
            : name(n), asset(a), type(t), isActive(false) {}
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

        // Tab management (Godot-style)
        void OpenAssetInTab(CAsset* asset);
        void CloseTab(size_t tabIndex);
        void SetActiveTab(size_t tabIndex);
        size_t GetActiveTabIndex() const;

    private:
        // Panel rendering functions
        void RenderMenuBar();
        void RenderMainLayout();
        void RenderAssetBrowser();
        void RenderAssetInspector();
        void RenderViewport3D();
        void RenderModelViewer3D();    // New dedicated 3D model viewer
        void RenderAssetPreview();
        void RenderMaterialPreview();  // New dedicated material sphere preview
        void RenderProperties();
        void RenderConsole();

        // Godot-style tabbed center panel
        void RenderTabbedCenterPanel();
        void RenderTabContent(const OpenTab& tab);
        
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
        
        // Material sphere preview
        void RenderMaterialSpherePreview(const void* material, CAsset* materialAsset);
        bool CreateMaterialSphereRenderTarget(int width, int height);
        void DestroyMaterialSphereRenderTarget();
        void CreateSphereGeometry();
        void DestroySphereGeometry();
        void RenderMaterialSphereToTexture(const void* material);
        void RenderMaterialViewer(CAsset* materialAsset);
        void RenderModel3DViewport();

        // Skybox functions
        bool CreateSkyboxResources();
        void DestroySkyboxResources();
        void RenderSkybox(ID3D11DeviceContext* context);
        
        // UI State
        bool m_panelVisible[8]; // One for each PanelType (updated for MaterialPreview)
        std::vector<CAsset*> m_selectedAssets;
        AssetTreeNode m_assetTreeRoot;
        bool m_showAssetTreeView = true;
        bool m_showAssetTableView = false;
        bool m_treeNeedsRebuild = true;
        bool m_lastJobActionState = false; // Track PAK loading state

        // Tab management (Godot-style)
        std::vector<OpenTab> m_openTabs;
        size_t m_activeTabIndex = 0;
        size_t m_forceSelectTabIndex = SIZE_MAX; // Index of tab to force select (SIZE_MAX = none)
        
        // 3D Model Viewer State
        struct ModelViewerState {
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
            float cameraFOV = 75.0f;  // Field of view in degrees
            float skyboxScale = 50.0f; // Skybox cube size
            
            // Render-to-texture resources
            ID3D11Texture2D* renderTexture = nullptr;
            ID3D11RenderTargetView* renderTargetView = nullptr;
            ID3D11ShaderResourceView* shaderResourceView = nullptr;
            ID3D11Texture2D* depthTexture = nullptr;
            ID3D11DepthStencilView* depthStencilView = nullptr;
            int renderWidth = 800;
            int renderHeight = 600;

            // Skybox resources
            ID3D11Texture2D* skyboxTexture = nullptr;
            ID3D11ShaderResourceView* skyboxSRV = nullptr;
            ID3D11Buffer* skyboxVertexBuffer = nullptr;
            ID3D11Buffer* skyboxIndexBuffer = nullptr;
            UINT skyboxIndexCount = 0;
        } m_modelViewerState;
        
        // Material Sphere Preview State
        struct MaterialSphereState {
            bool autoRotate = true;
            float rotationSpeed = 1.0f;
            float sphereScale = 1.0f;
            float currentRotation = 0.0f;
            bool showWireframe = false;
            bool showLighting = true;
            
            // Render-to-texture resources
            ID3D11Texture2D* renderTexture = nullptr;
            ID3D11RenderTargetView* renderTargetView = nullptr;
            ID3D11ShaderResourceView* shaderResourceView = nullptr;
            ID3D11Texture2D* depthTexture = nullptr;
            ID3D11DepthStencilView* depthStencilView = nullptr;
            int renderWidth = 400;
            int renderHeight = 400;
            
            // Sphere geometry
            ID3D11Buffer* vertexBuffer = nullptr;
            ID3D11Buffer* indexBuffer = nullptr;
            UINT indexCount = 0;
            UINT vertexStride = 0;
        } m_materialSphereState;
        
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
