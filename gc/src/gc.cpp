#include "stdafx.h"

#include "gc.hpp"

namespace garbage_collection
{
	const gc_bits gc_bits::zero = { 0 };

	collector the_gc;

	thread_local thread_data this_thread_data;

	gc_bits gc_bits::_gc_build_reference(object_base* base_pointer, void* typed_pointer) {
		gc_bits unresolved = the_gc.the_arena.pointer_to_bits(base_pointer);
		unresolved.address.generation = gc_bits::young;
		unresolved.address.seen_by_marker = the_gc.current_marker_phase.load(std::memory_order_acquire);
		ptrdiff_t typed_offset = (reinterpret_cast<byte*>(base_pointer) - static_cast<byte*>(typed_pointer)) / static_cast<ptrdiff_t>(sizeof(void*));
		unresolved.address.typed_offset = typed_offset;
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
			rw_base        = static_cast<byte*>(::MapViewOfFileEx(section, FILE_MAP_WRITE, 0, 0, section_size.QuadPart, putative_base));
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
		::UnmapViewOfFile(rw_base_shadow);
		::UnmapViewOfFile(rw_base);
		::UnmapViewOfFile(base_shadow);
		::UnmapViewOfFile(base);
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
			const size_t h = high_watermark.load(std::memory_order_acquire);
			const size_t l = low_watermark.load(std::memory_order_acquire);
			const size_t exactly_available = size - (h - l);

			if(allocated_size > exactly_available) {
				// TODO force collection and compaction, try again, and only fail if it's still not possible
				// ... but I can't collect from here, because the thread lock is held.
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
		return TRUE == ::IsBadWritePtr(bits_to_pointer(location), 1);
	}

	void collector::collect() {
		/*for(;;) */{
			size_t low  = the_arena.low_watermark.load(std::memory_order_acquire);
			size_t high = the_arena.high_watermark.load(std::memory_order_acquire);

			flip();
			mark();
			std::cout << "allocateds" << std::endl;
			allocateds.visualize();
			std::cout << "usage" << std::endl;
			usage.visualize();
			std::cout << "reachables" << std::endl;
			reachables.visualize();
			// wait for all mutators to hit safepoint
			finalize(low, high);
			shrink(low, high);
			relocate(low, high);
			remap();
		}
	}

	void collector::queue_object(object_base* obj) {
		std::scoped_lock<std::mutex> l(mutated_lock);
		mutated_objects.insert(obj);
	}

	void collector::mark() {
		reachables.reset();
		usage.reset();

		root_set roots = scan_roots();

		marker m(this);
		for(registration r : roots) {
			if(raw_reference* ref = std::get<raw_reference*>(r)) {
				m.trace(ref);
			} else {
				m.trace(std::get<object_base*>(r));
			}
		}
		for(;;) {
			mark_queue local_copy;
			{
				std::scoped_lock<std::mutex> l(mutated_lock);
				local_copy.swap(mutated_objects);
			}
			if(local_copy.empty()) {
				break;
			}
			for(object_base* obj : local_copy) {
				m.trace(obj);
			}
		}
	}
	
	void collector::finalize(size_t bottom, size_t top) {
		bottom /= arena::maximum_alignment;
		top    /= arena::maximum_alignment;
		size_t offset = bottom * arena::maximum_alignment;
		bottom /= bit_table::element_bits;
		top    /= bit_table::element_bits;
		for(size_t unit_number = bottom; unit_number != top; ++unit_number) {
			if(allocateds.bits[unit_number] == 0) {
				offset += arena::maximum_alignment * bit_table::element_bits;
				continue;
			}
			bit_table::element mask = 1;
			for(size_t i = 0; i < bit_table::element_bits; ++i, mask <<= 1, offset += arena::maximum_alignment) {
				if((allocateds.bits[unit_number] & mask) == mask
				&& (reachables.bits[unit_number] & mask) != mask) {
					byte* const block = static_cast<byte*>(the_arena.offset_to_pointer(offset));
					const arena::allocation_header* const header = the_arena.get_header(block);
					const object_base* const obj = reinterpret_cast<const object_base*>(block + header->object_offset);
					// TODO: put this on a separate thread, so that GC threads are never at risk of owning GC objects
					object_base::guarded_destruct(obj);
				}
			}
		}
	}

	void collector::shrink(size_t bottom, size_t top) {
		bottom /= arena::maximum_alignment;
		top    /= arena::maximum_alignment;
		size_t offset = bottom * arena::maximum_alignment;
		bottom /= bit_table::element_bits;
		top    /= bit_table::element_bits;
		for(size_t unit_number = bottom; unit_number != top; ++unit_number) {
			if(allocateds.bits[unit_number] == 0) {
				offset += arena::maximum_alignment * bit_table::element_bits;
				continue;
			}
			break;
		}
		const size_t upper_limit = top * bit_table::element_bits * arena::maximum_alignment;
		if(offset <= upper_limit) {
			the_arena.low_watermark.store(offset, std::memory_order_release);
		}
	}

	void collector::relocate(size_t bottom, [[maybe_unused]] size_t top) {
		[[maybe_unused]] const size_t page = bottom / arena::page_size;
		constexpr float threshold = 0.75f;
		[[maybe_unused]] constexpr uint16_t byte_threshold = static_cast<uint16_t>(threshold * static_cast<float>(arena::page_size));

	}

	collector::root_set collector::scan_roots() {
		root_set roots = globals.get_snapshot();
		
		std::scoped_lock<std::mutex> read_lock(thread_lock);
		for(auto it = mutator_threads.begin(), end = mutator_threads.end(); it != end; ++it) {
			root_set thread_roots = it->second->get_snapshot();
			roots.insert(roots.end(), thread_roots.begin(), thread_roots.end());
		}
		return roots;
	}

	void collector::flip() {
		page_allocations = std::make_unique<std::atomic<uint16_t>[]>(the_arena.page_count);
		std::memset(page_allocations.get(), 0, sizeof(std::atomic<uint16_t>) * the_arena.page_count);
		current_marker_phase.fetch_xor(1ui64, std::memory_order_release);
		// TODO wait for all mutators to acknowledge flip
	}

	void collector::update_usage(const void* const addr, size_t object_size) {
		size_t offset = the_arena.pointer_to_offset(addr);
		size_t page_space_remaining = arena::page_size - (offset % arena::page_size);
		size_t page = offset / arena::page_size;
		if(object_size > page_space_remaining) {
			page_allocations[page++].fetch_add(static_cast<uint16_t>(page_space_remaining));
			object_size -= page_space_remaining;
			while(object_size >= arena::page_size) {
				page_allocations[page++].store(arena::page_size);
				object_size -= arena::page_size;
			}
		}
		page_allocations[page].fetch_add(static_cast<uint16_t>(object_size));
	}

	void marker::trace(const raw_reference* ref) {
		if(ref->pointer.load(std::memory_order_relaxed).address.seen_by_marker != the_gc->current_marker_phase) {
			for(;;) {
				gc_bits old_value = ref->pointer.load(std::memory_order_acquire);
				gc_bits new_value = old_value;
				new_value.address.seen_by_marker = the_gc->current_marker_phase;
				if(ref->pointer.compare_exchange_strong(old_value, new_value, std::memory_order_release)) {
					break;
				}
			}
		}
		object_base* obj = ref->_gc_resolve_base();
		if(!obj) {
			return;
		}
		trace(obj);
	}

	void marker::trace(const object_base* obj) {
		void* base_address = obj->_gc_get_block();
		if(!the_gc->is_marked_allocated(base_address)) {
			// this can occur when the stack unwind destroys the last reference during a GC sweep
			// TODO handle the race: the stack unwind can destruct an object mid-trace. Is this safe?
			return;
		}
		if(!the_gc->is_marked_reachable(base_address)) {
			the_gc->mark_reachable(base_address);
			obj->_gc_trace(this);
		}
	}

	void nuller::trace(const raw_reference* ref) {
		[[gsl::suppress(type.3)]]
		*const_cast<raw_reference*>(ref) = nullptr;
	}

}
