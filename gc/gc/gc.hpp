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
				size_t page_offset : 12; // 4K pages => 12 bit page offset
				size_t page_number : 28;
				size_t generation  : 2;
				size_t nmt         : 2;
			} address;
			struct
			{
				size_t raw_address       : 44;
				size_t colour            : 2;
				size_t relocate          : 2;  // 0 = resident, 1 = moving, 2 = moved
				size_t reference_count   : 2;  // 0/1/2 = rced, 3 = gced
				size_t needs_destruction : 1;
				size_t is_embedded       : 1; // embedded objects aren't tracked separately
			} header;
			struct
			{
				size_t raw_address : 44;
				size_t raw_header  : 8;
				size_t unused      : 12;
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
			//free           = 3  // unreachable
		};

		static gc_bits _gc_build_reference(void* memory);
	};

	static_assert(sizeof(gc_bits) == sizeof(std::size_t), "gc_bits is the wrong size");

	using gc_pointer = std::atomic<gc_bits>;

	//static_assert(gc_pointer::is_always_lock_free(), "gc_pointer isn't lock-free");

	static constexpr size_t get_max_ref_count() noexcept {
		return 3;
	}

	template <class T>
	static inline constexpr
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool>
	is_power_of_two(T v) noexcept {
		return (v != 0) && (v & (v - 1)) == 0;
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
		void mark(raw_reference&) {
		}
	};

	struct arena
	{
		static constexpr size_t      size = 1 << 30;
		static constexpr size_t page_size = 1 << 12;
		static constexpr size_t page_count = size / page_size;

		void* section;
		byte* base;
		byte* base_shadow;
		byte* rw_base;
		byte* rw_base_shadow;
		std::atomic<size_t> low_watermark;
		std::atomic<size_t> high_watermark;

		arena();
		~arena();
		arena(const arena&) = delete;
		arena(arena&&) = delete;
		arena& operator=(const arena&) = delete;
		arena& operator=(arena&&) = delete;

		static constexpr size_t maximum_alignment = 512 / CHAR_BIT; // AVX512 alignment is the worst we're ever likely to see
		static_assert(is_power_of_two(maximum_alignment), "maximum_alignment isn't a power of two");

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

		void* _gc_resolve(const gc_bits location) const noexcept {
			if(location.raw.pointer == 0) {
				return nullptr;
			}
			const size_t offset = (location.bits.address.page_number * page_size) + location.bits.address.page_offset;
			return base + offset;
		}
	
		gc_bits _gc_unresolve(const void* location) const noexcept {
			if(location == nullptr) {
				return gc_bits{ 0 };
			}
			const size_t offset = static_cast<size_t>(static_cast<const byte*>(location) - base);
			gc_bits value = { 0 };
			value.bits.address.page_number = offset / page_size;
			value.bits.address.page_offset = offset % page_size;
			return value;
		}
	};

	struct bit_table {
		void set_bit(void* address) {
			std::pair<size_t, uint8_t> position = get_bit_position(address);
			bits[position.first].fetch_or(static_cast<uint8_t>(1ui8 << position.second), std::memory_order_release);
		}

		std::pair<size_t, uint8_t> get_bit_position(void* address);

		std::array<std::atomic<uint8_t /*std::byte*/>, (arena::size / arena::maximum_alignment) / CHAR_BIT> bits;
	};

	struct collector {
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

		void register_finalizable_object(void* addr) {
			finalizable_objects.set_bit(addr);
		}

		using root_set = std::vector<object_base*>;

		void flip();

		void scan_roots();

		using mark_queue = void;

		bit_table finalizable_objects;
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

		virtual void _gc_trace(visitor*) = 0;

	protected:
		template<bool needs_finalization>
		void _gc_set_block_unsafe(void* addr) noexcept {
			gc_bits location = the_gc.the_arena._gc_unresolve(addr);
			gc_bits current = count.load(std::memory_order_acquire);
			current.bits.header.raw_address = location.bits.header.raw_address;
			count.store(current, std::memory_order_release);
			if constexpr(needs_finalization) {
				the_gc.register_finalizable_object(addr);
			}
		}

		void* _gc_get_block() noexcept {
			gc_bits bits = count.load(std::memory_order_acquire);
			return the_gc.the_arena._gc_resolve(bits);
		}

	private:
		gc_pointer count;

		template<typename T, typename... Args>
		friend std::enable_if_t<!std::is_array_v<T>, handle<T> > gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(std::initializer_list<typename array<std::remove_extent_t<T> >::element_type> init);

		friend struct raw_reference;
		friend struct finalized_object;

		size_t _gc_add_ref() noexcept {
			if(count.load(std::memory_order_relaxed).bits.header.reference_count != get_max_ref_count()) {
				for(;;) {
					gc_bits current = count.load(std::memory_order_acquire);
					gc_bits new_value = current;
					if(new_value.bits.header.reference_count == get_max_ref_count()) {
						return get_max_ref_count();
					}
					++new_value.bits.header.reference_count;
					if(count.compare_exchange_strong(current, new_value)) {
						return new_value.bits.header.reference_count;
					}
				}
			}
			return get_max_ref_count();
		}

		size_t _gc_del_ref() noexcept {
			if(count.load(std::memory_order_relaxed).bits.header.reference_count != get_max_ref_count()) {
				for(;;) {
					gc_bits current = count.load(std::memory_order_acquire);
					gc_bits new_value = current;
					if(new_value.bits.header.reference_count == get_max_ref_count()) {
						return get_max_ref_count();
					}
					--new_value.bits.header.reference_count;
					if(count.compare_exchange_strong(current, new_value)) {
						if(current.bits.header.reference_count == 1) {
							this->~object_base();
							// TODO deallocate the object?
						}
						return new_value.bits.header.reference_count;
					}
				}
			}
			return get_max_ref_count();
		}
	};

	struct object : virtual object_base
	{
		virtual void _gc_trace(visitor*) override {
		}
	};

	struct finalized_object : object
	{
		finalized_object() noexcept {
			gc_bits bits = count.load();
			bits.bits.header.needs_destruction = 1;
			count.store(bits);
		}

		virtual void _gc_finalize() {
		}

		virtual ~finalized_object() override {
			_gc_finalize();
			gc_bits bits = count.load();
			bits.bits.header.needs_destruction = 0;
			count.store(bits);
		}
	};

	template<typename T>
	struct
	alignas(std::conditional_t<std::is_base_of_v<object, T>, reference<T>, T>)
	alignas(std::conditional_t<std::is_base_of_v<finalized_object, T>, finalized_object, object>)
	alignas(size_t)
	array : std::conditional_t<std::is_base_of_v<finalized_object, T>, finalized_object, object>
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

		size_t length() const {
			return len;
		}

		virtual void _gc_trace(visitor* v) override {
			return _gc_trace_dispatch(v);
		}

	private:
		static size_t allocation_size(size_t len) {
			return sizeof(array<T>) + (len * sizeof(element_type));
		}

		element_type* elements() {
			return reinterpret_cast<element_type*>(this + 1);
		}

		template<typename U = T>
		std::enable_if_t<std::is_base_of_v<object, U>> _gc_trace_dispatch(visitor* visitor) {
			element_type* es = elements();
			for(size_t i = 0; i < len; ++i) {
				if(es[i]) {
					es[i]->_gc_trace(visitor);
				}
			}
			return object::_gc_trace(visitor);
		}

		template<typename U = T>
		std::enable_if_t<!std::is_base_of_v<object, U>> _gc_trace_dispatch(visitor* visitor) {
			return object::_gc_trace(visitor);
		}

		size_t len;
		// element_type elements[];
	};

	struct raw_reference
	{
		explicit operator bool() const noexcept {
			return _gc_resolve() != nullptr;
		}

		bool operator==(const raw_reference& rhs) const noexcept {
			return pointer.load().raw.pointer == rhs.pointer.load().raw.pointer ||
			       _gc_resolve() == rhs._gc_resolve();
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
			if(object_base* obj = rhs._gc_resolve()) {
				obj->_gc_add_ref();
			}
			_gc_write_barrier();
			pointer.store(rhs.pointer.load());
		}

		raw_reference& operator=(const raw_reference& rhs) noexcept {
			if(this != &rhs) {
				this->~raw_reference();
				new(this) raw_reference(rhs);
			}
			return *this;
		}

		raw_reference& operator=(std::nullptr_t) noexcept {
			this->~raw_reference();
			new(this) raw_reference();
			return *this;
		}

		virtual ~raw_reference() noexcept {
			if(object_base* obj = _gc_resolve()) {
				obj->_gc_del_ref();
			}
			pointer.store(gc_bits{ 0 }, std::memory_order_release);
		}

		object_base* _gc_resolve() const noexcept {
			if(!pointer.load(std::memory_order_acquire).raw.pointer) {
				return nullptr;
			}
			gc_bits value = _gc_read_barrier();
			return reinterpret_cast<object_base*>(the_gc.the_arena._gc_resolve(value));
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
				the_gc.queue_object(reinterpret_cast<object*>(the_gc.the_arena._gc_resolve(value)));
			}
			if(relocated) {
				if(value.bits.header.relocate != gc_bits::moved) {
					// TODO perform relocate
				}
//				gc_bits new_location = arenas[value.bits.address.arena].get_new_location(value);
//				value.bits.address = new_location.bits.address;
				// forwarded address stored in the corpse of the old object
				value.bits.address = reinterpret_cast<object*>(the_gc.the_arena._gc_resolve(value))->count.load().bits.address;
			}
			pointer.compare_exchange_strong(old_value, value, std::memory_order_release);
			return value;
		}

	private:
		template<typename T>
		friend struct reference;

		raw_reference(gc_bits bits) noexcept : pointer(bits) {
		}

		mutable gc_pointer pointer;
	};

	struct reference_registry
	{
		void register_reference(raw_reference* ref, object* obj) {
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

		std::vector<object*> get_snapshot() {
			std::lock_guard<std::mutex> guard(references_mtx);
			std::vector<object*> snapshot;
			std::transform(references.begin(), references.end(), std::back_inserter(snapshot), [](auto p) { return p.second; });
			return snapshot;
		}

	private:
		using registration = std::tuple<raw_reference*, object*>;

		std::mutex references_mtx;
		std::map<raw_reference*, object*> references;
	};

	// a typed GCed reference
	template<typename T>
	struct reference : raw_reference
	{
		static_assert(std::is_base_of_v<object, T> || std::is_array_v<T>, "T must be derived from object");

		using referenced_type = std::conditional_t<std::is_array_v<T>, array<std::remove_extent_t<T> >, T>;

		using my_type = reference<T>;
		using base_type = raw_reference;

		reference() noexcept : base_type() {
		}

		reference(std::nullptr_t) noexcept : base_type(nullptr) {
		}

		// TODO ensure equivalence between reference<T[]> and reference<array<T> >
		reference(const reference& rhs) noexcept : base_type(rhs) {
		}

		template<typename U>
		reference(const reference<U>& rhs) noexcept : base_type(rhs) {
		}

		template<typename U>
		my_type& operator=(const reference& rhs) noexcept {
			base_type::operator=(rhs);
			return *this;
		}

		my_type& operator=(const base_type& rhs) noexcept {
			base_type::operator=(rhs);
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
			assert(dynamic_cast<referenced_type*>(raw_reference::_gc_resolve()) != nullptr);
			return dynamic_cast<referenced_type*>(raw_reference::_gc_resolve());
		}

	private:
		// used for stack references to gc heap objects
		template<typename T>
		friend struct handle;

		// used for gc object members of gc objects
		template<typename T>
		friend struct member;

		template<typename T, typename... Args>
		friend std::enable_if_t<!std::is_array_v<T>, handle<T> > gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< std::is_array_v<T>, handle<T> > gcnew(std::initializer_list<typename array<std::remove_extent_t<T> >::element_type> init);
		template<typename T>
		friend struct array;

		reference(gc_bits bits) noexcept : raw_reference(bits) {
		}
	};

	template<typename T, typename Self>
	struct registered_reference : reference<T>
	{
		using my_type = registered_reference<T, Self>;
		using base_type = reference<T>;

		registered_reference() {
			static_cast<Self*>(this)->get_registrar()->register_reference(this, this->_gc_resolve());
		}

		registered_reference(const my_type& rhs) : base_type(rhs) {
			static_cast<Self*>(this)->get_registrar()->register_reference(this, this->_gc_resolve());
		}

		registered_reference(const base_type& rhs) : base_type(rhs) {
			static_cast<Self*>(this)->get_registrar()->register_reference(this, this->_gc_resolve());
		}

		my_type& operator=(const my_type& rhs) {
			base_type::operator=(rhs);
			return *this;
		}

		my_type& operator=(const base_type& rhs) {
			base_type::operator=(rhs);
			return *this;
		}

		my_type& operator=(std::nullptr_t) {
			base_type::operator=(nullptr);
			return *this;
		}

		virtual ~registered_reference() override {
			static_cast<Self*>(this)->get_registrar()->unregister_reference(this);
		}

	};

	// a typed GCed reference that registers itself as a field (no registration)
	template<typename T>
	struct member;

	// a typed GCed reference that registers itself as a global
	template<typename T>
	struct global;

	// a typed GCed reference that registers itself as a class static (same as a global)
	template<typename T>
	struct gc_static;

	// a typed GCed reference that registers itself as a stack variable
	template<typename T>
	struct handle;

	template<typename T>
	struct handle : registered_reference<T, handle<T> >
	{
		using base_type = registered_reference<T, handle<T> >;

		handle() {
		}

		handle(const handle<T>& rhs) : base_type(rhs) {
		}

		handle(const reference<T>& rhs) : base_type(rhs) {
		}

		handle(const member<T>& rhs);

		handle<T>& operator=(const handle<T>& rhs) {
			if(this != &rhs) {
				this->~handle();
				new(this) handle<T>(rhs);
			}
			return *this;
		}

		handle<T>& operator=(const reference<T>& rhs) {
			if(this != &rhs) {
				this->~handle();
				new(this) handle<T>(rhs);
			}
			return *this;
		}

		handle<T>& operator=(const member<T>& rhs);

		virtual ~handle() override {
		}

		reference_registry* get_registrar() const {
			return &this_gc_thread;
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) = delete;
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) = delete;
	};

	template<typename T>
	struct member : reference<T>
	{
		using base_type = reference<T>;

		member() {
		}

		member(const member<T>& rhs) : base_type(rhs) {
		}

		member(const handle<T>& rhs) : base_type(rhs) {
		}

		member(const reference<T>& rhs) : base_type(rhs) {
		}

		member<T>& operator=(const member<T>& rhs) {
			if(this != &rhs) {
				this->~member();
				new(this) member<T>(rhs);
			}
			return *this;
		}

		member<T>& operator=(const reference<T>& rhs) {
			if(this != &rhs) {
				this->~member();
				new(this) member<T>(rhs);
			}
			return *this;
		}

		member<T>& operator=(const handle<T>& rhs) {
			this->~member();
			new(this) member<T>(rhs);
			return *this;
		}
	};

	template<typename T>
	handle<T>::handle(const member<T>& rhs) : handle<T>::base_type(rhs) {
	}

	template<typename T>
	handle<T>& handle<T>::operator=(const member<T>& rhs) {
		this->~handle();
		new(this) handle<T>(rhs);
		return *this;
	}

	struct thread_data;

	struct global_data : reference_registry
	{
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

	struct thread_data : reference_registry
	{
		thread_data() : is_mutator_thread(true) {
			globals.register_mutator_thread(this);
		}

		~thread_data() {
			if(is_mutator_thread) {
				globals.unregister_mutator_thread();
			}
		}
	
		thread_data(const thread_data&) = delete;

		struct guard_unsafe
		{
			guard_unsafe() {
				this_gc_thread.enter_unsafe();
			}

			~guard_unsafe() {
				this_gc_thread.leave_unsafe();
			}
		};
	
		std::vector<object*> get_snapshot() {
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
		object->_gc_set_block_unsafe<std::is_base_of_v<finalized_object, T> >(memory);
		return handle<T>(gc_bits::_gc_build_reference(dynamic_cast<object_base*>(object)));
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
		object->_gc_set_block_unsafe<std::is_base_of_v<finalized_object, array_type> >(memory);
		return handle<T>(gc_bits::_gc_build_reference(dynamic_cast<object_base*>(object)));
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
		object->_gc_set_block_unsafe<std::is_base_of_v<finalized_object, array_type> >(memory);
		return handle<T>(gc_bits::_gc_build_reference(dynamic_cast<object_base*>(object)));
	}

}
