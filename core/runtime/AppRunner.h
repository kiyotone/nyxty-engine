#pragma once
#include "core/foundation/Core.h"

namespace Nyxty {

class IApp;

class NYXTY_CORE_API AppRunner {
public:
    int Run(IApp& app) const;
};

} // namespace Nyxty
