// gc-test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gc/gc.hpp"
#include "gc/gc-macros.hpp"

#include "gc/detail/ws-queue.hpp"

void* read_write;
void* read_only;
ULARGE_INTEGER segment_size;

bool is_in_ro_segment(void* address) {
	if(reinterpret_cast<size_t>(read_only) <= reinterpret_cast<size_t>(address)
	&& reinterpret_cast<size_t>(address)   <  reinterpret_cast<size_t>(read_only) + segment_size.QuadPart) {
		return true;
	}
	return false;
}

size_t get_ro_offset(void* address) {
	if(is_in_ro_segment(address)) {
		return reinterpret_cast<size_t>(address) - reinterpret_cast<size_t>(read_only);
	} else {
		return 0;
	}
}

void* ro_to_rw(void* address) {
	if(is_in_ro_segment(address)) {
		return reinterpret_cast<void*>(reinterpret_cast<size_t>(read_write) + get_ro_offset(address));
	} else {
		return nullptr;
	}
}

int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {
	if(code != 0xc0000005) { // access denied
		return EXCEPTION_EXECUTE_HANDLER;
	}
	if(ep->ExceptionRecord->ExceptionInformation[0] != 1) { // write
		return EXCEPTION_EXECUTE_HANDLER;
	}
	size_t target = ep->ExceptionRecord->ExceptionInformation[1];
	if(!is_in_ro_segment(reinterpret_cast<void*>(target))) {
		return EXCEPTION_EXECUTE_HANDLER;
	}
	// maybe disassemble ep->ExceptionRecord->ExceptionAddress to directly find the memory address

	       if(ep->ContextRecord->Rax == target) {
		ep->ContextRecord->Rax = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->Rbx == target) {
		ep->ContextRecord->Rbx = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->Rcx == target) {
		ep->ContextRecord->Rcx = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->Rdx == target) {
		ep->ContextRecord->Rdx = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R8 == target) {
		ep->ContextRecord->R8 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R9 == target) {
		ep->ContextRecord->R9 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R10 == target) {
		ep->ContextRecord->R10 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R11 == target) {
		ep->ContextRecord->R11 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R12 == target) {
		ep->ContextRecord->R12 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R13 == target) {
		ep->ContextRecord->R13 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R14 == target) {
		ep->ContextRecord->R14 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else if(ep->ContextRecord->R15 == target) {
		ep->ContextRecord->R15 = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<void*>(target)));
	} else {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}

struct my_object : gc::object {
	virtual void hello_world() const {
		std::cout << "Hello, World!" << std::endl;
	}

	virtual void _gc_trace(gc::visitor* visitor) const override {
		gc::object::_gc_trace(visitor);
	}
};

struct ambiguous_object : gc::object {
	ambiguous_object(size_t parameter) : value(parameter) {
		std::cout << "parameter: " << parameter << std::endl;
	}

	void foo() const {
		std::cout << "foo: " << value << std::endl;
	}

	virtual void _gc_trace(gc::visitor* visitor) const override {
		std::cout << __FUNCSIG__ << std::endl;
		gc::object::_gc_trace(visitor);
	}

private:
	size_t value;
};

struct left : gc::object
{
	virtual void _gc_trace(gc::visitor* visitor) const override {
		std::cout << __FUNCSIG__ << std::endl;
		gc::object::_gc_trace(visitor);
	}
};

struct right : gc::object
{
	virtual void _gc_trace(gc::visitor* visitor) const override {
		std::cout << __FUNCSIG__ << std::endl;
		gc::object::_gc_trace(visitor);
	}
};

//struct combined : left, right
//{
//	virtual void _gc_trace(gc::visitor* visitor) override {
//		std::cout << __FUNCSIG__ << std::endl;
//		left::_gc_trace(visitor);
//		right::_gc_trace(visitor);
//	}
//};

struct combined : left, right
{
	combined() : ao1(gc::gcnew<ambiguous_object>(10u)),
	             ao2(gc::gcnew<ambiguous_object>(11u))
	{
		ao1->foo();
		ao2->foo();
	}

	DEFINE_GC_MEMBERS(
		(left, right),
		(
			(public, ambiguous_object, ao1),
			(public, ambiguous_object, ao2)
		)
	)
};

struct B : gc::object {
	DEFINE_GC_MEMBERS(
		(gc::object),
		()
	)

	virtual void foo() {
		std::cout << this << " " << typeid(this).name() << std::endl;
	}
};

struct D1 : B {
	DEFINE_GC_MEMBERS(
		(B),
		()
	)

	virtual void foo() {
		std::cout << this << " " << typeid(this).name() << std::endl;
	}
};

struct D2 : B {
	DEFINE_GC_MEMBERS(
		(B),
		()
	)

	virtual void foo() {
		std::cout << this << " " << typeid(this).name() << std::endl;
	}
};

struct D3 : D1, D2 {
	DEFINE_GC_MEMBERS(
		(D1, D2),
		()
	)
};

//#include <variant>
//#include <string_view>
//#include <vector>
//#include <unordered_map>
//#include <utility>
//
//namespace json {
//	struct Value;
//	using Null = std::nullptr_t;
//	using Bool = bool;
//	using Number = double;
//	using String = std::string_view;
//	using Array = std::vector<Value>;
//	using Object = std::unordered_map<String, Value>;
//	struct Value : public std::variant<Object, Array, String, Number, Bool, Null> {
//		using variant::variant;
//	};
//} // end namespace json
//
//#include <cassert>
//
//int main() {
//	auto const Obj = json::Object{
//		{ "Hello", json::Value{ 1.5 } },
//		{ "World", json::Value{ true } },
//		{ "Foo", json::Value{ nullptr } },
//		{ "Bar", json::Value{ "Majestic" } } };
//
//	assert(std::get<json::Number>(Obj.at("Hello")) == 1.5);
//	assert(std::get<json::Bool>(Obj.at("World")) == true);
//	assert(std::get<json::Null>(Obj.at("Foo")) == nullptr);
//	assert(std::get<json::String>(Obj.at("Bar")) == json::String{ "Majestic" });
//	return 0;
//}

int main() {
	using namespace gc;
	{
		work_stealing_queue<void*> wsq;
		wsq.push(nullptr);
		wsq.push(nullptr);
		auto popped = wsq.pop();
		auto stolen = wsq.steal();
	}

	{
		work_stealing_queue<int> wsq;
		wsq.push(0);
		wsq.push(0);
		auto popped = wsq.pop();
		auto stolen = wsq.steal();
	}

	{
		struct alignas(16) X {
			X() noexcept : val(0) {}
			X(size_t v) : val(v) {}
			X(const X&) = default;

			size_t val;
		};
		work_stealing_queue<X> wsq;
		wsq.push(X{1});
		wsq.push(X{2});
		auto popped = wsq.pop();
		if(popped) {
			std::cout << popped->val << std::endl;
		}
		auto stolen = wsq.steal();

		if(auto x = std::get_if<X>(&stolen)) {
			std::cout << x->val << std::endl;
		}
	}

	{
		//arena arena;
		//void* zero = arena.allocate(0);
		//void* one = arena.allocate(1);
		//void* alig = arena.allocate(arena.maximum_alignment);
		//void* page = arena.allocate(arena.page_size);
		//void* pages = arena.allocate(arena.page_size * 2);
		//arena.deallocate(pages);
		//arena.deallocate(page);
		//arena.deallocate(alig);
		//arena.deallocate(one);
		//arena.deallocate(zero);
	}

	{
		handle<my_object> obj = gcnew<my_object>();
		obj->hello_world();
		obj->hello_world();
		handle<my_object> obj2(obj);
		obj2->hello_world();
		obj2->hello_world();
		handle<ambiguous_object  > ao = gcnew<ambiguous_object  >(3UL);
		ao->foo();
		handle<ambiguous_object[]> ar1 = gcnew<ambiguous_object[]>(4);
		handle<ambiguous_object[]> ar2 = gcnew<ambiguous_object[]>({ gcnew<ambiguous_object>(5UL) });
		ar2[0]->foo();
	}

	{
		handle<size_t[]          > sa1 = gcnew<size_t[]>(1);
		handle<size_t[]          > sa2 = gcnew<size_t[]>({ 1, 2, 3, 4 });
		std::cout << sa2[0] << std::endl;
	}

	{
		handle<combined> c = gcnew<combined>();
		handle<left> l1 = c;
		handle<left> l2(c);
		handle<left> l3; l3 = c;

		handle<combined> c2 = gc_cast<combined>(l1);
		handle<ambiguous_object> ao = gcnew<ambiguous_object>(3UL);
		handle<combined> c3 = gc_cast<combined>(ao);

		marker m;
		m.trace(c);
	}

	{
		handle<D3> d3 = gcnew<D3>();
		handle<D1> d1 = d3;
		handle<D2> d2 = d3;

		handle<B> b1 = d1; // ok
		handle<B> b2 = d2; // ok

		handle<B> b3 = gc_cast<B>(gc_cast<D1>(d3));

		std::cout << !!b1 << " " << !!b2 << std::endl;

		d1->foo();
		d2->foo();

		b1->foo();
		b2->foo();
	}

	return 0;

	//segment_size.QuadPart = 1 << 30;
	//HANDLE section = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, segment_size.HighPart, segment_size.LowPart, nullptr);
	//read_write = MapViewOfFile(section, FILE_MAP_WRITE, 0, 0, segment_size.QuadPart);
	//read_only = MapViewOfFile(section, FILE_MAP_READ, 0, 0, segment_size.QuadPart);

	//int* rw = static_cast<int*>(read_write);
	//int* ro = static_cast<int*>(read_only);

	//int ro1 = *ro;
	//*rw = 1;
	//int ro2 = *ro;

	//std::cout << ro1 << std::endl;
	//std::cout << ro2 << std::endl;

	//__try {
	//	*ro = 2;
	//} __except(filter(GetExceptionCode(), GetExceptionInformation())) {
	//	
	//}
}
