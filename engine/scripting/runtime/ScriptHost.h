#pragma once

#include "core/foundation/Core.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Nyxty {

    struct ScriptCommandResult {
        bool success{ false };
        std::string output;
    };

    class NYXTY_CORE_API ScriptHost {
    public:
        using Command = std::function<ScriptCommandResult(const std::vector<std::string>&)>;

        void RegisterCommand(std::string name, Command command);
        ScriptCommandResult Execute(const std::string& name, const std::vector<std::string>& args = {}) const;
        bool HasCommand(const std::string& name) const;

    private:
        std::unordered_map<std::string, Command> m_Commands;
    };

} // namespace Nyxty
