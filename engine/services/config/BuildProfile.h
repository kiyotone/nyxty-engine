#pragma once

#include "core/foundation/Core.h"

#include <string>

namespace Nyxty {

    enum class BuildProfileKind {
        Debug,
        Release,
        Shipping
    };

    struct BuildProfile {
        BuildProfileKind kind{ BuildProfileKind::Debug };
        bool verboseLogging{ true };
        bool assertionsEnabled{ true };
        bool developerToolsEnabled{ true };
    };

    NYXTY_CORE_API BuildProfile BuildProfileFromName(const std::string& name);
    NYXTY_CORE_API const char* ToString(BuildProfileKind kind);

} // namespace Nyxty
