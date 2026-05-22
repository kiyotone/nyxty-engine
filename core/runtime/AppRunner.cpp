#include "core/runtime/AppRunner.h"

#include "core/runtime/IApp.h"

namespace Nyxty {

int AppRunner::Run(IApp& app) const {
    return app.Run();
}

} // namespace Nyxty
