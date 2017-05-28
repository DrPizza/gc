#pragma once

#include <atomic>
#include <algorithm>
#include <optional>
#include <memory>
#include <type_traits>
#include <cassert>

// queue from http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.170.1097&rep=rep1&type=pdf
// atomics taken from http://www.di.ens.fr/~zappa/readings/ppopp13.pdf

#pragma warning(push)
#pragma warning(disable: 4789)

template<typename T>
struct work_stealing_queue {
	static_assert(std::is_default_constructible_v<T>, "T must be DefaultConstructible");
	static_assert(std::is_trivially_copyable_v<T>, "T must be TriviallyCopyable");

	work_stealing_queue() : top(0), bottom(0), arr(circular_array::make_circular_array(5)) {
	}

	~work_stealing_queue() {
		circular_array::destroy_circular_array(arr.load(std::memory_order_relaxed));
	}

	void push(const T& elem) {
		size_t b = bottom.load(std::memory_order_relaxed);
		size_t t = top.load(std::memory_order_acquire);
		circular_array* a = arr.load(std::memory_order_relaxed);
		if(b - t > a->size() - 1) {
			a = a->grow(t, b);
			arr.store(a, std::memory_order_relaxed);
		}
		a->put(b, elem);
		std::atomic_thread_fence(std::memory_order_release);
		bottom.store(b + 1, std::memory_order_relaxed);
	}

	std::optional<T> pop() {
		size_t b = bottom.load(std::memory_order_relaxed) - 1;
		circular_array* a = arr.load(std::memory_order_relaxed);
		bottom.store(b, std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_seq_cst);
		size_t t = top.load(std::memory_order_relaxed);
		if(t <= b) {
			std::optional<T> x = a->get(b);
			if(t == b) {
				if(!top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
					x = std::nullopt;
				}
				bottom.store(b + 1, std::memory_order_relaxed);
			}
			return x;
		} else {
			bottom.store(b + 1, std::memory_order_relaxed);
			return std::nullopt;
		}
	}

	std::pair<bool, std::optional<T>> steal() {
		size_t t = top.load(std::memory_order_acquire);
		std::atomic_thread_fence(std::memory_order_seq_cst);
		size_t b = bottom.load(std::memory_order_acquire);
		if(t < b) {
			circular_array* a = arr.load(std::memory_order_acquire);
			std::optional<T> x = a->get(t);
			if(!top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
				return std::make_pair(false, std::nullopt);
			}
			return std::make_pair(true, x);
		}
		return std::make_pair(true, std::nullopt);
	}

private:
	using atomic_value = std::atomic<T>;

	struct circular_array;

	struct circular_array_data {
		circular_array_data(size_t log_size_, void(*deleter)(circular_array*)) : log_size(log_size_), previous(nullptr, deleter) {
		}

		size_t log_size;
		using chunk_ptr = std::unique_ptr<circular_array, void(*)(circular_array*)>;
		chunk_ptr previous;
	};

	struct
	alignas(atomic_value)
	alignas(circular_array_data)
	circular_array : circular_array_data {
		circular_array(size_t log_size_) : circular_array_data(log_size_, &destroy_circular_array) {
			atomic_value* elts = elements();
			size_t sz = size();
			for(size_t i = 0; i < sz; ++i) {
				new (&elts[i]) atomic_value(T{});
			}
		}

		~circular_array() {
			atomic_value* elts = elements();
			size_t sz = size();
			for(size_t i = 0; i < sz; ++i) {
				elts[sz - i - 1].~atomic_value();
			}
		}

		size_t size() const {
			return 1ull << this->log_size;
		}

		void put(size_t i, const T& v) {
			elements()[i & (size() - 1)].store(v, std::memory_order_relaxed);
		}

		T get(size_t i) {
			return elements()[i & (size() - 1)].load(std::memory_order_relaxed);
		}

		circular_array* grow(size_t t, size_t b) {
			circular_array* a = make_circular_array(this->log_size + 1);
			a->previous.reset(this);
			for(size_t i = t; i != b; ++i) {
				a->put(i, get(i));
			}
			return a;
		}

		static size_t get_allocation_size(size_t log_size) {
			const size_t allocation_size = sizeof(circular_array) + ((1ull << log_size) * sizeof(atomic_value));
			return allocation_size;
		}

		static circular_array* make_circular_array(size_t log_size) {
			const size_t allocation_size = get_allocation_size(log_size);
			void* raw_memory = ::operator new(allocation_size);
			return new(raw_memory) circular_array(log_size);
		}

		static void destroy_circular_array(circular_array* c) {
			c->~circular_array();
			::operator delete(c);
		}

		atomic_value* elements() {
			assert(reinterpret_cast<atomic_value*>(this + 1) == &elements_[0]);
			return reinterpret_cast<atomic_value*>(this + 1);
			//return reinterpret_cast<atomic_value*>(reinterpret_cast<byte*>(this) + sizeof(*this));
		}

#ifdef _DEBUG
#pragma warning(suppress: 4200)
		atomic_value elements_[];
#endif
	};

	std::atomic<size_t> top;
	std::atomic<size_t> bottom;

	std::atomic<circular_array*> arr;
};

#pragma warning(pop)
