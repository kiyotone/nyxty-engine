#pragma once

namespace Nyxty {

class IApp;

class AppRunner {
public:
    int Run(IApp& app) const;
};

} // namespace Nyxty
