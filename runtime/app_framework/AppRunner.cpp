#include "runtime/app_framework/AppRunner.h"

#include "runtime/app_framework/IApp.h"

namespace Nyxty {

int AppRunner::Run(IApp& app) const {
    return app.Run();
}

} // namespace Nyxty
