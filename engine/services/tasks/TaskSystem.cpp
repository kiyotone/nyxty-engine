#include "services/tasks/TaskSystem.h"

#include "core/logging/Log.h"

#include <algorithm>

namespace Nyxty {

TaskSystem::~TaskSystem() {
    Stop();
}

void TaskSystem::Start(u32 workerCount) {
    if (m_Running) {
        return;
    }

    if (workerCount == 0) {
        const unsigned int hardware = std::thread::hardware_concurrency();
        workerCount = hardware > 1 ? hardware - 1 : 1;
    }

    m_Running = true;
    m_Workers.reserve(workerCount);
    for (u32 index = 0; index < workerCount; ++index) {
        m_Workers.emplace_back([this]() { WorkerLoop(); });
    }

    NYXTY_ENGINE_INFO("TaskSystem: started with {} worker(s).", workerCount);
}

void TaskSystem::Stop() {
    {
        std::scoped_lock lock(m_WorkerMutex);
        m_Running = false;
    }
    m_WorkerSignal.notify_all();

    for (std::thread& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_Workers.clear();
}

void TaskSystem::EnqueueWorker(Task task) {
    if (!task) {
        return;
    }

    {
        std::scoped_lock lock(m_WorkerMutex);
        m_WorkerQueue.push(std::move(task));
    }
    m_WorkerSignal.notify_one();
}

void TaskSystem::EnqueueMainThread(Task task) {
    if (!task) {
        return;
    }

    std::scoped_lock lock(m_MainMutex);
    m_MainQueue.push(std::move(task));
}

u32 TaskSystem::DrainMainThread(u32 maxTasks) {
    u32 drained = 0;

    while (drained < maxTasks) {
        Task task;
        {
            std::scoped_lock lock(m_MainMutex);
            if (m_MainQueue.empty()) {
                break;
            }
            task = std::move(m_MainQueue.front());
            m_MainQueue.pop();
        }

        task();
        ++drained;
    }

    return drained;
}

size_t TaskSystem::GetPendingWorkerTaskCount() const {
    std::scoped_lock lock(m_WorkerMutex);
    return m_WorkerQueue.size();
}

size_t TaskSystem::GetPendingMainThreadTaskCount() const {
    std::scoped_lock lock(m_MainMutex);
    return m_MainQueue.size();
}

void TaskSystem::WorkerLoop() {
    while (true) {
        Task task;
        {
            std::unique_lock lock(m_WorkerMutex);
            m_WorkerSignal.wait(lock, [this]() {
                return !m_Running || !m_WorkerQueue.empty();
            });

            if (!m_Running && m_WorkerQueue.empty()) {
                return;
            }

            task = std::move(m_WorkerQueue.front());
            m_WorkerQueue.pop();
        }

        task();
    }
}

} // namespace Nyxty
