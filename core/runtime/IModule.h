#pragma once

#include "core/foundation/Base.h"
#include "core/foundation/Core.h"

#include <string>
#include <vector>

enum class ModuleInspectorFieldKind {
    Label,
    Toggle,
    TextInput,
    IntegerInput,
    FloatInput,
    Choice,
    Action,
    MultilineLabel,
    MultilineTextInput,
};

struct ModuleInspectorChoiceOption {
    std::string value;
    std::string label;
};

struct ModuleInspectorField {
    std::string id;
    std::string label;
    std::string value;
    std::string help;
    ModuleInspectorFieldKind kind{ ModuleInspectorFieldKind::Label };
    bool readOnly{ false };
    bool sensitive{ false };
    float step{ 1.0f };
    std::vector<ModuleInspectorChoiceOption> options;
};

struct ModuleInspectorSection {
    std::string id;
    std::string title;
    std::string summary;
    std::vector<ModuleInspectorField> fields;
};

struct ModuleInspectorPanel {
    std::string id;
    std::string title;
    std::string summary;
    bool defaultOpen{ true };
    std::vector<ModuleInspectorSection> sections;
};

enum class ModulePaletteItemKind {
    Action,
    View,
    Resource,
    Other,
};

struct ModulePaletteItem {
    std::string id;
    std::string title;
    std::string subtitle;
    std::string category;
    std::string keywords;
    std::string copyText;
    ModulePaletteItemKind kind{ ModulePaletteItemKind::Other };
    bool pinned{ false };
    bool recent{ false };
    bool dangerous{ false };
    bool enabled{ true };
};

class IModule {
public:
    virtual ~IModule() = default;

    // Module identity
    virtual const char* GetName() = 0;

    // Version info
    virtual Nyxty::u32 GetVersionMajor() = 0;
    virtual Nyxty::u32 GetVersionMinor() = 0;
    virtual Nyxty::u32 GetVersionPatch() = 0;

    // Lifecycle
    virtual bool OnLoad() = 0;
    virtual void OnUnload() = 0;
    virtual void OnUpdate() {};

    // Module ID for event system
    virtual Nyxty::u32 GetModuleID() = 0;

    // Optional inspector extensions rendered by the engine UI.
    virtual std::vector<ModuleInspectorPanel> GetInspectorPanels() const { return {}; }
    virtual bool ApplyInspectorField(const std::string&, const std::string&, const std::string&) { return false; }

    // Optional command palette/search provider surface.
    virtual std::vector<ModulePaletteItem> GetPaletteItems() const { return {}; }
    virtual bool InvokePaletteItem(const std::string&, const std::string&) { return false; }

    // Optional debug UI panel. Return a non-null name to get a docked panel each frame.
    // DebugUI discovers panels by iterating ModuleLoader — no separate registration needed.
    virtual const char* GetPanelName() const { return nullptr; }
    virtual void        OnDrawUI()           {}
};

// Module factory functions (implemented in each module)
MODULE_EXPORT IModule* CreateModule();
MODULE_EXPORT void DestroyModule(IModule* module);
