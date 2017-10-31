#pragma once

//#pragma warning(disable: 4571) // warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
//#pragma warning(disable: 4987) // warning C4987: nonstandard extension used: 'throw (...)'

#if !defined(_STL_EXTRA_DISABLED_WARNINGS)
#define _STL_EXTRA_DISABLED_WARNINGS 4061 4623 4365 4571 4625 4626 4710 4820 4987 5026 5027
#endif

#include <cstddef>
#include <new>
#include <cassert>
#include <atomic>
#include <array>
#include <vector>
#include <thread>
#include <unordered_map>
#include <type_traits>
#include <initializer_list>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <tuple>
#include <map>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include "lock.hpp"

#pragma warning(disable: 4324) // warning C4234: structure was padded due to alignment specifier
#pragma warning(disable: 4625) // warning C4625: '%s': copy constructor was implicitly defined as deleted
#pragma warning(disable: 4626) // warning C4626: '%s': assignment operator was implicitly defined as deleted
#pragma warning(disable: 4710) // warning C4710: '%s': function not inlined
#pragma warning(disable: 4820) // warning C4820: '%s': '%d' bytes padding added after data member '%s'
#pragma warning(disable: 5026) // warning C5026: '%s': move constructor was implicitly defined as deleted
#pragma warning(disable: 5027) // warning C5027: '%s': move assignment operator was implicitly defined as deleted

#pragma warning(push)

#pragma warning(disable: 4365) // warning C4365: '%s': conversion from '%s' to '%s', signed/unsigned mismatch
#pragma warning(disable: 4464) // warning C4464: relative include path contains '..'
#pragma warning(disable: 4574) // warning C4574: '%s' is defined to be '%s': did you mean to use '#if %s'?

//
//#include <tbb/concurrent_queue.h>
//#include <tbb/enumerable_thread_specific.h>
//
//#include <ppl.h>

#pragma warning(pop)

#pragma pointers_to_members(full_generality, virtual_inheritance)  

namespace garbage_collection
{
	struct object_base;
	struct collector;
	struct arena;

	union gc_bits
	{
		size_t value;
		// used by raw_reference to point to an object_base on the gc heap
		// used by reference_t<T>  to include the offset to the T object on the gc heap
		// used by object_base to record either the start of the containing memory block, or the relocated object
		struct
		{
			size_t    page_offset    : 12; // 4K pages => 12 bit page offset
			size_t    page_number    : 28;
			size_t    generation     : 2;
			size_t    seen_by_marker : 2;
			ptrdiff_t typed_offset   : 12; // where to find the start of the typed object, relative to the address of the object_base, in void*s
		} address;
		// used by object_base to track ref counts, mark status, whether the destructor has been called
		struct
		{
			size_t raw_address       : 56;
			size_t colour            : 2;
			size_t relocate          : 2;  // 0 = resident, 1 = moving, 2 = moved
			size_t reference_count   : 3;  // 0-6 = rced, 7 = gced
			size_t destructed        : 1;
		} header;
		struct
		{
			size_t raw_address : 56;
			size_t raw_header  : 8;
		} layout;
		struct
		{
			size_t pointer : 40;
		} raw;

		enum
		{
			resident = 0,
			moving = 1,
			moved = 2,
		};
		enum
		{
			fixed = 0,
			young = 1,
			old = 2,
			max_arenas = 3,
		};
		enum
		{
			white_or_black = 0, // black: reachable and scanned
			black_or_white = 1, // white: unreachable and scanned
			grey           = 2, // grey:  reachable and unscanned
			free           = 3  // free:  already reclaimed (e.g. by ref-count)
		};

		static gc_bits _gc_build_reference(collector* the_gc, arena* the_arena, object_base* memory, ptrdiff_t typed_offset);

		static const gc_bits zero;
	};

	static_assert(sizeof(gc_bits) == sizeof(std::size_t), "gc_bits is the wrong size");

	using gc_pointer = std::atomic<gc_bits>;

	static_assert(gc_pointer::is_always_lock_free, "gc_pointer isn't lock-free");

	static constexpr size_t get_max_ref_count() noexcept {
		return 7;
	}

	template <class T>
	static inline constexpr
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool>
	is_power_of_two(T v) noexcept {
		return (v != 0) && (v & (v - 1)) == 0;
	}

	template <class T>
	static inline constexpr
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
	log2(T n, T p) noexcept {
		return n <= 1 ? p : log2(n / 2, p + 1);
	}

	template <class T>
	static inline constexpr
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
	log2(T n) noexcept {
		return log2(n, T(0));
	}

	struct collector;

	struct object_base;

	struct raw_reference;

	template<typename T>
	struct reference_t;

	template<typename T>
	struct handle_t;

	template<typename T>
	struct array;

	template<typename T>
	struct box;

	template<typename T, bool InArray>
	struct normalize {
		using type = std::conditional_t<std::is_base_of_v<object_base, T>,
		                                T,
		                                std::conditional_t<InArray, T, box<T>>
		>;
	};

	template<typename T, bool InArray>
	struct normalize<array<T>, InArray> {
		using type = array<typename normalize<T, true>::type>;
	};

	template<typename T, bool InArray>
	struct normalize<T[], InArray> {
		using type = array<typename normalize<T, true>::type>;
	};

	template<typename T, bool InArray, size_t N>
	struct normalize<T[N], InArray> {
		using type = array<typename normalize<T, true>::type>;
	};

	template<typename T>
	using normalize_t = typename normalize<T, false>::type;

	template<typename T>
	struct is_array : std::false_type {
	};

	template<typename T>
	struct is_array<array<T>> : std::true_type {
	};

	template<typename T>
	constexpr bool is_array_v = is_array<T>::value;

	struct visitor
	{
		virtual void trace(const raw_reference* ref) = 0;
		virtual void trace(const object_base*) {
		}

		void trace(const raw_reference& ref) {
			trace(&ref);
		}
	};

	struct arena
	{
		static constexpr size_t page_size = 1 << 12;
		size_t size;
		size_t page_count;

		void* section;
		byte* base;
		byte* base_shadow;
		byte* rw_base;
		byte* rw_base_shadow;
		std::atomic<size_t> low_watermark;
		std::atomic<size_t> high_watermark;

		arena(size_t size_ = 1ui64 << 30);
		~arena();
		arena(const arena&) = delete;
		arena(arena&&) = delete;
		arena& operator=(const arena&) = delete;
		arena& operator=(arena&&) = delete;

		static constexpr size_t maximum_alignment = 512 / CHAR_BIT; // AVX512 alignment is the worst we're ever likely to see
		static_assert(is_power_of_two(maximum_alignment), "maximum_alignment isn't a power of two");
		static constexpr size_t maximum_alignment_shift = 0; // log2(maximum_alignment);

		struct allocation_header {
			std::atomic<size_t> allocated_size;
			ptrdiff_t object_offset;
		};

		void* allocate(size_t amount);

		void deallocate(void* region);
		bool is_protected(const gc_bits location) const;

		size_t approximately_available() const noexcept {
			return size - (high_watermark.load(std::memory_order_relaxed) - low_watermark.load(std::memory_order_relaxed));
		}

		gc_bits get_new_location(const gc_bits old_location) const noexcept {
			// TODO
			return old_location;
		}

		allocation_header* get_header(void* block) const noexcept {
			byte* region = static_cast<byte*>(block);
			allocation_header* header = reinterpret_cast<allocation_header*>(region - sizeof(allocation_header));
			return header;
		}

		size_t get_block_size(void* block) const noexcept {
			allocation_header* header = get_header(block);
			return header->allocated_size.load(std::memory_order_acquire);
		}

		void* _gc_resolve(const gc_bits location) const noexcept {
			if(location.raw.pointer == 0) {
				return nullptr;
			}
			const size_t offset = (location.address.page_number * page_size) + (location.address.page_offset << maximum_alignment_shift);
			return base + offset;
		}
	
		gc_bits _gc_unresolve(const void* location) const noexcept {
			if(location == nullptr) {
				return gc_bits{ 0 };
			}
			const size_t offset = static_cast<size_t>(static_cast<const byte*>(location) - base);
			
			//assert(offset % maximum_alignment == 0);

			gc_bits value = { 0 };
			value.address.page_number = offset / page_size;
			value.address.page_offset = (offset % page_size) >> maximum_alignment_shift;
			return value;
		}
	};

	struct bit_table
	{
		bit_table(arena* the_arena_, size_t size_) : the_arena(the_arena_), size((size_ / arena::maximum_alignment) / CHAR_BIT), bits(new std::atomic<uint8_t>[size]) {

		}

		uint8_t set_bit(void* address) {
			std::pair<size_t, uint8_t> position = get_bit_position(address);
			uint8_t mask = static_cast<uint8_t>(1ui8 << position.second);
			uint8_t orig = bits[position.first].fetch_or(mask, std::memory_order_release);
			return static_cast<uint8_t>((orig & mask) >> position.second);
		}

		uint8_t clear_bit(void* address) {
			std::pair<size_t, uint8_t> position = get_bit_position(address);
			uint8_t mask = static_cast<uint8_t>(1ui8 << position.second);
			uint8_t orig = bits[position.first].fetch_and(static_cast<uint8_t>(~mask), std::memory_order_release);
			return static_cast<uint8_t>((orig & mask) >> position.second);
		}

		bool query_bit(void* address) {
			std::pair<size_t, uint8_t> position = get_bit_position(address);
			return 1ui8 == ((bits[position.first].load(std::memory_order_relaxed) >> position.second) & 1ui8);
		}

		void reset() {
			for(size_t i = 0; i < size; ++i) {
				bits[i].store(0ui8, std::memory_order_relaxed);
			}
		}

		std::pair<size_t, uint8_t> get_bit_position(void* address);

		arena* the_arena;
		size_t size;
		std::unique_ptr<std::atomic<uint8_t /*std::byte*/>[]> bits;
	};

	struct collector
	{
		collector(size_t size_ = 1ui64 << 30) : the_arena(size_), reachables(&the_arena, size_), allocateds(&the_arena, size_) {
		}

		template<typename T, typename... Args>
		std::enable_if_t<!is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(Args&&... args);
		template<typename T>
		std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(size_t size);
		template<typename T>
		std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(std::initializer_list<typename normalize_t<T>::element_type> init);
		template<typename To, typename From>
		handle_t<normalize_t<To>> gc_cast(const reference_t<From>& rhs);

	//protected:
		friend struct raw_reference;
		friend union gc_bits;

		void queue_object(object_base*);

		void collect();

		void mark();

		void relocate() {

		}

		void remap() {

		}

		void finalize() {

		}

		arena the_arena;

		std::atomic<size_t> current_marker_phase;
		std::atomic<size_t> condemned_colour;
		std::atomic<size_t> scanned_colour;

	protected:
		bool is_marked_reachable(void* address) {
			return reachables.query_bit(address);
		}

		void mark_reachable(void* address) {
			reachables.set_bit(address);
		}

		void mark_allocated(void* address) {
			allocateds.set_bit(address);
		}

	private:
		friend struct object_base;
		friend struct marker;

		using root_set = std::vector<object_base*>;

		void flip();

		void scan_roots();

		using mark_queue = void;

		bit_table reachables;
		bit_table allocateds;
	};

	inline std::pair<size_t, uint8_t> bit_table::get_bit_position(void* addr) {
		assert(addr != nullptr);
		const gc_bits offset = the_arena->_gc_unresolve(addr);
		const size_t block_number = offset.raw.pointer / (arena::maximum_alignment / CHAR_BIT);
		const size_t byte_number = block_number / CHAR_BIT;
		const uint8_t bit_number = block_number % CHAR_BIT;
		return { byte_number, bit_number };
	}

	struct marker : visitor {
		marker(collector* the_gc_, arena* the_arena_) : the_gc(the_gc_), the_arena(the_arena_) {
		}

		virtual void trace(const raw_reference* ref) override;
		virtual void trace(const object_base* obj) override;
		using visitor::trace;

	private:
		collector* the_gc;
		arena* the_arena;
	};

	struct nuller : visitor {
		virtual void trace(const raw_reference* ref) override;
		using visitor::trace;
	};

	struct object_base
	{
		object_base() noexcept {
			gc_bits bits = count.load(std::memory_order_acquire);
			bits.header.reference_count = 1;
			count.store(bits, std::memory_order_release);
		}

		object_base(const object_base&) = delete;
		object_base(object_base&&) = delete;
		object_base& operator=(const object_base&) = delete;
		object_base& operator=(object_base&&) = delete;

		virtual ~object_base() noexcept {
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) { return ptr; }
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) { return ptr; }

		virtual void _gc_trace(visitor*) const = 0;

		static void guarded_destruct(const object_base* obj) {
			for(;;) {
				// some casual UB here: we read from a potentially-already-destructed object.
				// we know that the destructor leaves the object in a valid/meaningful state.
				// we could in principle use a 'finalize' method and never call the real d-tor,
				// but that's tedious to plumb into every subobject properly. We want the 
				// automatic destruction of child objects etc, which only the real destructor
				// gives us. This is important because as often as possible, we want members
				// to use ref-count rather than full GC.
				gc_bits original = obj->count.load(std::memory_order_acquire);
				if(original.header.destructed) {
					return;
				}
				gc_bits updated = original;
				updated.header.destructed = 1;
				if(obj->count.compare_exchange_strong(original, updated, std::memory_order_release)) {
					nuller n;
					obj->_gc_trace(&n);
					obj->~object_base();
					return;
				}
			}
		}

	protected:
		void _gc_set_block_unsafe(collector* the_gc, arena* the_arena, void* addr, object_base* obj) noexcept {
			gc_bits location = the_arena->_gc_unresolve(addr);
			gc_bits current = count.load(std::memory_order_acquire);
			current.header.raw_address = location.header.raw_address;
			count.store(current, std::memory_order_release);

			the_gc->mark_allocated(addr);
			the_arena->get_header(addr)->object_offset = reinterpret_cast<byte*>(obj) - reinterpret_cast<byte*>(addr);
		}

		void* _gc_get_block(arena* the_arena) const noexcept {
			gc_bits location = count.load(std::memory_order_acquire);
			return the_arena->_gc_resolve(location);
		}

	private:
		mutable gc_pointer count;

		friend struct collector;
		friend struct marker;
		friend struct raw_reference;

		size_t _gc_add_ref() const noexcept {
			for(;;) {
				gc_bits current = count.load(std::memory_order_acquire);
				if(current.header.reference_count == get_max_ref_count()) {
					return get_max_ref_count();
				}
				gc_bits new_value = current;
				++new_value.header.reference_count;
				if(count.compare_exchange_strong(current, new_value)) {
					return new_value.header.reference_count;
				}
			}
		}

		size_t _gc_del_ref() const noexcept {
			for(;;) {
				gc_bits current = count.load(std::memory_order_acquire);
				if(current.header.reference_count == get_max_ref_count()) {
					return get_max_ref_count();
				}
				gc_bits new_value = current;
				--new_value.header.reference_count;
				if(count.compare_exchange_strong(current, new_value)) {

					if(current.header.reference_count == 1) {
						//void* block = this->_gc_get_block();
						object_base::guarded_destruct(this);
						//the_gc.the_arena.deallocate(block);
					}
					return new_value.header.reference_count;
				}
			}
		}
	};

	struct object : virtual object_base
	{
		virtual void _gc_trace(visitor*) const override {
		}
	};

	template<typename T>
	struct
	alignas(std::conditional_t<std::is_base_of_v<object, T>, reference_t<T>, T>)
	alignas(object)
	array final : object
	{
		friend struct collector;

		using element_type = std::conditional_t<std::is_base_of_v<object, T>, reference_t<T>, T>;

		array() = delete;
		array(const array&) = delete;

		explicit array(size_t length_) noexcept(std::is_nothrow_constructible_v<element_type>) : len(length_) {
			element_type* elts = elements();
			for(size_t i = 0; i < len; ++i) {
				new(&elts[i]) element_type();
			}
		}
	
		explicit array(std::initializer_list<element_type> init) noexcept(std::is_nothrow_copy_constructible_v<element_type>) : len(init.size()) {
			size_t offset = 0;
			for(const element_type& e : init) {
				new(&(elements()[offset++])) element_type(e);
			}
		}

		element_type& operator[](size_t offset) {
			if(offset >= length()) {
				throw std::out_of_range("out of bounds");
			}
			return elements()[offset];
		}

		const element_type& operator[](size_t offset) const {
			if(offset >= length()) {
				throw std::out_of_range("out of bounds");
			}
			return elements()[offset];
		}

		size_t length() const noexcept {
			return len;
		}

		virtual void _gc_trace(visitor* v) const override {
			if constexpr(std::is_base_of_v<object, T>) {
				const element_type* es = elements();
				for(size_t i = 0; i < len; ++i) {
					if(es[i]) {
						v->trace(es[i]);
					}
				}
			}
			return object::_gc_trace(v);
		}

	private:
		static size_t allocation_size(size_t len) {
			return sizeof(array<T>) + (len * sizeof(element_type));
		}

		element_type* elements() noexcept {
			return reinterpret_cast<element_type*>(this + 1);
		}

		const element_type* elements() const noexcept {
			return reinterpret_cast<const element_type*>(this + 1);
		}

		size_t len;
		// element_type elements[];
	};

	template<typename T>
	struct box final : object {
		using element_type = std::conditional_t<std::is_base_of_v<object, T>, reference_t<T>, T>;

		box(const element_type& e) noexcept(std::is_nothrow_copy_constructible_v<element_type>) : val(e) {
		}

		virtual void _gc_trace(visitor* v) const override {
			if constexpr(std::is_base_of_v<object, T>) {
				v->trace(val);
			}
			return object::_gc_trace(v);
		}

		operator element_type&() noexcept {
			return val;
		}

		operator const element_type&() const noexcept {
			return val;
		}

	private:
		element_type val;
	};

	struct raw_reference
	{
		explicit operator bool() const noexcept {
			return pointer.load().raw.pointer != gc_bits::zero.raw.pointer;
		}

		bool operator==(const raw_reference& rhs) const noexcept {
			return (the_gc == rhs.the_gc && pointer.load().raw.pointer == rhs.pointer.load().raw.pointer) ||
			       _gc_resolve_base() == rhs._gc_resolve_base();
		}

		bool operator!=(const raw_reference& rhs) const noexcept {
			return !(*this == rhs);
		}

	protected:
		raw_reference() noexcept : the_gc(nullptr), the_arena(nullptr), pointer() {
		}

		raw_reference(std::nullptr_t) noexcept : the_gc(nullptr), the_arena(nullptr), pointer() {
		}

		raw_reference(const raw_reference& rhs) noexcept {
			assign(rhs);
		}

		raw_reference& operator=(const raw_reference& rhs) noexcept {
			if(this != &rhs) {
				clear();
				assign(rhs);
			}
			return *this;
		}

		raw_reference& operator=(std::nullptr_t) noexcept {
			clear();
			return *this;
		}

		virtual void assign(const raw_reference& rhs) noexcept {
			if(object_base* obj = rhs._gc_resolve_base()) {
				obj->_gc_add_ref();
			}
			_gc_write_barrier();
			the_gc = rhs.the_gc;
			the_arena = rhs.the_arena;
			pointer.store(rhs.pointer.load(std::memory_order_acquire));
		}

		virtual void clear() noexcept {
			if(object_base* obj = _gc_resolve_base()) {
				obj->_gc_del_ref();
			}
			pointer.store(gc_bits::zero, std::memory_order_release);
			the_arena = nullptr;
			the_gc = nullptr;
		}

		virtual ~raw_reference() noexcept {
			clear();
		}

		void* _gc_resolve() const noexcept {
			if(!pointer.load(std::memory_order_acquire).raw.pointer) {
				return nullptr;
			}
			gc_bits value = _gc_read_barrier();
			object_base* ob = reinterpret_cast<object_base*>(the_arena->_gc_resolve(value));
			byte* derived_object = reinterpret_cast<byte*>(ob) + (value.address.typed_offset * static_cast<ptrdiff_t>(sizeof(void*)));
			return derived_object;
		}

		object_base* _gc_resolve_base() const noexcept {
			if(!pointer.load(std::memory_order_acquire).raw.pointer) {
				return nullptr;
			}
			gc_bits value = _gc_read_barrier();
			object_base* ob = reinterpret_cast<object_base*>(the_arena->_gc_resolve(value));
			return ob;
		}

		void _gc_write_barrier() const noexcept {
			// awkward. if my retaining object is oldgen, and the new object is newgen, I need to treat that specially.
			// but I don't know what my retaining object is.
			// maybe I can judge based on the location of 'this'
		}

		gc_bits _gc_read_barrier() const noexcept {
			gc_bits value = pointer.load();
			bool nmt_changed = false;
			if(value.address.seen_by_marker != the_gc->current_marker_phase) {
				nmt_changed = true;
			}
			bool relocated = false;
			if(the_arena->is_protected(value)) {
				relocated = true;
			}
			if(nmt_changed || relocated) {
				value = _gc_load_barrier_fix(value, nmt_changed, relocated);
			}
			return value;
		}

		gc_bits _gc_load_barrier_fix(gc_bits value, bool nmt_changed, bool relocated) const noexcept {
			gc_bits old_value = value;
			if(nmt_changed) {
				value.address.seen_by_marker = the_gc->current_marker_phase;
				object_base* ob = reinterpret_cast<object_base*>(the_arena->_gc_resolve(value));
				the_gc->queue_object(ob);
			}
			if(relocated) {
				if(value.header.relocate != gc_bits::moved) {
					// TODO perform relocate
				}
				// forwarded address stored in the corpse of the old object
				object_base* ob = reinterpret_cast<object_base*>(the_arena->_gc_resolve(value));
				gc_bits relocated_location = ob->count.load(std::memory_order_acquire);
				value.address.page_number = relocated_location.address.page_number;
				value.address.page_offset = relocated_location.address.page_offset;
			}
			pointer.compare_exchange_strong(old_value, value, std::memory_order_release);
			return value;
		}

	private:
		friend struct marker;
		friend struct nuller;

		template<typename T>
		friend struct reference_t;

		raw_reference(collector* the_gc_, arena* the_arena_, gc_bits bits) noexcept : the_gc(the_gc_), the_arena(the_arena_), pointer(bits) {
		}

		collector* the_gc;
		arena* the_arena;
		mutable gc_pointer pointer;
	};

	struct reference_registry
	{
		using registration = std::tuple<raw_reference*, object_base*>;

		void register_reference(raw_reference* ref, object_base* obj) {
			if(ref != nullptr) {
				std::lock_guard<std::mutex> guard(references_mtx);
				references[ref] = obj;
			}
		}
	
		void unregister_reference(raw_reference* ref) {
			if(ref != nullptr) {
				std::lock_guard<std::mutex> guard(references_mtx);
				references.erase(ref);
			}
		}

		virtual std::vector<registration> get_snapshot() {
			std::lock_guard<std::mutex> guard(references_mtx);
			std::vector<registration> snapshot;
			std::transform(references.begin(), references.end(), std::back_inserter(snapshot), [](auto p) { return std::make_tuple(p.first, p.second); });
			return snapshot;
		}

		virtual ~reference_registry() {
		}

	private:

		std::mutex references_mtx;
		std::map<raw_reference*, object_base*> references;
	};

	struct thread_data;

	struct global_data : reference_registry {
		void register_mutator_thread(thread_data* data) {
			gc_lock l(spinlock);
			mutator_threads[std::this_thread::get_id()] = data;
		}

		void unregister_mutator_thread() {
			gc_lock l(spinlock);
			mutator_threads.erase(std::this_thread::get_id());
		}

	private:
		gc_spinlock spinlock;
		std::unordered_map<std::thread::id, thread_data*> mutator_threads;

		std::vector<raw_reference*> globals;
		std::vector<raw_reference*> statics;
	};

	extern global_data globals;

	extern thread_local thread_data this_gc_thread;
	
	struct thread_data : reference_registry {
		thread_data() : is_mutator_thread(true) {
			globals.register_mutator_thread(this);
		}

		~thread_data() {
			if(is_mutator_thread) {
				globals.unregister_mutator_thread();
			}
		}

		thread_data(const thread_data&) = delete;

		// create one of these whenever we might have a reference that needs tracking
		// but which is currently not tracked
		struct guard_unsafe {
			guard_unsafe() {
				this_gc_thread.enter_unsafe();
			}

			~guard_unsafe() {
				this_gc_thread.leave_unsafe();
			}
		};

		virtual std::vector<registration> get_snapshot() {
			std::lock_guard<std::recursive_mutex> guard(unsafe);
			std::vector<registration> snapshot = reference_registry::get_snapshot();
			for(registration& reg : snapshot) {
				std::get<0>(reg) = nullptr;
			}
			return snapshot;
		}

		void set_as_gc_thread() {
			globals.unregister_mutator_thread();
			is_mutator_thread = false;
		}

	private:
		void enter_unsafe() {
			unsafe.lock();
		}

		void leave_unsafe() {
			unsafe.unlock();
		}

		std::recursive_mutex unsafe;

		bool is_mutator_thread;
	};

	extern thread_local thread_data this_gc_thread;

	// a typed GCed reference
	template<typename T>
	struct reference_t : raw_reference
	{
		static_assert(std::is_base_of_v<object, T>, "T must be derived from object");

		using referenced_type = T;

		using my_type   = reference_t<T>;
		using base_type = raw_reference;

		reference_t() noexcept : base_type() {
		}

		reference_t(std::nullptr_t) noexcept : base_type(nullptr) {
		}

		reference_t(const my_type& rhs) noexcept : base_type(rhs) {
		}

		template<typename U, typename = std::enable_if_t<(std::is_base_of_v<T, U> && std::is_convertible_v<U*, T*>)|| (is_array_v<T> && std::is_same_v<T, U>) > >
		reference_t(const reference_t<U>& rhs) noexcept : base_type(rhs) {
			fix_reference(rhs._gc_resolve());
		}

		template<typename U, typename = std::enable_if_t<(std::is_base_of_v<T, U> && std::is_convertible_v<U*, T*>) || (is_array_v<T> && std::is_same_v<T, U>) > >
		my_type& operator=(const reference_t<U>& rhs) noexcept {
			base_type::operator=(rhs);
			fix_reference(rhs._gc_resolve());
			return *this;
		}

		my_type& operator=(std::nullptr_t) noexcept {
			base_type::operator=(nullptr);
			return *this;
		}

		referenced_type* operator->() const noexcept {
			return _gc_resolve();
		}

		template<typename U = T>
		std::enable_if_t<is_array_v<U>, typename U::element_type&>
		operator[](size_t offset) const noexcept {
			return _gc_resolve()->operator[](offset);
		}

	protected:
		referenced_type* _gc_resolve() const noexcept {
			return reinterpret_cast<referenced_type*>(raw_reference::_gc_resolve());
		}

	private:
		template<typename T>
		friend struct reference_t;

		friend struct collector;

		template<typename T>
		friend struct array;

		reference_t(collector* the_gc_, arena* the_arena_, gc_bits bits) noexcept : raw_reference(the_gc_, the_arena_, bits) {
		}

		template<typename U>
		void fix_reference(U* source) {
			T* destination  = dynamic_cast<T*>(source);
			object_base* ob = dynamic_cast<object_base*>(source);
			ptrdiff_t typed_offset = (reinterpret_cast<byte*>(destination) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
			gc_bits current = pointer.load(std::memory_order_acquire);
			current.address.typed_offset = typed_offset;
			pointer.store(current, std::memory_order_release);
		}
	};

	template<typename T, typename Self>
	struct registered_reference : reference_t<T>
	{
		using my_type = registered_reference<T, Self>;
		using base_type = reference_t<T>;

		using base_type::base_type;
		using base_type::operator=;

	protected:
		virtual void assign(const raw_reference& rhs) noexcept override {
			raw_reference::assign(rhs);
			if(*this) {
				static_cast<Self*>(this)->get_registrar()->register_reference(this, this->_gc_resolve());
			}
		}

		virtual void clear() noexcept override {
			if(*this) {
				static_cast<Self*>(this)->get_registrar()->unregister_reference(this);
			}
			raw_reference::clear();
		}
	};

	// a typed GCed reference that registers itself as a field (no registration)
	template<typename T>
	struct member_t : reference_t<T> {
		using base_type = reference_t<T>;
		using my_type   = member_t<T>;

		using base_type::base_type;
	};

	// a typed GCed reference that registers itself as a global
	template<typename T>
	struct global_t : registered_reference<T, global_t<T> > {
		using base_type = registered_reference<T, global_t<T> >;
		using my_type = global_t<T>;

		using base_type::base_type;
		using base_type::operator=;

		reference_registry* get_registrar() const {
			return &globals;
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) = delete;
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) = delete;
	};

	// a typed GCed reference that registers itself as a stack variable
	template<typename T>
	struct handle_t : registered_reference<T, handle_t<T> >
	{
		using base_type = registered_reference<T, handle_t<T> >;
		using my_type   = handle_t<T>;

		using base_type::base_type;

		reference_registry* get_registrar() const {
			return &this_gc_thread;
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) = delete;
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) = delete;
	};

	template<typename To, typename From>
	handle_t<normalize_t<To>> inline collector::gc_cast(const reference_t<From>& rhs) {
		using real_type = normalize_t<To>;
		if(!rhs) {
			return handle_t<real_type>();
		}

		From* object = rhs._gc_resolve();
		real_type* destination = dynamic_cast<real_type*>(object);
		if(destination) {
			object_base* ob = dynamic_cast<object_base*>(destination);
			ob->_gc_add_ref();
			ptrdiff_t typed_offset = (reinterpret_cast<byte*>(destination) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
			return handle_t<real_type>(reference_t<real_type>(this, &the_arena, gc_bits::_gc_build_reference(this, &the_arena, ob, typed_offset)));
		} else {
			return handle_t<real_type>();
		}
	}

	template<typename T, typename... Args>
	std::enable_if_t<!is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> inline collector::gcnew(Args&&... args) {
		thread_data::guard_unsafe g;
		using real_type = normalize_t<T>;

		void* memory = the_arena.allocate(sizeof(real_type));
		if(!memory) {
			throw std::bad_alloc();
		}

		real_type* object = new(memory) real_type(std::forward<Args>(args)...);
		object_base* ob = dynamic_cast<object_base*>(object);
		ptrdiff_t typed_offset = (reinterpret_cast<byte*>(object) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
		object->_gc_set_block_unsafe(this, &the_arena, memory, ob);
		return handle_t<real_type>(reference_t<real_type>(this, &the_arena, gc_bits::_gc_build_reference(this, &the_arena, ob, typed_offset)));
	}

	template<typename T>
	std::enable_if_t<is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> inline collector::gcnew(size_t len) {
		thread_data::guard_unsafe g;
		using real_type = normalize_t<T>;

		void* memory = the_arena.allocate(real_type::allocation_size(len));
		if(!memory) {
			throw std::bad_alloc();
		}

		real_type* object = new(memory) real_type(len);
		object_base* ob = dynamic_cast<object_base*>(object);
		ptrdiff_t typed_offset = (reinterpret_cast<byte*>(object) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
		object->_gc_set_block_unsafe(this, &the_arena, memory, ob);
		return handle_t<real_type>(reference_t<real_type>(this, &the_arena, gc_bits::_gc_build_reference(this, &the_arena, ob, typed_offset)));
	}

	template<typename T>
	std::enable_if_t<is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> inline collector::gcnew(std::initializer_list<typename normalize_t<T>::element_type> init) {
		thread_data::guard_unsafe g;
		using real_type = normalize_t<T>;

		size_t size = init.size();
		void* memory = the_arena.allocate(real_type::allocation_size(size));
		if(!memory) {
			throw std::bad_alloc();
		}

		real_type* object = new(memory) real_type(init);
		object_base* ob = dynamic_cast<object_base*>(object);
		ptrdiff_t typed_offset = (reinterpret_cast<byte*>(object) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
		object->_gc_set_block_unsafe(this, &the_arena, memory, ob);
		return handle_t<real_type>(reference_t<real_type>(this, &the_arena, gc_bits::_gc_build_reference(this, &the_arena, ob, typed_offset)));
	}

	template<typename T>
	using reference = reference_t<normalize_t<T>>;

	template<typename T>
	using handle = handle_t<normalize_t<T>>;

	template<typename T>
	using member = member_t<normalize_t<T>>;

	template<typename T>
	using global = global_t<normalize_t<T>>;
}
