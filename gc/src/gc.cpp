#include "stdafx.h"

#include "gc.hpp"

namespace gc
{
	const gc_bits gc_bits::zero = { 0 };

	global_data globals;

	thread_local thread_data this_gc_thread;

	collector the_gc;

	gc_bits gc_bits::_gc_build_reference(object_base* memory, ptrdiff_t typed_offset) {
		gc_bits unresolved = the_gc.the_arena._gc_unresolve(memory);
		unresolved.address.generation = gc_bits::young;
		unresolved.address.seen_by_marker = the_gc.current_marker_phase.load(std::memory_order_acquire);
		unresolved.address.typed_offset = typed_offset;
		unresolved.header.colour = gc_bits::grey;
		return unresolved;
	}

	arena::arena(size_t size_) : size(size_), page_count(size / page_size) {
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
		low_watermark .store(maximum_alignment - sizeof(allocation_header), std::memory_order_release);
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
			if(high_watermark.compare_exchange_strong(old_offset, new_offset, std::memory_order_release)) {
				base_offset = old_offset;
				break;
			}
		}
		allocation_header* header = reinterpret_cast<allocation_header*>(base + base_offset);
		header->allocated_size.store(allocated_size, std::memory_order_release);
		void* allocation = base + base_offset + sizeof(allocation_header);
		std::memset(allocation, 0, allocated_size - sizeof(allocation_header));
		return allocation;
	}

	void arena::deallocate(void* region_) {
		if(region_ == nullptr) {
			return;
		}

		byte* region = static_cast<byte*>(region_);
		if(region -    base >= static_cast<ptrdiff_t>(size)
		&& region - rw_base >= static_cast<ptrdiff_t>(size)) {
			throw std::runtime_error("attempt to deallocate memory belonging to another arena");
		}

		allocation_header* header = reinterpret_cast<allocation_header*>(region - sizeof(allocation_header));
		size_t block_size = header->allocated_size.load(std::memory_order_acquire);
#ifdef _DEBUG
		std::memset(region, 0xff, block_size - sizeof(allocation_header));
#endif
		header->allocated_size.store(0ui64, std::memory_order_release);

		size_t block_offset = static_cast<size_t>(region - base);
		// if this just happens to be the lowest block, bump the low watermark optimistically
		low_watermark.compare_exchange_strong(block_offset, block_offset + block_size, std::memory_order_release);
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
		current_marker_phase.fetch_xor(1ui64, std::memory_order_release);
		condemned_colour.fetch_xor(1ui64, std::memory_order_release);
		scanned_colour.fetch_xor(1ui64, std::memory_order_release);
	}

	void marker::trace(const raw_reference& ref) {
		// mark the reference as marked through
		for(;;) {
			gc_bits old_value = ref.pointer.load(std::memory_order_acquire);
			gc_bits new_value = old_value;
			new_value.address.seen_by_marker = the_gc.current_marker_phase;
			if(ref.pointer.compare_exchange_strong(old_value, new_value, std::memory_order_release)) {
				break;
			}
		}

		object_base* obj = ref._gc_resolve_base();
		if(!obj) {
			return;
		}
		if(obj->count.load(std::memory_order_relaxed).header.destructed) {
			return;
		}

		// grey the object
		for(;;) {
			gc_bits old_value = obj->count.load(std::memory_order_acquire);
			gc_bits new_value = old_value;
			new_value.header.colour = gc_bits::grey;
			if(obj->count.compare_exchange_strong(old_value, new_value, std::memory_order_release)) {
				break;
			}
		}
		// trace the object
		obj->_gc_trace(this);
		// black the object
		for(;;) {
			gc_bits old_value = obj->count.load(std::memory_order_acquire);
			gc_bits new_value = old_value;
			new_value.header.colour = the_gc.scanned_colour;
			if(obj->count.compare_exchange_strong(old_value, new_value, std::memory_order_release)) {
				break;
			}
		}
	}

	void nuller::trace(const raw_reference& ref) {
		const_cast<raw_reference&>(ref).clear();
	}

}
