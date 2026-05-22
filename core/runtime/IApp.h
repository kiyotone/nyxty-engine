#pragma once

namespace Nyxty {

class IApp {
public:
    virtual ~IApp() = default;
    virtual int Run() = 0;
};

} // namespace Nyxty
