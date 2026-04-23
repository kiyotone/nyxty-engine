#include "scripting/runtime/ScriptHost.h"

#include <utility>

namespace Nyxty {

void ScriptHost::RegisterCommand(std::string name, Command command) {
    if (!command) {
        m_Commands.erase(name);
        return;
    }

    m_Commands[std::move(name)] = std::move(command);
}

ScriptCommandResult ScriptHost::Execute(const std::string& name, const std::vector<std::string>& args) const {
    const auto it = m_Commands.find(name);
    if (it == m_Commands.end()) {
        return ScriptCommandResult{ false, "command not found" };
    }
    return it->second(args);
}

bool ScriptHost::HasCommand(const std::string& name) const {
    return m_Commands.contains(name);
}

} // namespace Nyxty
