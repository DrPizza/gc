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

namespace gc
{

	union gc_bits
	{
		size_t value;
		union
		{
			struct
			{
				size_t    page_offset : 12; // 4K pages => 12 bit page offset
				size_t    page_number : 28;
				size_t    generation  : 2;
				size_t    nmt         : 2;
				ptrdiff_t base_offset : 12; // where to find the start of the object_base, relative to this address, in void*s
			} address;
			struct
			{
				size_t raw_address       : 56;
				size_t colour            : 2;
				size_t relocate          : 2;  // 0 = resident, 1 = moving, 2 = moved
				size_t reference_count   : 3;  // 0-6 = rced, 7 = gced
				size_t unused            : 1;
			} header;
			struct
			{
				size_t raw_address : 56;
				size_t raw_header  : 8;
			} layout;
		} bits;
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

		static gc_bits _gc_build_reference(void* memory, ptrdiff_t base_offset);

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

	struct object_base;

	struct raw_reference;

	template<typename T>
	struct reference;

	template<typename T>
	struct handle;

	template<typename T>
	struct array;

	struct finalized_object;

	struct visitor
	{
		virtual void trace(const raw_reference& ref) = 0;
	};

	struct marker : visitor
	{
		virtual void trace(const raw_reference& ref);
	};

	struct nuller : visitor
	{
		virtual void trace(const raw_reference& ref);
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

		size_t get_block_size(void* block) const noexcept {
			byte* region = static_cast<byte*>(block);
			allocation_header* header = reinterpret_cast<allocation_header*>(region - sizeof(allocation_header));
			return header->allocated_size.load(std::memory_order_acquire);
		}

		void* _gc_resolve(const gc_bits location) const noexcept {
			if(location.raw.pointer == 0) {
				return nullptr;
			}
			const size_t offset = (location.bits.address.page_number * page_size) + (location.bits.address.page_offset << maximum_alignment_shift);
			return base + offset;
		}
	
		gc_bits _gc_unresolve(const void* location) const noexcept {
			if(location == nullptr) {
				return gc_bits{ 0 };
			}
			const size_t offset = static_cast<size_t>(static_cast<const byte*>(location) - base);
			
			//assert(offset % maximum_alignment == 0);

			gc_bits value = { 0 };
			value.bits.address.page_number = offset / page_size;
			value.bits.address.page_offset = (offset % page_size) >> maximum_alignment_shift;
			return value;
		}
	};

	struct bit_table
	{
		bit_table(size_t size) : bits(new std::atomic<uint8_t>[(size / arena::maximum_alignment) / CHAR_BIT]) {

		}
		void set_bit(void* address) {
			std::pair<size_t, uint8_t> position = get_bit_position(address);
			bits[position.first].fetch_or ( static_cast<uint8_t>(1ui8 << position.second), std::memory_order_release);
		}

		void clear_bit(void* address) {
			std::pair<size_t, uint8_t> position = get_bit_position(address);
			bits[position.first].fetch_and(~static_cast<uint8_t>(1ui8 << position.second), std::memory_order_release);
		}

		std::pair<size_t, uint8_t> get_bit_position(void* address);

		std::unique_ptr< std::atomic<uint8_t /*std::byte*/>[] > bits;
	};

	struct collector
	{
		collector(size_t size_ = 1ui64 << 30) : the_arena(size_), reachables(size_), allocateds(size_) {
		}

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

		std::atomic<size_t> current_nmt;
		std::atomic<size_t> condemned_colour;
		std::atomic<size_t> scanned_colour;

	private:
		friend struct object_base;

		using root_set = std::vector<object_base*>;

		void flip();

		void scan_roots();

		using mark_queue = void;

		bit_table reachables;
		bit_table allocateds;

	};

	extern collector the_gc;

	inline std::pair<size_t, uint8_t> bit_table::get_bit_position(void* addr) {
		assert(addr != nullptr);
		const gc_bits offset = the_gc.the_arena._gc_unresolve(addr);
		const size_t block_number = offset.raw.pointer / (arena::maximum_alignment / CHAR_BIT);
		const size_t byte_number = block_number / CHAR_BIT;
		const uint8_t bit_number = block_number % CHAR_BIT;
		return { byte_number, bit_number };
	}

	struct object_base
	{
		object_base() noexcept {
			gc_bits bits = count.load(std::memory_order_acquire);
			bits.bits.header.reference_count = 1;
			count.store(bits, std::memory_order_release);
		}

		object_base(const object_base&) = delete;
		object_base(object_base&&) = delete;
		object_base& operator=(const object_base&) = delete;
		object_base& operator=(object_base&&) = delete;

		virtual ~object_base() {
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) { return ptr; }
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) { return ptr; }

		virtual void _gc_trace(visitor*) const = 0;

	protected:
		void _gc_set_block_unsafe(void* addr) noexcept {
			gc_bits location = the_gc.the_arena._gc_unresolve(addr);
			gc_bits current = count.load(std::memory_order_acquire);
			current.bits.header.raw_address = location.bits.header.raw_address;
			count.store(current, std::memory_order_release);
		}

		void* _gc_get_block() const noexcept {
			gc_bits location = count.load(std::memory_order_acquire);
			return the_gc.the_arena._gc_resolve(location);
		}

	private:
		mutable gc_pointer count;

		template<typename T, typename... Args>
		friend std::enable_if_t<!std::is_array_v<T>, handle<T> > gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(std::initializer_list<typename array<std::remove_extent_t<T> >::element_type> init);

		template<typename To, typename From>
		friend handle<To> gc_cast(const reference<From>& rhs);

		friend struct marker;
		friend struct raw_reference;

		size_t _gc_add_ref() const noexcept {
			for(;;) {
				gc_bits current = count.load(std::memory_order_acquire);
				if(current.bits.header.reference_count == get_max_ref_count()) {
					return get_max_ref_count();
				}
				gc_bits new_value = current;
				++new_value.bits.header.reference_count;
				if(count.compare_exchange_strong(current, new_value)) {
					return new_value.bits.header.reference_count;
				}
			}
		}

		size_t _gc_del_ref() const noexcept {
			for(;;) {
				gc_bits current = count.load(std::memory_order_acquire);
				if(current.bits.header.reference_count == get_max_ref_count()) {
					return get_max_ref_count();
				}
				gc_bits new_value = current;
				--new_value.bits.header.reference_count;
				if(count.compare_exchange_strong(current, new_value)) {

					if(current.bits.header.reference_count == 1) {
						//void* block = this->_gc_get_block();
						this->~object_base();
						//the_gc.the_arena.deallocate(block);
					}
					return new_value.bits.header.reference_count;
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
	alignas(std::conditional_t<std::is_base_of_v<object, T>, reference<T>, T>)
	alignas(object)
	alignas(size_t)
	array : object
	{
		template<typename T>
		friend std::enable_if_t<std::is_array_v<T>, handle<T> > gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t<std::is_array_v<T>, handle<T> > gcnew(std::initializer_list<typename array<std::remove_extent_t<T> >::element_type> init);

		using element_type = std::conditional_t<std::is_base_of_v<object, T>, reference<T>, T>;

		array() = delete;
		array(const array&) = delete;

		explicit array(size_t length_) : len(length_) {
			element_type* elts = elements();
			for(size_t i = 0; i < len; ++i) {
				new(&elts[i]) element_type();
			}
		}
	
		explicit array(std::initializer_list<element_type> init) : len(init.size()) {
			size_t offset = 0;
			for(const element_type& e : init) {
				new(&(elements()[offset++])) element_type(e);
			}
		}

		element_type& operator[](size_t offset) {
			return elements()[offset];
		}

		const element_type& operator[](size_t offset) const {
			return elements()[offset];
		}

		size_t length() const {
			return len;
		}

		virtual void _gc_trace(visitor* v) const override {
			return _gc_trace_dispatch(v);
		}

	private:
		static size_t allocation_size(size_t len) {
			return sizeof(array<T>) + (len * sizeof(element_type));
		}

		element_type* elements() {
			return reinterpret_cast<element_type*>(this + 1);
		}

		const element_type* elements() const {
			return reinterpret_cast<const element_type*>(this + 1);
		}

		template<typename U = T>
		std::enable_if_t<std::is_base_of_v<object, U>> _gc_trace_dispatch(visitor* visitor) const {
			const element_type* es = elements();
			for(size_t i = 0; i < len; ++i) {
				if(es[i]) {
					visitor->trace(es[i]);
				}
			}
			return object::_gc_trace(visitor);
		}

		template<typename U = T>
		std::enable_if_t<!std::is_base_of_v<object, U>> _gc_trace_dispatch(visitor* visitor) const {
			return object::_gc_trace(visitor);
		}

		size_t len;
		// element_type elements[];
	};

	struct raw_reference
	{
		explicit operator bool() const noexcept {
			return pointer.load().raw.pointer != gc_bits::zero.raw.pointer;
		}

		bool operator==(const raw_reference& rhs) const noexcept {
			return pointer.load().raw.pointer == rhs.pointer.load().raw.pointer ||
			       _gc_resolve_base() == rhs._gc_resolve_base();
		}

		bool operator!=(const raw_reference& rhs) const noexcept {
			return !(*this == rhs);
		}

	protected:
		raw_reference() noexcept : pointer() {
		}

		raw_reference(std::nullptr_t) noexcept : pointer() {
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
			pointer.store(rhs.pointer.load());
		}

		virtual void clear() noexcept {
			if(object_base* obj = _gc_resolve_base()) {
				obj->_gc_del_ref();
			}
			pointer.store(gc_bits::zero, std::memory_order_release);
		}

		virtual ~raw_reference() noexcept {
			clear();
		}

		void* _gc_resolve() const noexcept {
			if(!pointer.load(std::memory_order_acquire).raw.pointer) {
				return nullptr;
			}
			gc_bits value = _gc_read_barrier();
			object_base* ob = reinterpret_cast<object_base*>(the_gc.the_arena._gc_resolve(value));
			byte* derived_object = reinterpret_cast<byte*>(ob) + (value.bits.address.base_offset * static_cast<ptrdiff_t>(sizeof(void*)));
			return derived_object;
		}

		object_base* _gc_resolve_base() const noexcept {
			if(!pointer.load(std::memory_order_acquire).raw.pointer) {
				return nullptr;
			}
			gc_bits value = _gc_read_barrier();
			object_base* ob = reinterpret_cast<object_base*>(the_gc.the_arena._gc_resolve(value));
			return ob;
		}

		void _gc_write_barrier() const noexcept {
			// awkward. if my retaining object is oldgen, and the new object is newgen, I need to treat that specially.
			// but I don't know what my retaining object is.
		}

		gc_bits _gc_read_barrier() const noexcept {
			gc_bits value = pointer.load();
			bool nmt_changed = false;
			if(value.bits.address.nmt != the_gc.current_nmt) {
				nmt_changed = true;
			}
			bool relocated = false;
			if(the_gc.the_arena.is_protected(value)) {
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
				value.bits.address.nmt = the_gc.current_nmt;
				object_base* ob = reinterpret_cast<object_base*>(the_gc.the_arena._gc_resolve(value));
				the_gc.queue_object(ob);
			}
			if(relocated) {
				if(value.bits.header.relocate != gc_bits::moved) {
					// TODO perform relocate
				}
				// forwarded address stored in the corpse of the old object
				object_base* ob = reinterpret_cast<object_base*>(the_gc.the_arena._gc_resolve(value));
				gc_bits relocated_location = ob->count.load(std::memory_order_acquire);
				value.bits.address.page_number = relocated_location.bits.address.page_number;
				value.bits.address.page_number = relocated_location.bits.address.page_number;
			}
			pointer.compare_exchange_strong(old_value, value, std::memory_order_release);
			return value;
		}

	private:
		friend struct marker;
		friend struct nuller;

		template<typename T>
		friend struct reference;

		raw_reference(gc_bits bits) noexcept : pointer(bits) {
		}

		mutable gc_pointer pointer;
	};

	struct reference_registry
	{
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

		std::vector<object_base*> get_snapshot() {
			std::lock_guard<std::mutex> guard(references_mtx);
			std::vector<object_base*> snapshot;
			std::transform(references.begin(), references.end(), std::back_inserter(snapshot), [](auto p) { return p.second; });
			return snapshot;
		}

	private:
		using registration = std::tuple<raw_reference*, object_base*>;

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

		std::vector<object_base*> get_snapshot() {
			std::lock_guard<std::recursive_mutex> guard(unsafe);
			return reference_registry::get_snapshot();
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
	struct reference : raw_reference
	{
		static_assert(std::is_base_of_v<object, T> || std::is_array_v<T>, "T must be derived from object");

		using referenced_type = std::conditional_t<std::is_array_v<T>, array<std::remove_extent_t<T> >, T>;

		using my_type   = reference<T>;
		using base_type = raw_reference;

		reference() noexcept : base_type() {
		}

		reference(std::nullptr_t) noexcept : base_type(nullptr) {
		}

		// TODO ensure equivalence between reference<T[]> and reference<array<T> >
		reference(const my_type& rhs) noexcept : base_type(rhs) {
		}

		template<typename U, typename = std::enable_if_t<(std::is_base_of_v<T, U> && std::is_convertible_v<U*, T*>)|| (std::is_array_v<T> && std::is_same_v<T, U>) > >
		reference(const reference<U>& rhs) noexcept : base_type(rhs) {
			fix_reference(rhs._gc_resolve());
		}

		template<typename U, typename = std::enable_if_t<(std::is_base_of_v<T, U> && std::is_convertible_v<U*, T*>) || (std::is_array_v<T> && std::is_same_v<T, U>) > >
		my_type& operator=(const reference<U>& rhs) noexcept {
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
		std::enable_if_t<std::is_array_v<U>, typename array<std::remove_extent_t<U>>::element_type>
		operator[](size_t offset) const noexcept {
			return _gc_resolve()->operator[](offset);
		}

	protected:
		referenced_type* _gc_resolve() const noexcept {
			return reinterpret_cast<referenced_type*>(raw_reference::_gc_resolve());
		}

	private:
		template<typename T>
		friend struct reference;

		template<typename T, typename... Args>
		friend std::enable_if_t<!std::is_array_v<T>, handle<T> > gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(std::initializer_list<typename array<std::remove_extent_t<T> >::element_type> init);
		template<typename T>
		friend struct array;

		template<typename To, typename From>
		friend handle<To> gc_cast(const reference<From>& rhs);

		reference(gc_bits bits) noexcept : raw_reference(bits) {
		}

		template<typename U>
		void fix_reference(U* source) {
			T* destination  = dynamic_cast<T*>(source);
			object_base* ob = dynamic_cast<object_base*>(source);
			ptrdiff_t base_offset = (reinterpret_cast<byte*>(destination) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
			gc_bits new_address = gc_bits::_gc_build_reference(ob, base_offset);
			gc_bits current = pointer.load(std::memory_order_acquire);
			current.bits.address.page_number = new_address.bits.address.page_number;
			current.bits.address.page_offset = new_address.bits.address.page_offset;
			current.bits.address.base_offset = new_address.bits.address.base_offset;
			pointer.store(current, std::memory_order_release);
		}
	};

	template<typename T, typename Self>
	struct registered_reference : reference<T>
	{
		using my_type = registered_reference<T, Self>;
		using base_type = reference<T>;

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
	struct member : reference<T> {
		using base_type = reference<T>;
		using my_type   = member<T>;

		using base_type::base_type;
	};

	// a typed GCed reference that registers itself as a global
	template<typename T>
	struct global : registered_reference<T, global<T> > {
		using base_type = registered_reference<T, global<T> >;
		using my_type = global<T>;

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
	struct handle : registered_reference<T, handle<T> >
	{
		using base_type = registered_reference<T, handle<T> >;
		using my_type   = handle<T>;

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
	handle<To> gc_cast(const reference<From>& rhs) {
		if(!rhs) {
			return handle<To>();
		}

		From* object = rhs._gc_resolve();
		To* destination = dynamic_cast<To*>(object);
		if(destination) {
			object_base* ob = dynamic_cast<object_base*>(destination);
			ob->_gc_add_ref();
			ptrdiff_t base_offset = (reinterpret_cast<byte*>(destination) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
			return handle<To>(reference<To>(gc_bits::_gc_build_reference(ob, base_offset)));
		}
		else {
			return handle<To>();
		}
	}

	template<typename T, typename... Args>
	std::enable_if_t<!std::is_array_v<T>, handle<T> > inline gcnew(Args&&... args) {
		thread_data::guard_unsafe g;
		static_assert(std::is_base_of_v<object, T>, "T must be derived from object");
		void* memory = the_gc.the_arena.allocate(sizeof(T));
		if(!memory) {
			throw std::bad_alloc();
		}

		gc_bits allocation_base = the_gc.the_arena._gc_unresolve(memory);
		T* object = new(memory) T(std::forward<Args>(args)...);
		object_base* ob = dynamic_cast<object_base*>(object);
		ptrdiff_t base_offset = (reinterpret_cast<byte*>(object) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
		object->_gc_set_block_unsafe(memory);
		return handle<T>(reference<T>(gc_bits::_gc_build_reference(ob, base_offset)));
	}

	template<typename T>
	std::enable_if_t<std::is_array_v<T>, handle<T> > inline gcnew(size_t len) {
		thread_data::guard_unsafe g;
		using array_type = array<std::remove_extent_t<T> >;
		void* memory = the_gc.the_arena.allocate(array_type::allocation_size(len));
		if(!memory) {
			throw std::bad_alloc();
		}

		array_type* object = new(memory) array_type(len);
		object_base* ob = dynamic_cast<object_base*>(object);
		ptrdiff_t base_offset = (reinterpret_cast<byte*>(object) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
		object->_gc_set_block_unsafe(memory);
		return handle<T>(reference<T>(gc_bits::_gc_build_reference(ob, base_offset)));
	}

	template<typename T>
	std::enable_if_t<std::is_array_v<T>, handle<T> > inline gcnew(std::initializer_list<typename array<std::remove_extent_t<T> >::element_type> init) {
		thread_data::guard_unsafe g;
		using array_type = array<std::remove_extent_t<T> >;
		size_t size = init.size();
		void* memory = the_gc.the_arena.allocate(array_type::allocation_size(size));
		if(!memory) {
			throw std::bad_alloc();
		}

		array_type* object = new(memory) array_type(init);
		object_base* ob = dynamic_cast<object_base*>(object);
		ptrdiff_t base_offset = (reinterpret_cast<byte*>(object) - reinterpret_cast<byte*>(ob)) / static_cast<ptrdiff_t>(sizeof(void*));
		object->_gc_set_block_unsafe(memory);
		return handle<T>(reference<T>(gc_bits::_gc_build_reference(ob, base_offset)));
	}
}
