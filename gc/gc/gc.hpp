#pragma once

#if !defined(_STL_EXTRA_DISABLED_WARNINGS)
#define _STL_EXTRA_DISABLED_WARNINGS 4061 4324 4365 4514 4571 4582 4583 4623 4625 4626 4710 4774 4820 4987 5026 5027 5039
#endif

#if !defined(_SCL_SECURE_NO_WARNINGS)
#define _SCL_SECURE_NO_WARNINGS 1
#endif

#if !defined(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 1
#endif

// disable for everything
#pragma warning(disable: 4324) // warning C4234: structure was padded due to alignment specifier
#pragma warning(disable: 4514) // warning C4514: '%s': unreferenced inline function has been removed
#pragma warning(disable: 4625) // warning C4625: '%s': copy constructor was implicitly defined as deleted
#pragma warning(disable: 4626) // warning C4626: '%s': assignment operator was implicitly defined as deleted
#pragma warning(disable: 4710) // warning C4710: '%s': function not inlined
#pragma warning(disable: 4820) // warning C4820: '%s': '%d' bytes padding added after data member '%s'
#pragma warning(disable: 5026) // warning C5026: '%s': move constructor was implicitly defined as deleted
#pragma warning(disable: 5027) // warning C5027: '%s': move assignment operator was implicitly defined as deleted

#pragma warning(disable: 26412) // warning C26412: Do not dereference an invalid pointer (lifetimes rule 1). 'return of %s' was invalidated at line %d by 'no initialization'.
#pragma warning(disable: 26481) // warning C26481: Don't use pointer arithmetic. Use span instead. (bounds.1: http://go.microsoft.com/fwlink/p/?LinkID=620413)
#pragma warning(disable: 26485) // warning C26485: Expression '%s::`vbtable'': No array to pointer decay. (bounds.3: http://go.microsoft.com/fwlink/p/?LinkID=620415)
#pragma warning(disable: 26490) // warning C26490: Don't use reinterpret_cast. (type.1: http://go.microsoft.com/fwlink/p/?LinkID=620417)
#pragma warning(disable: 26499) // warning C26499: Could not find any lifetime tracking information for '%s'

// disable for standard headers
#pragma warning(push)
#pragma warning(disable: 5039) // warning C5039: '%s': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.

#pragma warning(disable: 26400) // warning C26400: Do not assign the result of an allocation or a function call with an owner<T> return value to a raw pointer, use owner<T> instead. (i.11 http://go.microsoft.com/fwlink/?linkid=845474)
#pragma warning(disable: 26401) // warning C26401: Do not delete a raw pointer that is not an owner<T>. (i.11: http://go.microsoft.com/fwlink/?linkid=845474)
#pragma warning(disable: 26408) // warning C26408: Avoid malloc() and free(), prefer the nothrow version of new with delete. (r.10 http://go.microsoft.com/fwlink/?linkid=845483)
#pragma warning(disable: 26409) // warning C26409: Avoid calling new and delete explicitly, use std::make_unique<T> instead. (r.11 http://go.microsoft.com/fwlink/?linkid=845485)
#pragma warning(disable: 26411) // warning C26411: The parameter '%s' is a reference to unique pointer and it is never reassigned or reset, use T* or T& instead. (r.33 http://go.microsoft.com/fwlink/?linkid=845479)
#pragma warning(disable: 26412) // warning C26412: Do not dereference an invalid pointer (lifetimes rule 1). 'return of %s' was invalidated at line %d by 'end of function scope (local lifetimes end)'.
#pragma warning(disable: 26413) // warning C26413: Do not dereference nullptr (lifetimes rule 2). 'nullptr' was pointed to nullptr at line %d.
#pragma warning(disable: 26423) // warning C26423: The allocation was not directly assigned to an owner.
#pragma warning(disable: 26424) // warning C26424: Failing to delete or assign ownership of allocation at line %d.
#pragma warning(disable: 26425) // warning C26425: Assigning '%s' to a static variable.
#pragma warning(disable: 26429) // warning C26429: Symbol '%s' is never tested for nullness, it can be marked as not_null (f.23: http://go.microsoft.com/fwlink/?linkid=853921).
#pragma warning(disable: 26430) // warning C26430: Symbol '%s' is not tested for nullness on all paths (f.23: http://go.microsoft.com/fwlink/?linkid=853921).
#pragma warning(disable: 26432) // warning C26432: If you define or delete any default operation in the type '%s', define or delete them all (c.21: http://go.microsoft.com/fwlink/?linkid=853922).
#pragma warning(disable: 26434) // warning C26434: Function '%s' hides a non-virtual function '%s' (c.128: http://go.microsoft.com/fwlink/?linkid=853923).
#pragma warning(disable: 26436) // warning C26436: The type '%s' with a virtual function needs either public virtual or protected nonvirtual destructor (c.35: http://go.microsoft.com/fwlink/?linkid=853924).
#pragma warning(disable: 26437) // warning C26437: Do not slice (es.63: http://go.microsoft.com/fwlink/?linkid=853925).
#pragma warning(disable: 26439) // warning C26432: This kind of function may not throw. Declare it 'noexcept' (f.6: http://go.microsoft.com/fwlink/?linkid=853927).
#pragma warning(disable: 26440) // warning C26440: Function '%s' can be declared 'noexcept' (f.6: http://go.microsoft.com/fwlink/?linkid=853927).
#pragma warning(disable: 26460) // warning C26460: The reference argument '%s' for function '%s' can be marked as const (con.3: https://go.microsoft.com/fwlink/p/?LinkID=786684).
#pragma warning(disable: 26461) // warning C26461: The reference argument '%s' for function %s can be marked as const. (con.3: https://go.microsoft.com/fwlink/p/?LinkID=786684)
#pragma warning(disable: 26462) // warning C26462: The value pointed to by '%s' is assigned only once, mark it as a pointer to const (con.4: https://go.microsoft.com/fwlink/p/?LinkID=784969).
#pragma warning(disable: 26471) // warning C26471: Don't use reinterpret_cast. A cast from void* can use static_cast. (type.1: http://go.microsoft.com/fwlink/p/?LinkID=620417).
#pragma warning(disable: 26472) // warning C26472: Don't use a static_cast for arithmetic conversions. Use brace initialization, gsl::narrow_cast or gsl::narow (type.1: http://go.microsoft.com/fwlink/p/?LinkID=620417).
#pragma warning(disable: 26473) // warning C26473: Don't cast between pointer types where the source type and the target type are the same (type.1: http://go.microsoft.com/fwlink/p/?LinkID=620417).
#pragma warning(disable: 26474) // warning C26474: Don't cast between pointer types when the conversion could be implicit (type.1: http://go.microsoft.com/fwlink/p/?LinkID=620417).
#pragma warning(disable: 26475) // warning C26475: Do not use function style C-casts (es.49: http://go.microsoft.com/fwlink/?linkid=853930).
#pragma warning(disable: 26481) // warning C26481: Don't use pointer arithmetic. Use span instead. (bounds.1: http://go.microsoft.com/fwlink/p/?LinkID=620413)
#pragma warning(disable: 26482) // warning C26482: Only index into arrays using constant expressions. (bounds.2: http://go.microsoft.com/fwlink/p/?LinkID=620414)
#pragma warning(disable: 26490) // warning C26490: Don't use reinterpret_cast. (type.1: http://go.microsoft.com/fwlink/p/?LinkID=620417)
#pragma warning(disable: 26491) // warning C26491: Don't use static_cast downcasts (type.2: http://go.microsoft.com/fwlink/p/?LinkID=620418).
#pragma warning(disable: 26492) // warning C26492: Don't use const_cast to cast away const (type.3: http://go.microsoft.com/fwlink/p/?LinkID=620419).
#pragma warning(disable: 26493) // warning C26493: Don't use C-style casts that would perform a static_cast downcast, const_cast, or reinterpret_cast. (type.4: http://go.microsoft.com/fwlink/p/?LinkID=620420)
#pragma warning(disable: 26494) // warning C26494: Variable '%s' is uninitialized. Always initialize an object. (type.5: http://go.microsoft.com/fwlink/p/?LinkID=620421)
#pragma warning(disable: 26495) // warning C26495: Variable '%s' is uninitialized. Always initialize a member variable. (type.6: http://go.microsoft.com/fwlink/p/?LinkID=620422)
#pragma warning(disable: 26496) // warning C26496: Variable '%s' is assigned only once, mark it as const. (con.4: https://go.microsoft.com/fwlink/p/?LinkID=784969)
#pragma warning(disable: 26497) // warning C26497: This function %s could be marked constexpr if compile-time evaluation is desired. (f.4: https://go.microsoft.com/fwlink/p/?LinkID=784970)

#include <cstddef>
#include <thread>
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
#include <tuple>
#include <map>
#include <set>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

#include <gsl/gsl>

#pragma warning(pop)

#pragma pointers_to_members(full_generality, virtual_inheritance)
#pragma vtordisp(2)

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
			size_t    seen_by_marker : 1;
			size_t    valid          : 1;
			ptrdiff_t typed_offset   : 12; // where to find the start of the typed object, relative to the address of the object_base, in void*s.
			                               // this limits objects to 2^15 bytes. the C++ spec requires at least 2^18. maybe find some extra bits.
		} address;
		// used by object_base to track ref counts, mark status, whether the destructor has been called
		struct
		{
			size_t raw_address       : 56;
			size_t relocate          : 2;  // 0 = resident, 1 = moving, 2 = moved
			size_t reference_count   : 3;  // 0-6 = rced, 7 = gced
			size_t unused            : 3;
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
		};

		static gc_bits _gc_build_reference(gsl::not_null<object_base*> base_pointer, gsl::not_null<void*> typed_pointer) noexcept;

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
		using type = std::conditional_t<std::is_convertible_v<T*, object_base*>,
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
		visitor() = default;
		visitor(const visitor&) = default;
		visitor(visitor&&) = default;
		visitor& operator=(const visitor&) = default;
		visitor& operator=(visitor&&) = default;

		virtual void trace(const raw_reference* ref) noexcept = 0;
		virtual void trace(const object_base*) noexcept {
		}

		void trace(const raw_reference& ref) noexcept {
			trace(&ref);
		}

		virtual ~visitor() = default;
	};

	struct arena
	{
		static constexpr size_t page_size = 1 << 12;
		size_t size;
		size_t page_count;

		void* section;
		std::byte* base;
		std::byte* base_shadow;
		std::byte* rw_base;
		std::byte* rw_base_shadow;
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

		struct allocation_header {
			std::atomic<size_t> allocated_size;
			ptrdiff_t object_offset;
		};

		[[nodiscard]] void* allocate(size_t amount) noexcept;
		void deallocate(void* region);

		size_t approximately_available() const noexcept {
			return size - (high_watermark.load(std::memory_order_relaxed) - low_watermark.load(std::memory_order_relaxed));
		}

		[[nodiscard]] gc_bits get_new_location(const gc_bits old_location) const noexcept {
			// TODO
			return old_location;
		}

		[[nodiscard]] allocation_header* get_header(gsl::not_null<void*> const block) const noexcept {
			allocation_header* const header = reinterpret_cast<allocation_header*>(static_cast<std::byte*>(block.get()) - sizeof(allocation_header));
			return header;
		}

		[[nodiscard]] size_t get_block_size(gsl::not_null<void*> const block) const noexcept {
			const allocation_header* const header = get_header(block);
			return header->allocated_size.load(std::memory_order_acquire);
		}

		[[nodiscard]] void* offset_to_pointer(size_t offset) const noexcept {
			return base + offset;
		}

		[[nodiscard]] size_t pointer_to_offset(gsl::not_null<const void*> const location) const noexcept {
			return gsl::narrow_cast<std::size_t>(static_cast<const std::byte*>(location.get()) - base);
		}

		[[nodiscard]] void* bits_to_pointer(const gc_bits location) const noexcept {
			return offset_to_pointer(bits_to_offset(location));
		}
	
		[[nodiscard]] gc_bits pointer_to_bits(gsl::not_null<const void*> const location) const noexcept {
			const size_t offset = pointer_to_offset(location);
			return offset_to_bits(pointer_to_offset(location));
		}

		[[nodiscard]] size_t bits_to_offset(const gc_bits location) const noexcept {
			const size_t offset = (location.address.page_number * page_size) + location.address.page_offset;
			return offset;
		}

		[[nodiscard]] gc_bits offset_to_bits(size_t offset) const noexcept {
			gc_bits value = { 0 };
			value.address.page_number = offset / page_size;
			value.address.page_offset = offset % page_size;
			value.address.valid = 1;
			return value;
		}
	};

	struct bit_table
	{
		using element = uint8_t;
		static constexpr size_t element_bits = sizeof(element) * CHAR_BIT;

		bit_table(size_t size_) : size((size_ / arena::maximum_alignment) / element_bits), bits(std::make_unique<std::atomic<element>[]>(size)) {
			std::memset(bits.get(), 0, size * sizeof(element));
		}

		bool set_bit(size_t offset) noexcept {
			const std::pair<size_t, uint8_t> position = offset_to_bit_position(offset);
			const element mask = gsl::narrow_cast<element>(1u << position.second);
			const element orig = bits[position.first].fetch_or(mask, std::memory_order_release);
			return 1u == gsl::narrow_cast<element>((orig & mask) >> position.second);
		}

		bool clear_bit(size_t offset) noexcept {
			const std::pair<size_t, uint8_t> position = offset_to_bit_position(offset);
			const element mask = gsl::narrow_cast<element>(1u << position.second);
			const element orig = bits[position.first].fetch_and(gsl::narrow_cast<element>(~mask), std::memory_order_release);
			return 1u == gsl::narrow_cast<element>((orig & mask) >> position.second);
		}

		[[nodiscard]] bool query_bit(size_t offset) const noexcept {
			const std::pair<size_t, uint8_t> position = offset_to_bit_position(offset);
			return 1u == ((bits[position.first].load(std::memory_order_relaxed) >> position.second) & 1u);
		}

		void reset() {
			bits = std::make_unique<std::atomic<element>[]>(size);
			std::memset(bits.get(), 0, size * sizeof(std::atomic<element>));
		}

		[[nodiscard]] static std::pair<size_t, uint8_t> offset_to_bit_position(size_t offset) noexcept {
			const size_t block_number = offset / arena::maximum_alignment;
			const size_t byte_number = block_number / element_bits;
			const uint8_t bit_number = block_number % element_bits;
			return { byte_number, bit_number };
		}

		[[nodiscard]] constexpr static size_t bit_position_to_offset(size_t byte_number, size_t bit_number) noexcept {
			const size_t block_number = (byte_number * element_bits) + bit_number;
			return block_number * arena::maximum_alignment;
		}

		void visualize() const noexcept;

		size_t size;
		std::unique_ptr<std::atomic<element /*std::byte*/>[]> bits;
	};

#pragma warning(push)
#pragma warning(disable: 26439) // warning C26439: This kind of function may not throw. Declare it 'noexcept' (f.6: http://go.microsoft.com/fwlink/?linkid=853927).
	struct reference_registry
	{
		reference_registry() = default;
		reference_registry(const reference_registry&) = delete;
		reference_registry(reference_registry&&) = delete;
		reference_registry& operator=(const reference_registry&) = delete;
		reference_registry& operator=(reference_registry&&) = delete;

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

		[[nodiscard]] virtual std::vector<registration> get_snapshot() {
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

	struct thread_data : reference_registry {
		thread_data();
		thread_data(const thread_data&) = delete;
		thread_data(thread_data&&) = delete;
		thread_data& operator=(const thread_data&) = delete;
		thread_data& operator=(thread_data&&) = delete;
		~thread_data();

		// create one of these whenever we might have unprotected references on the stack
		struct [[nodiscard]] guard_unsafe {
			guard_unsafe(thread_data* td_) noexcept : td(td_) {
				td->enter_unsafe();
			}

			guard_unsafe(guard_unsafe&& rhs) noexcept : td(std::exchange(rhs.td, nullptr)) {
				rhs.td = nullptr;
			}

			guard_unsafe() = delete;
			guard_unsafe(const guard_unsafe&) = delete;
			guard_unsafe& operator=(const guard_unsafe&) = delete;
			guard_unsafe& operator=(guard_unsafe&& rhs) = delete;

			~guard_unsafe() noexcept {
				if(td) {
					td->leave_unsafe();
				}
			}

		private:
			thread_data* td;
		};

		guard_unsafe make_guard() noexcept {
			return guard_unsafe(this);
		}

		[[nodiscard]] virtual std::vector<registration> get_snapshot() {
			std::lock_guard<std::recursive_mutex> guard(unsafe);
			std::vector<registration> snapshot = reference_registry::get_snapshot();
			for(registration& reg : snapshot) {
				std::get<0>(reg) = nullptr;
			}
			return snapshot;
		}

	private:
		void enter_unsafe() {
			unsafe.lock();
		}

		void leave_unsafe() {
			unsafe.unlock();
		}

		std::recursive_mutex unsafe;
	};

	extern thread_local thread_data this_thread_data;

	struct global_data : reference_registry
	{
	};
#pragma warning(pop)

	struct collector
	{
		collector(size_t size_ = 1ui64 << 30) noexcept : the_arena(size_),
		                                                 current_marker_phase(0ui64),
		                                                 allocateds(size_),
		                                                 reachables(size_),
		                                                 usage(size_),
		                                                 readonly(size_),
		                                                 page_allocations(std::make_unique<std::atomic<uint16_t>[]>(the_arena.page_count))
		{
			master_thread = std::thread([] {
				return;
			});
		}

		collector(const collector&) = delete;
		collector(collector&&) = delete;
		collector& operator=(const collector&) = delete;
		collector& operator=(collector&&) = delete;

		~collector() {
			master_thread.join();
		}

		template<typename T, typename... Args>
		friend std::enable_if_t<!is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(std::initializer_list<typename normalize_t<T>::element_type> init);

		void collect();

	protected:
		friend struct raw_reference;
		template<typename T>
		friend struct handle_t;
		template<typename T>
		friend struct global_t;
		friend struct marker;
		friend struct thread_data;
		friend union gc_bits;

		void queue_object(object_base*);

		void mark();

		void update_usage(const void* const addr, size_t object_size);

		void relocate(size_t bottom, size_t top);

		void remap(size_t bottom, size_t top);

		void finalize(size_t bottom, size_t top);

		void shrink(size_t bottom, size_t top);

		arena the_arena;

		std::atomic<size_t> current_marker_phase;

	protected:
		void register_mutator_thread(thread_data* data) {
			std::scoped_lock<std::mutex> rw_lock(thread_lock);
			mutator_threads[std::this_thread::get_id()] = data;
		}

		void unregister_mutator_thread() {
			std::scoped_lock<std::mutex> rw_lock(thread_lock);
			mutator_threads.erase(std::this_thread::get_id());
		}

	protected:
		// TODO move this to the collector class
		[[nodiscard]] bool is_protected(const gc_bits location) const noexcept {
			return readonly.query_bit(the_arena.bits_to_offset(location));
		}


		[[nodiscard]] bool is_marked_reachable(gsl::not_null<const void*> const address) const noexcept {
			return reachables.query_bit(the_arena.pointer_to_offset(address));
		}

		[[nodiscard]] bool is_marked_allocated(gsl::not_null<const void*> const address) const noexcept {
			return allocateds.query_bit(the_arena.pointer_to_offset(address));
		}

		void mark_reachable(gsl::not_null<void*> const address) {
			const arena::allocation_header* const header = the_arena.get_header(address);
			size_t object_size = header->allocated_size;
			update_usage(address, object_size);
			size_t offset = the_arena.pointer_to_offset(address);
			reachables.set_bit(offset);
			usage.set_bit(offset);
			while(object_size > arena::maximum_alignment) {
				offset += arena::maximum_alignment;
				usage.set_bit(offset);
				object_size -= arena::maximum_alignment;
			}
		}

		void mark_allocated(gsl::not_null<const void*> const address) noexcept {
			allocateds.set_bit(the_arena.pointer_to_offset(address));
		}

		bool mark_destroyed(gsl::not_null<const void*> const address) noexcept {
			return allocateds.clear_bit(the_arena.pointer_to_offset(address));
		}

	private:
		friend struct object_base;
		friend struct marker;

		using registration = std::tuple<raw_reference*, object_base*>;
		using root_set = std::vector<registration>;
		using mark_queue = std::unordered_set<object_base*>;

		void prepare();
		void flip() noexcept;

		root_set scan_roots();

		std::mutex thread_lock;
		std::unordered_map<std::thread::id, thread_data*> mutator_threads;
		global_data globals;
		
		std::mutex mutated_lock;
		mark_queue mutated_objects;

		bit_table allocateds;
		bit_table reachables;
		bit_table usage;
		bit_table readonly;
		std::unique_ptr<std::atomic<uint16_t>[]> page_allocations;

		std::thread master_thread;
	};

	extern collector the_gc;

	inline thread_data::thread_data() {
		the_gc.register_mutator_thread(this);
	}

	inline thread_data::~thread_data() {
		the_gc.unregister_mutator_thread();
	}

	struct marker : visitor
	{
		marker(collector* the_gc_) noexcept : the_gc(the_gc_) {
		}

		virtual void trace(const raw_reference* ref) noexcept override;
		virtual void trace(const object_base* obj) noexcept override;
		using visitor::trace;

	private:
		collector* the_gc;
	};

	struct nuller : visitor
	{
		virtual void trace(const raw_reference* ref) noexcept override;
		using visitor::trace;
	};

	struct object_base
	{
		object_base() noexcept {
			gc_bits bits = { 0 };
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
		static void* operator new  (size_t, void* ptr) noexcept { return ptr; }
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) noexcept { return ptr; }

		virtual void _gc_trace(gsl::not_null<visitor*>) const noexcept = 0;

		static void guarded_destruct(gsl::not_null<const object_base*> obj) noexcept {
			if(the_gc.mark_destroyed(obj->_gc_get_block())) {
				nuller n;
				obj->_gc_trace(&n);
				obj->~object_base();
				return;
			}
		}

	protected:
		void _gc_set_block_unsafe(gsl::not_null<void*> addr, gsl::not_null<const object_base*> obj) noexcept {
			const gc_bits location = the_gc.the_arena.pointer_to_bits(addr);
			gc_bits current = count.load(std::memory_order_acquire);
			current.header.raw_address = location.header.raw_address;
			count.store(current, std::memory_order_release);

			the_gc.the_arena.get_header(addr)->object_offset = reinterpret_cast<const std::byte*>(obj.get()) - static_cast<std::byte*>(addr.get());
		}

		void* _gc_get_block() const noexcept {
			const gc_bits location = count.load(std::memory_order_acquire);
			return the_gc.the_arena.bits_to_pointer(location);
		}

	private:
		mutable gc_pointer count;

		friend struct collector;
		friend struct marker;
		friend struct raw_reference;
		template<typename To, typename From>
		friend handle_t<normalize_t<To>> gc_cast(const reference_t<From>& rhs) noexcept;
		template<typename T, typename... Args>
		friend std::enable_if_t<!is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(std::initializer_list<typename normalize_t<T>::element_type> init);

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
						object_base::guarded_destruct(this);
					}
					return new_value.header.reference_count;
				}
			}
		}
	};

	struct object : virtual object_base
	{
		virtual void _gc_trace(gsl::not_null<visitor*>) const noexcept override {
		}
	};

	template<typename T>
	struct
	alignas(std::conditional_t<std::is_convertible_v<T*, object_base*>, reference_t<T>, T>)
	alignas(object)
	array final : object
	{
		friend struct collector;

		using element_type = std::conditional_t<std::is_convertible_v<T*, object_base*>, reference_t<T>, T>;

		array() = delete;
		array(const array&) = delete;

		explicit array(size_t length_) noexcept(std::is_nothrow_constructible_v<element_type>) : len(length_) {
			std::uninitialized_value_construct_n(elements(), len);
		}
	
		explicit array(std::initializer_list<element_type> init) noexcept(std::is_nothrow_copy_constructible_v<element_type>) : len(init.size()) {
			std::uninitialized_copy(init.begin(), init.end(), elements());
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

		virtual void _gc_trace(gsl::not_null<visitor*> v) const noexcept override {
			if constexpr(std::is_convertible_v<T*, object_base*>) {
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
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(std::initializer_list<typename normalize_t<T>::element_type> init);

		constexpr static size_t allocation_size(size_t len) noexcept {
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
	struct box final : object
	{
		using element_type = std::conditional_t<std::is_convertible_v<T*, object_base*>, reference_t<T>, T>;

		box(const element_type& e) noexcept(std::is_nothrow_copy_constructible_v<element_type>) : val(e) {
		}

		virtual void _gc_trace(gsl::not_null<visitor*> v) const noexcept override {
			if constexpr(std::is_convertible_v<T*, object_base*>) {
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
			return pointer.load().address.valid;
		}

		bool operator==(const raw_reference& rhs) const noexcept {
			return pointer.load().raw.pointer == rhs.pointer.load().raw.pointer ||
			       _gc_resolve_base() == rhs._gc_resolve_base();
		}

		bool operator!=(const raw_reference& rhs) const noexcept {
			return !(*this == rhs);
		}

		raw_reference& operator=(std::nullptr_t) noexcept {
			clear();
			return *this;
		}

		virtual ~raw_reference() noexcept {
			clear();
		}

	protected:
		raw_reference() noexcept : pointer() {
		}

		raw_reference(std::nullptr_t) noexcept : pointer() {
		}

		raw_reference(const raw_reference& rhs) noexcept {
			assign(rhs);
		}

		raw_reference(raw_reference&& rhs) noexcept {
			assign(rhs);
		}

		raw_reference& operator=(const raw_reference& rhs) noexcept {
			if(this != &rhs) {
				clear();
				assign(rhs);
			}
			return *this;
		}

		raw_reference& operator=(raw_reference&& rhs) noexcept {
			if(this != &rhs) {
				clear();
				assign(rhs);
			}
			return *this;
		}

		virtual void assign(const raw_reference& rhs) noexcept {
			if(rhs.pointer.load(std::memory_order_acquire).address.valid) {
				const object_base* const obj = rhs._gc_resolve_base();
				obj->_gc_add_ref();
			}
			_gc_write_barrier();
			pointer.store(rhs.pointer.load(std::memory_order_acquire));
		}

		virtual void assign(raw_reference&& rhs) noexcept {
			_gc_write_barrier();
			pointer.store(rhs.pointer.load(std::memory_order_acquire), std::memory_order_release);
			rhs.pointer.store(gc_bits::zero, std::memory_order_release);
		}

		virtual void clear() noexcept {
			if(pointer.load(std::memory_order_acquire).address.valid) {
				const object_base* const obj = _gc_resolve_base_no_queue();
				obj->_gc_del_ref();
			}
			pointer.store(gc_bits::zero, std::memory_order_release);
		}

		// use this exclusively when a reference is being destroyed
		object_base* _gc_resolve_base_no_queue() const noexcept {
			const gc_bits value = _gc_read_barrier<false>();
			object_base* ob = static_cast<object_base*>(the_gc.the_arena.bits_to_pointer(value));
			return ob;
		}

		object_base* _gc_resolve_base() const noexcept {
			if(!pointer.load(std::memory_order_acquire).address.valid) {
				return nullptr;
			}
			const gc_bits value = _gc_read_barrier();
			object_base* ob = static_cast<object_base*>(the_gc.the_arena.bits_to_pointer(value));
			return ob;
		}

		void* _gc_resolve() const noexcept {
			if(!pointer.load(std::memory_order_acquire).address.valid) {
				return nullptr;
			}
			const gc_bits value = _gc_read_barrier();
			object_base* ob = static_cast<object_base*>(the_gc.the_arena.bits_to_pointer(value));
			std::byte* derived_object = reinterpret_cast<std::byte*>(ob) + (value.address.typed_offset * gsl::narrow_cast<ptrdiff_t>(sizeof(void*)));
			return derived_object;
		}

		void _gc_write_barrier() const noexcept {
			// awkward. if my retaining object is oldgen, and the new object is newgen, I need to treat that specially.
			// but I don't know what my retaining object is.
			// maybe I can judge based on the location of 'this'
		}

		template<bool check_marker = true>
		[[gsl::suppress(con.4)]]
		gc_bits _gc_read_barrier() const noexcept {
			gc_bits value = pointer.load();
			bool marker_phase_changed = false;
			if constexpr(check_marker) {
				if(value.address.seen_by_marker != the_gc.current_marker_phase) {
					marker_phase_changed = true;
				}
			}
			bool relocated = false;
			if(the_gc.is_protected(value)) {
				relocated = true;
			}
			if(marker_phase_changed || relocated) {
				value = _gc_load_barrier_fix(value, marker_phase_changed, relocated);
			}
			return value;
		}

		gc_bits _gc_load_barrier_fix(gc_bits old_value, bool marker_phase_changed, bool relocated) const noexcept {
			object_base* ob = static_cast<object_base*>(the_gc.the_arena.bits_to_pointer(old_value));
			if(marker_phase_changed) {
				the_gc.queue_object(ob);
			}
			for(;;) {
				gc_bits new_value = old_value;
				if(marker_phase_changed) {
					new_value.address.seen_by_marker = the_gc.current_marker_phase;
				}
				if(relocated) {
					gc_bits object_header = ob->count.load(std::memory_order_acquire);

					while(object_header.header.relocate != gc_bits::moved) {
						std::this_thread::yield();
						object_header = ob->count.load(std::memory_order_acquire);
					}
					// forwarded address stored in the corpse of the old object
					new_value.address.page_number = object_header.address.page_number;
					new_value.address.page_offset = object_header.address.page_offset;
				}
				if(pointer.compare_exchange_strong(old_value, new_value, std::memory_order_release)) {
					return new_value;
				}
			}
		}

	private:
		friend struct marker;

		template<typename T>
		friend struct reference_t;

		raw_reference(gc_bits bits) noexcept : pointer(bits) {
		}

		mutable gc_pointer pointer;
	};

	// a typed GCed reference
	template<typename T>
	struct reference_t : raw_reference
	{
		static_assert(std::is_convertible_v<T*, object_base*>, "T must be derived from object");

		using referenced_type = T;

		using my_type   = reference_t<T>;
		using base_type = raw_reference;

		reference_t() noexcept : base_type() {
		}

		~reference_t() noexcept = default;

		reference_t(std::nullptr_t) noexcept : base_type(nullptr) {
		}

		reference_t(const my_type& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
		}

		reference_t(my_type&& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
		}

		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*> || (is_array_v<T> && std::is_same_v<T, U>) > >
		reference_t(const reference_t<U>& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
			fix_reference(rhs);
		}

		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*> || (is_array_v<T> && std::is_same_v<T, U>) > >
		reference_t(reference_t<U>&& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
			fix_reference(rhs);
		}

		my_type& operator=(const my_type& rhs) noexcept {
			base_type::operator=(rhs);
			return *this;
		}

		my_type& operator=(my_type&& rhs) noexcept {
			base_type::operator=(rhs);
			return *this;
		}

		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*> || (is_array_v<T> && std::is_same_v<T, U>) > >
		my_type& operator=(const reference_t<U>& rhs) noexcept {
			base_type::operator=(rhs);
			fix_reference(rhs);
			return *this;
		}

		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*> || (is_array_v<T> && std::is_same_v<T, U>) > >
		my_type& operator=(reference_t<U>&& rhs) noexcept {
			base_type::operator=(rhs);
			fix_reference(rhs);
			return *this;
		}

		my_type& operator=(std::nullptr_t) noexcept {
			base_type::operator=(nullptr);
			return *this;
		}

		template<typename U = T>
		std::enable_if_t<!is_array_v<U>, referenced_type*>
		operator->() const noexcept {
			return _gc_resolve();
		}

		template<typename U = T>
		std::enable_if_t<is_array_v<U>, typename U::element_type&>
		operator[](size_t offset) const noexcept {
			if(!*this) {
				std::terminate();
			}
			return _gc_resolve()->operator[](offset);
		}

	protected:
		referenced_type* _gc_resolve() const noexcept {
			return static_cast<referenced_type*>(raw_reference::_gc_resolve());
		}

		template<typename U>
		void fix_reference(const reference_t<U>& ref) noexcept {
			U* source = ref._gc_resolve();
			T* destination = dynamic_cast<T*>(source);
			const object_base* ob = source;
			const ptrdiff_t typed_offset = (reinterpret_cast<std::byte*>(destination) - reinterpret_cast<const std::byte*>(ob)) / gsl::narrow_cast<ptrdiff_t>(sizeof(void*));
			gc_bits current = pointer.load(std::memory_order_acquire);
			current.address.typed_offset = typed_offset;
			pointer.store(current, std::memory_order_release);
		}

	private:
		template<typename T>
		friend struct reference_t;

		friend struct collector;

		template<typename T>
		friend struct array;

		template<typename To, typename From>
		friend handle_t<normalize_t<To>> gc_cast(const reference_t<From>& rhs) noexcept;
		template<typename T, typename... Args>
		friend std::enable_if_t<!is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(Args&&... args);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(size_t size);
		template<typename T>
		friend std::enable_if_t< is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> gcnew(std::initializer_list<typename normalize_t<T>::element_type> init);

		template<typename T>
		static reference_t<T> make_reference(gsl::not_null<object_base*> obj, gsl::not_null<T*> typed) noexcept {
			return reference_t<T>(gc_bits::_gc_build_reference(obj, typed));
		}

		reference_t(gc_bits bits) noexcept : raw_reference(bits) {
		}
	};

	template<typename T, typename Self>
	struct registered_reference_t : reference_t<T>
	{
		using my_type = registered_reference_t<T, Self>;
		using base_type = reference_t<T>;

		registered_reference_t() noexcept : base_type() {
		}

		registered_reference_t(std::nullptr_t) noexcept : base_type(nullptr) {
		}

		registered_reference_t(const my_type& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
		}

		registered_reference_t(my_type&& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
		}

		my_type& operator=(const my_type& rhs) {
			base_type::operator=(rhs);
			return *this;
		}

		my_type& operator=(my_type&& rhs) {
			base_type::operator=(rhs);
			return *this;
		}

		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*> || (is_array_v<T> && std::is_same_v<T, U>) > >
		registered_reference_t(const reference_t<U>& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
			this->fix_reference(rhs);
		}

		template<typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*> || (is_array_v<T> && std::is_same_v<T, U>) > >
		registered_reference_t(reference_t<U>&& rhs) noexcept : base_type(nullptr) {
			assign(rhs);
			this->fix_reference(rhs);
		}

		virtual ~registered_reference_t() noexcept override {
			clear();
		}

		using base_type::operator=;

	protected:
		virtual void assign(const raw_reference& rhs) noexcept override {
			raw_reference::assign(rhs);
			if(*this) {
				dynamic_cast<Self*>(this)->get_registrar()->register_reference(this, this->_gc_resolve());
			}
		}

		virtual void assign(raw_reference&& rhs) noexcept override {
			raw_reference::assign(rhs);
			if(*this) {
				dynamic_cast<Self*>(this)->get_registrar()->unregister_reference(&rhs);
				dynamic_cast<Self*>(this)->get_registrar()->register_reference(this, this->_gc_resolve());
			}
		}

		virtual void clear() noexcept override {
			if(*this) {
				dynamic_cast<Self*>(this)->get_registrar()->unregister_reference(this);
			}
			raw_reference::clear();
		}
	};

	// a typed GCed reference that registers itself as a field (no registration)
	template<typename T>
	struct member_t final : reference_t<T>
	{
		using base_type = reference_t<T>;
		using my_type   = member_t<T>;

		using base_type::base_type;
		using base_type::operator=;
	};

	// a typed GCed reference that registers itself as a global
	template<typename T>
	struct global_t final : registered_reference_t<T, global_t<T> >
	{
		using base_type = registered_reference_t<T, global_t<T> >;
		using my_type   = global_t<T>;

		using base_type::base_type;
		using base_type::operator=;

		reference_registry* get_registrar() const noexcept {
			return &this->the_gc.globals;
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) = delete;
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) = delete;
	};

	// a typed GCed reference that registers itself as a stack variable
	template<typename T>
	struct handle_t final : registered_reference_t<T, handle_t<T> >
	{
		using base_type = registered_reference_t<T, handle_t<T> >;
		using my_type   = handle_t<T>;

		using base_type::base_type;
		using base_type::operator=;

		reference_registry* get_registrar() const noexcept {
			return &this_thread_data;
		}

		static void* operator new  (size_t) = delete;
		static void* operator new  (size_t, void* ptr) = delete;
		static void* operator new[](size_t) = delete;
		static void* operator new[](size_t, void* ptr) = delete;
	};

	template<typename To, typename From>
	[[nodiscard]] handle_t<normalize_t<To>> inline gc_cast(const reference_t<From>& rhs) noexcept {
		using real_type = normalize_t<To>;
		if(!rhs) {
			return handle_t<real_type>();
		}

		From* object = rhs._gc_resolve();
		real_type* destination = dynamic_cast<real_type*>(object);
		if(destination) {
			gsl::not_null<object_base*> base = destination;
			base->_gc_add_ref();
			return handle_t<real_type>(reference_t<real_type>::make_reference(base, gsl::not_null<real_type*>(destination)));
		} else {
			return handle_t<real_type>();
		}
	}

#pragma warning(push)

#pragma warning(disable: 26423) // warning C26423: The allocation was not directly assigned to an owner.
#pragma warning(disable: 26424) // warning C264224: Failing to delete or assign ownership of allocation at line %d.

	template<typename T, typename... Args>
	[[gsl::suppress(i.11)]]
	[[gsl::suppress(r.11)]]
	[[gsl::suppress(r.12)]]
	[[nodiscard]]
	std::enable_if_t<!is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> inline gcnew(Args&&... args) {
		thread_data::guard_unsafe g(this_thread_data.make_guard());
		using real_type = normalize_t<T>;

		void* memory = the_gc.the_arena.allocate(sizeof(real_type));
		if(!memory) {
			throw std::bad_alloc();
		}

		gsl::not_null<real_type*> typed = new(memory) real_type(std::forward<Args>(args)...);
		the_gc.mark_allocated(memory);
		gsl::not_null<object_base*> base = typed;
		base->_gc_set_block_unsafe(memory, base);
		return handle_t<real_type>(reference_t<real_type>::make_reference(base, typed));
	}

	template<typename T>
	[[gsl::suppress(i.11)]]
	[[gsl::suppress(r.11)]]
	[[gsl::suppress(r.12)]]
	[[nodiscard]]
	std::enable_if_t<is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> inline gcnew(size_t len) {
		thread_data::guard_unsafe g(this_thread_data.make_guard());
		using real_type = normalize_t<T>;

		void* memory = the_gc.the_arena.allocate(real_type::allocation_size(len));
		if(!memory) {
			throw std::bad_alloc();
		}

		gsl::not_null<real_type*> typed = new(memory) real_type(len);
		the_gc.mark_allocated(memory);
		gsl::not_null<object_base*> base = typed;
		base->_gc_set_block_unsafe(memory, base);
		return handle_t<real_type>(reference_t<real_type>::make_reference(base, typed));
	}

	template<typename T>
	[[gsl::suppress(i.11)]]
	[[gsl::suppress(r.11)]]
	[[gsl::suppress(r.12)]]
	[[nodiscard]]
	std::enable_if_t<is_array_v<normalize_t<T>>, handle_t<normalize_t<T>>> inline gcnew(std::initializer_list<typename normalize_t<T>::element_type> init) {
		thread_data::guard_unsafe g(this_thread_data.make_guard());
		using real_type = normalize_t<T>;

		void* memory = the_gc.the_arena.allocate(real_type::allocation_size(init.size()));
		if(!memory) {
			throw std::bad_alloc();
		}

		gsl::not_null<real_type*> typed = new(memory) real_type(init);
		the_gc.mark_allocated(memory);
		gsl::not_null<object_base*> base = typed;
		base->_gc_set_block_unsafe(memory, base);
		return handle_t<real_type>(reference_t<real_type>::make_reference(base, typed));
	}

	template<typename T>
	[[nodiscard]]
	std::enable_if_t<is_array_v<normalize_t<T>> && std::is_array_v<T> && std::extent_v<T> != 0, handle_t<normalize_t<T>>> inline gcnew() {
		return gcnew<T>(std::extent_v<T>);
	}

#pragma warning(pop)

	template<typename T>
	using reference = reference_t<normalize_t<T>>;

	template<typename T>
	using handle = handle_t<normalize_t<T>>;

	template<typename T>
	using member = member_t<normalize_t<T>>;

	template<typename T>
	using global = global_t<normalize_t<T>>;
}
