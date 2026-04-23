#include "core/memory/Memory.h"

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace Nyxty {

namespace {
    size_t AlignForward(size_t value, size_t alignment) {
        if (alignment == 0) {
            return value;
        }

        const size_t mask = alignment - 1;
        return (value + mask) & ~mask;
    }
}

LinearAllocator::LinearAllocator(size_t size) {
    Resize(size);
}

void LinearAllocator::Resize(size_t size) {
    m_Buffer.resize(size);
    m_Offset = 0;
}

void* LinearAllocator::Allocate(size_t size, size_t alignment) {
    const size_t aligned = AlignForward(m_Offset, alignment);
    if (aligned + size > m_Buffer.size()) {
        return nullptr;
    }

    void* result = m_Buffer.data() + aligned;
    m_Offset = aligned + size;
    return result;
}

void LinearAllocator::Reset() {
    m_Offset = 0;
}

StackAllocator::StackAllocator(size_t size) {
    Resize(size);
}

void StackAllocator::Resize(size_t size) {
    m_Buffer.resize(size);
    m_Offset = 0;
}

void* StackAllocator::Allocate(size_t size, size_t alignment) {
    const size_t aligned = AlignForward(m_Offset, alignment);
    if (aligned + size > m_Buffer.size()) {
        return nullptr;
    }

    void* result = m_Buffer.data() + aligned;
    m_Offset = aligned + size;
    return result;
}

void StackAllocator::FreeToMarker(size_t marker) {
    m_Offset = std::min(marker, m_Buffer.size());
}

void StackAllocator::Reset() {
    m_Offset = 0;
}

FixedBlockAllocator::FixedBlockAllocator(size_t blockSize, size_t blockCount) {
    Reset(blockSize, blockCount);
}

void FixedBlockAllocator::Reset(size_t blockSize, size_t blockCount) {
    m_BlockSize = AlignForward(std::max(blockSize, sizeof(void*)), alignof(std::max_align_t));
    m_BlockCount = blockCount;
    m_Buffer.resize(m_BlockSize * m_BlockCount);
    m_FreeList.clear();
    m_FreeList.reserve(m_BlockCount);

    for (size_t index = 0; index < m_BlockCount; ++index) {
        m_FreeList.push_back(m_BlockCount - index - 1);
    }
}

void* FixedBlockAllocator::Allocate() {
    if (m_FreeList.empty() || m_Buffer.empty()) {
        return nullptr;
    }

    const size_t index = m_FreeList.back();
    m_FreeList.pop_back();
    return m_Buffer.data() + index * m_BlockSize;
}

void FixedBlockAllocator::Free(void* block) {
    if (!block || m_Buffer.empty()) {
        return;
    }

    auto* bytes = static_cast<byte*>(block);
    auto* begin = m_Buffer.data();
    auto* end = begin + m_Buffer.size();
    if (bytes < begin || bytes >= end) {
        assert(false && "block does not belong to FixedBlockAllocator");
        return;
    }

    const size_t offset = static_cast<size_t>(bytes - begin);
    if (offset % m_BlockSize != 0) {
        assert(false && "block is not aligned to allocator block size");
        return;
    }

    m_FreeList.push_back(offset / m_BlockSize);
}

} // namespace Nyxty
