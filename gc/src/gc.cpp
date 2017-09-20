#include "stdafx.h"

#include "gc.hpp"

namespace gc
{
	global_data globals;

	thread_local thread_data this_gc_thread;

	collector the_gc;

	gc_bits gc_bits::_gc_build_reference(void* memory) {
		gc_bits unresolved = the_gc.the_arena._gc_unresolve(memory);
		unresolved.bits.address.generation = gc_bits::young;
		unresolved.bits.address.nmt = the_gc.current_nmt.load(std::memory_order_acquire);
		unresolved.bits.header.colour = gc_bits::grey;
		return unresolved;
	}

	arena::arena() {
		ULARGE_INTEGER section_size = { 0 };
		section_size.QuadPart = size;
		section = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, section_size.HighPart, section_size.LowPart, nullptr);
		if(section == NULL) {
			throw std::bad_alloc();
		}
		do {
			void* putative_base = ::VirtualAlloc(nullptr, size * 2, MEM_RESERVE, PAGE_READWRITE);
			::VirtualFree(putative_base, 0, MEM_RELEASE);
			base        = static_cast<byte*>(::MapViewOfFileEx(section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart, putative_base));
			base_shadow = static_cast<byte*>(::MapViewOfFileEx(section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart, base + size  ));
			if(base_shadow == nullptr) {
				::UnmapViewOfFile(base);
			}
		}
		while(base == nullptr || base_shadow == nullptr);

		do {
			void* putative_base = ::VirtualAlloc(nullptr, size * 2, MEM_RESERVE, PAGE_READWRITE);
			::VirtualFree(putative_base, 0, MEM_RELEASE);
			rw_base        = static_cast<byte*>(::MapViewOfFile  (section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart));
			rw_base_shadow = static_cast<byte*>(::MapViewOfFileEx(section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart, rw_base + size));
			if(rw_base_shadow == nullptr) {
				::UnmapViewOfFile(rw_base);
			}
		}
		while(rw_base == nullptr || rw_base_shadow == nullptr);

		high_watermark.store(maximum_alignment - sizeof(allocation_header), std::memory_order_release);
		low_watermark.store(0ui64, std::memory_order_release);
	}

	arena::~arena() {
		::UnmapViewOfFile(rw_base);
		::UnmapViewOfFile(rw_base_shadow);
		::UnmapViewOfFile(base);
		::UnmapViewOfFile(base_shadow);
		::CloseHandle(section);
	}

	void* arena::allocate(size_t amount) {
		// base_offset + sizeof(allocation_header) is already maximally aligned.
		const size_t requested_size = sizeof(allocation_header) + amount;
		      size_t padding_size = ((requested_size + (maximum_alignment - 1)) & ~(maximum_alignment - 1)) - requested_size;

		if(padding_size < sizeof(allocation_header)) {
			padding_size += maximum_alignment;
		}
		const size_t allocated_size = requested_size + padding_size;

		size_t base_offset = 0;
		allocation_header* header = nullptr;
		for(;;) {
			size_t h = high_watermark.load(std::memory_order_acquire);
			size_t l = low_watermark.load(std::memory_order_acquire);
			size_t exactly_available = size - (h - l);

			if(allocated_size > exactly_available) {
				// TODO force collection and compaction, try again, and only fail if it's still not possible
				return nullptr;
			}

			size_t old_offset = h;
			size_t new_offset = old_offset + allocated_size;
			if(!high_watermark.compare_exchange_strong(old_offset, new_offset, std::memory_order_release)) {
				continue;
			}

			base_offset = new_offset;
			header = reinterpret_cast<allocation_header*>(base + base_offset);
			size_t old_header_size = 0ui64;
			if(header->allocated_size.compare_exchange_strong(old_header_size, allocated_size, std::memory_order_release)) {
				break;
			}
		}
		return base + base_offset + sizeof(allocation_header);
	}

	void arena::deallocate(void* region_) {
		if(region_ == nullptr) {
			return;
		}

		byte* region = static_cast<byte*>(region_);
		if(region -    base >= size
		&& region - rw_base >= size) {
			throw std::runtime_error("attempt to deallocate memory belonging to another arena");
		}

		static const size_t padded_header_size     = (sizeof(allocation_header) + (maximum_alignment - 1)) & ~(maximum_alignment - 1);

		region -= padded_header_size;
		if(region <    base
		&& region < rw_base) {
			throw std::runtime_error("attempt to deallocate memory belonging to another arena");
		}

		allocation_header* header = reinterpret_cast<allocation_header*>(region);
		header->allocated_size.store(0ui64, std::memory_order_release);
	}

	bool arena::is_protected(const gc_bits location) const {
		// TODO: the arena should track which pages or ojects are being moved explicitly
		// so that this can be made faster
		return TRUE == ::IsBadWritePtr(_gc_resolve(location), 1);
	}

	void collector::collect() {
		for(;;) {
			flip();
			mark();
			finalize();
			relocate();
			remap();
		}
	}

	void collector::queue_object(object_base*) {

	}

	void collector::mark() {
		scan_roots();
	}
	
	void collector::scan_roots() {
		root_set roots{};
		for(gc::object_base* root : roots) {
			queue_object(root);
		}
	}

	void collector::flip() {
		current_nmt.fetch_xor(1ui64, std::memory_order_release);
		condemned_colour.fetch_xor(1ui64, std::memory_order_release);
		scanned_colour.fetch_xor(1ui64, std::memory_order_release);
	}
}
