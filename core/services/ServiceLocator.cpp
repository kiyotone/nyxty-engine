#include "core/services/ServiceLocator.h"

namespace Nyxty {

std::unordered_map<std::string, ServiceLocator::Entry> ServiceLocator::s_Services;
std::mutex ServiceLocator::s_Mutex;

bool ServiceLocator::Has(const std::string& key) {
    std::scoped_lock lock(s_Mutex);
    return s_Services.contains(key);
}

void ServiceLocator::Unregister(const std::string& key) {
    std::scoped_lock lock(s_Mutex);
    s_Services.erase(key);
}

void ServiceLocator::Clear() {
    std::scoped_lock lock(s_Mutex);
    s_Services.clear();
}

} // namespace Nyxty
