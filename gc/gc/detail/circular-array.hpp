#pragma once

#include <memory>
#include <atomic>
#include <type_traits>

namespace garbage_collection
{
	template<typename T>
	struct circular_array;

	namespace
	{
		template<typename T>
		struct circular_array_destroyer {
			void operator()(circular_array<T>* ptr);
		};

		template<typename T>
		struct circular_array_data {
			circular_array_data(size_t log_size_, circular_array<T>* previous_) : log_size(log_size_), previous(previous_, circular_array_destroyer<T>{}) {
			}

			size_t size() const {
				return 1ull << log_size;
			}

			size_t lg_size() const {
				return log_size;
			}

			size_t mask() const {
				return size() - 1;
			}

			void delink() {
				previous.release();
			}

		protected:
			size_t log_size;
			using chunk_ptr = std::unique_ptr<circular_array<T>, circular_array_destroyer<T>>;
			chunk_ptr previous;
		};
	}

	template<typename T>
	struct
	alignas(circular_array_data<T>) // this "spurious" alignment (derived classes are always suitably aligned for their bases) ensures that there's no warning...
	alignas(std::atomic<T>)         // when alignof(atomic<T>) < alignof(circular_array_data<T>). Underspecified alignment is otherwise undefined (!).
	circular_array : circular_array_data<T> {
		static_assert(std::is_default_constructible_v<T>, "T must be DefaultConstructible");
		static_assert(std::is_nothrow_constructible_v<T>, "T must be NothrowConstructible");
		static_assert(std::is_trivially_copyable_v<T>,    "T must be TriviallyCopyable");

		using atomic_value = std::atomic<T>;

		using circular_array_data<T>::size;
		using circular_array_data<T>::mask;

		circular_array(size_t log_size_, circular_array<T>* previous_) : circular_array_data<T>(log_size_, previous_) {
			atomic_value* elts = elements();
			size_t sz = size();
			for(size_t i = 0; i < sz; ++i) {
				new (&elts[i]) atomic_value();
			}
		}

		circular_array(const circular_array&) = delete;

		~circular_array() = default;

		void put(size_t i, const T& v, std::memory_order order = std::memory_order_relaxed) {
			elements()[i & mask()].store(v, order);
		}

		T get(size_t i, std::memory_order order = std::memory_order_relaxed) {
			return elements()[i & mask()].load(order);
		}

		constexpr static size_t get_allocation_size(size_t log_size) {
			return sizeof(circular_array<T>) + ((1ull << log_size) * sizeof(atomic_value));
		}

		static circular_array<T>* make_circular_array(size_t log_size, circular_array<T>* previous) {
			const size_t allocation_size = get_allocation_size(log_size);
			void* raw_memory = ::operator new(allocation_size);
			// this generates a spurious warning C6386: Buffer overrun while writing to 'raw_memory':  the writable size is 'allocation_size' bytes, but '16' bytes might be written.
			// the assumption suppresses the warning. I don't know why it can't see that this is true.
			__analysis_assume(allocation_size >= sizeof(circular_array<T>));
			return new(raw_memory) circular_array<T>(log_size, previous);
		}

		static circular_array<T>* make_circular_array(size_t log_size) {
			return make_circular_array(log_size, nullptr);
		}

		static void destroy_circular_array(circular_array<T>* c) {
			::operator delete(c);
		}

		atomic_value* elements() {
			return reinterpret_cast<atomic_value*>(this + 1);
		}
	};

	namespace {
		template<typename T>
		inline void circular_array_destroyer<T>::operator()(circular_array<T>* ptr) {
			circular_array<T>::destroy_circular_array(ptr);
		};
	}
}
