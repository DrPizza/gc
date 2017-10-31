#pragma once

#include <cstdint>
#include <atomic>
#include <variant>
#include <optional>
#include <memory>
#include <utility>
#include <tuple>

#include "circular-array.hpp"

// queue from http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.170.1097&rep=rep1&type=pdf
// atomics taken from http://www.di.ens.fr/~zappa/readings/ppopp13.pdf

namespace garbage_collection
{
	struct empty_t {};
	struct abort_t {};

	template<typename T>
	struct work_stealing_queue {
		work_stealing_queue() : top(0ui64), bottom(0ui64), arr(array_type::make_circular_array(5)) {
		}

		~work_stealing_queue() {
			array_type::destroy_circular_array(arr.load(std::memory_order_relaxed));
		}

		void push(const T& elem) {
			uint64_t b = bottom.load(std::memory_order_relaxed);
			uint64_t t = top.load(std::memory_order_acquire);
			array_type* a = arr.load(std::memory_order_relaxed);
			if(b - t > a->size() - 1) {
				a = grow(a, t, b);
				arr.store(a, std::memory_order_relaxed);
			}
			a->put(b, elem);
			std::atomic_thread_fence(std::memory_order_release);
			bottom.store(b + 1, std::memory_order_relaxed);
		}

		std::optional<T> pop() {
			uint64_t b = bottom.load(std::memory_order_relaxed) - 1;
			array_type* a = arr.load(std::memory_order_relaxed);
			bottom.store(b, std::memory_order_relaxed);
			std::atomic_thread_fence(std::memory_order_seq_cst);
			uint64_t t = top.load(std::memory_order_relaxed);
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

		using stolen_value_t = std::variant<empty_t, abort_t, T>;

		stolen_value_t steal() {
			uint64_t t = top.load(std::memory_order_acquire);
			std::atomic_thread_fence(std::memory_order_seq_cst);
			uint64_t b = bottom.load(std::memory_order_acquire);
			if(t < b) {
				array_type* a = arr.load(std::memory_order_acquire);
				T x = a->get(t);
				if(!top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
					return abort_t{};
				}
				return x;
			}
			return empty_t{};
		}

	private:
		using array_type = circular_array<T>;

		static array_type* grow(array_type* orig, uint64_t first, uint64_t last) {
			circular_array<T>* a = array_type::make_circular_array(orig->lg_size() + 1, orig);
			for(uint64_t i = first; i != last; ++i) {
				a->put(i, orig->get(i));
			}
			return a;
		}

		std::atomic<uint64_t> top;
		std::atomic<uint64_t> bottom;

		std::atomic<array_type*> arr;
	};
}
