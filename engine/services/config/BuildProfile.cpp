#include "services/config/BuildProfile.h"

#include <algorithm>
#include <cctype>

namespace Nyxty {

namespace {
    std::string Normalize(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }
}

BuildProfile BuildProfileFromName(const std::string& name) {
    const std::string normalized = Normalize(name);
    if (normalized == "shipping") {
        return BuildProfile{ BuildProfileKind::Shipping, false, false, false };
    }
    if (normalized == "release") {
        return BuildProfile{ BuildProfileKind::Release, false, false, true };
    }
    return BuildProfile{ BuildProfileKind::Debug, true, true, true };
}

const char* ToString(BuildProfileKind kind) {
    switch (kind) {
    case BuildProfileKind::Debug: return "debug";
    case BuildProfileKind::Release: return "release";
    case BuildProfileKind::Shipping: return "shipping";
    }
    return "unknown";
}

} // namespace Nyxty
