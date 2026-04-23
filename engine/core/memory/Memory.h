#pragma once

#include "core/foundation/Base.h"
#include "core/foundation/Core.h"

#include <cstddef>
#include <vector>

namespace Nyxty {

    class NYXTY_CORE_API LinearAllocator {
    public:
        explicit LinearAllocator(size_t size = 0);

        void Resize(size_t size);
        void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t));
        void Reset();

        size_t GetCapacity() const { return m_Buffer.size(); }
        size_t GetUsed() const { return m_Offset; }

    private:
        std::vector<byte> m_Buffer;
        size_t m_Offset{ 0 };
    };

    class NYXTY_CORE_API StackAllocator {
    public:
        explicit StackAllocator(size_t size = 0);

        void Resize(size_t size);
        void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t));
        void FreeToMarker(size_t marker);
        void Reset();

        size_t GetMarker() const { return m_Offset; }
        size_t GetCapacity() const { return m_Buffer.size(); }
        size_t GetUsed() const { return m_Offset; }

    private:
        std::vector<byte> m_Buffer;
        size_t m_Offset{ 0 };
    };

    class NYXTY_CORE_API FixedBlockAllocator {
    public:
        FixedBlockAllocator(size_t blockSize = 0, size_t blockCount = 0);

        void Reset(size_t blockSize, size_t blockCount);
        void* Allocate();
        void Free(void* block);

        size_t GetBlockSize() const { return m_BlockSize; }
        size_t GetBlockCount() const { return m_BlockCount; }
        size_t GetFreeCount() const { return m_FreeList.size(); }

    private:
        size_t m_BlockSize{ 0 };
        size_t m_BlockCount{ 0 };
        std::vector<byte> m_Buffer;
        std::vector<size_t> m_FreeList;
    };

} // namespace Nyxty
