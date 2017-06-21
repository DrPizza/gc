#include "stdafx.h"

#include "gc.hpp"

namespace gc
{
	std::array<std::atomic<size_t>, get_max_arenas()> current_arena_nmt;
	std::array<arena, get_max_arenas()> arenas;

	global_data globals;

	thread_local thread_data this_gc_thread;

	collector the_gc;

	std::atomic<size_t> thread_data::mutator_thread_count;

	gc_bits gc_bits::_gc_build_reference(void* memory) {
		gc_bits unresolved = arenas[gc_bits::young]._gc_unresolve(memory);
		unresolved.bits.address.arena = gc_bits::young;
		unresolved.bits.address.nmt = current_arena_nmt[gc_bits::young].load(std::memory_order_acquire);
		unresolved.bits.header.colour = gc_bits::grey;
		return unresolved;
	}

	arena::arena() : page_usage(new std::array<std::atomic<size_t>, page_count>()) {
		ULARGE_INTEGER section_size = { 0 };
		section_size.QuadPart = size;
		section = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, section_size.HighPart, section_size.LowPart, nullptr);
		if(section == NULL) {
			throw std::bad_alloc();
		}
		base    = static_cast<unsigned char*>(::MapViewOfFile(section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart));
		rw_base = static_cast<unsigned char*>(::MapViewOfFile(section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart));
		high_watermark.store(0);
	}

	arena::~arena() {
		::UnmapViewOfFile(rw_base);
		::UnmapViewOfFile(base);
		::CloseHandle(section);
	}

	void* arena::allocate(size_t amount) {
		if(amount > available()) {
			return nullptr;
		}
		// base_offset is already maximally aligned.
		// we allocate sizeof(allocation_header) + padding to next alignment boundary + amount + padding to next alignment boundary
		static const size_t padded_header_size     = (sizeof(allocation_header) + (alignment - 1)) & ~(alignment - 1);
		       const size_t padded_allocation_size = (amount                    + (alignment - 1)) & ~(alignment - 1);
		       const size_t allocated_size = padded_header_size + padded_allocation_size;
		       const size_t requested_size = amount;

		if(allocated_size > available()) {
			return nullptr;
		}

		size_t base_offset = 0;
		allocation_header* header = nullptr;
		for(;;) {
			base_offset = high_watermark.fetch_add(allocated_size);
			if(base_offset + allocated_size > size) {
				// someone got in ahead of us and used the last of the available memory
				high_watermark.fetch_sub(allocated_size);
				// TODO force collection and compaction, try again, and only fail if it's still not possible
				return nullptr;
			}
			header = reinterpret_cast<allocation_header*>(base + base_offset);
			if(!header->in_use.test_and_set()) {
				break;
			}
		}

		header->allocated_size.store(allocated_size, std::memory_order_release);
		header->requested_size = requested_size;
		header->base_offset = base_offset;
		header->base_page = base_offset / page_size;

		size_t base_page = header->base_page;
		size_t base_page_usage = base_offset % page_size;
		size_t base_page_available = page_size - base_page_usage;
		size_t remaining = allocated_size;
		if(base_page_available < remaining) {
			(*page_usage)[base_page++].fetch_add(base_page_available);
			remaining -= base_page_available;
			for(; remaining > page_size; remaining -= page_size) {
				(*page_usage)[base_page++].store(page_size);
			}
		}
		header->last_offset = remaining;
		header->last_page = base_page;
		(*page_usage)[header->last_page].fetch_add(header->last_offset);
		return base + base_offset + sizeof(allocation_header);
	}

	void arena::deallocate(void* region_) {
		if(region_ == nullptr) {
			return;
		}

		unsigned char* region = static_cast<unsigned char*>(region_);
		if(region -    base >= size
		&& region - rw_base >= size) {
			throw std::runtime_error("attempt to deallocate memory belonging to another arena");
		}

		static const size_t padded_header_size     = (sizeof(allocation_header) + (alignment - 1)) & ~(alignment - 1);

		region -= padded_header_size;
		if(region <    base
		&& region < rw_base) {
			throw std::runtime_error("attempt to deallocate memory belonging to another arena");
		}

		allocation_header* header = reinterpret_cast<allocation_header*>(region);
		size_t amount = header->allocated_size.load();
		size_t last_offset = header->last_offset;
		size_t last_page = header->last_page;

		if(last_offset <= amount) {
			(*page_usage)[last_page--].fetch_sub(last_offset);
			amount -= last_offset;
			for(; amount > page_size; amount -= page_size) {
				(*page_usage)[last_page--].store(0);
			}
		}
		(*page_usage)[last_page].fetch_sub(amount);
		header->requested_size = 0;
		header->in_use.clear();
	}

	bool arena::is_protected(const gc_bits location) const {
		// TODO: the arena should track which pages or ojects are being moved explicitly
		// so that this can be made faster
		return TRUE == ::IsBadWritePtr(_gc_resolve(location), 1);
	}
}
