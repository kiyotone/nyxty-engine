#pragma once

#include "core/foundation/Base.h"
#include "core/foundation/Core.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <limits>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>

namespace Nyxty {

    class NYXTY_CORE_API TaskSystem {
    public:
        using Task = std::function<void()>;

        TaskSystem() = default;
        ~TaskSystem();

        void Start(u32 workerCount = 0);
        void Stop();

        void EnqueueWorker(Task task);
        void EnqueueMainThread(Task task);
        u32 DrainMainThread(u32 maxTasks = std::numeric_limits<u32>::max());

        bool IsRunning() const { return m_Running; }
        u32 GetWorkerCount() const { return static_cast<u32>(m_Workers.size()); }
        size_t GetPendingWorkerTaskCount() const;
        size_t GetPendingMainThreadTaskCount() const;

        template<typename Fn>
        auto Submit(Fn&& fn) -> std::future<std::invoke_result_t<Fn>> {
            using Result = std::invoke_result_t<Fn>;

            auto task = std::make_shared<std::packaged_task<Result()>>(std::forward<Fn>(fn));
            std::future<Result> future = task->get_future();
            EnqueueWorker([task]() { (*task)(); });
            return future;
        }

    private:
        void WorkerLoop();

        mutable std::mutex m_WorkerMutex;
        mutable std::mutex m_MainMutex;
        std::condition_variable m_WorkerSignal;
        std::queue<Task> m_WorkerQueue;
        std::queue<Task> m_MainQueue;
        std::vector<std::thread> m_Workers;
        bool m_Running{ false };
    };

} // namespace Nyxty
