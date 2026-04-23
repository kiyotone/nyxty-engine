#pragma once

#include "core/foundation/Base.h"

#include <cstdint>
#include <limits>
#include <vector>

namespace Nyxty {

    template<typename Tag>
    struct Handle {
        u32 index{ std::numeric_limits<u32>::max() };
        u32 generation{ 0 };

        explicit operator bool() const {
            return index != std::numeric_limits<u32>::max();
        }

        friend bool operator==(const Handle& lhs, const Handle& rhs) {
            return lhs.index == rhs.index && lhs.generation == rhs.generation;
        }
    };

    template<typename Tag>
    class HandlePool {
    public:
        Handle<Tag> Allocate() {
            if (!m_Free.empty()) {
                const u32 index = m_Free.back();
                m_Free.pop_back();
                return Handle<Tag>{ index, m_Generations[index] };
            }

            const u32 index = static_cast<u32>(m_Generations.size());
            m_Generations.push_back(1);
            return Handle<Tag>{ index, 1 };
        }

        void Release(Handle<Tag> handle) {
            if (!IsAlive(handle)) {
                return;
            }

            ++m_Generations[handle.index];
            m_Free.push_back(handle.index);
        }

        bool IsAlive(Handle<Tag> handle) const {
            return handle.index < m_Generations.size() &&
                m_Generations[handle.index] == handle.generation &&
                handle.generation != 0;
        }

    private:
        std::vector<u32> m_Generations;
        std::vector<u32> m_Free;
    };

} // namespace Nyxty
