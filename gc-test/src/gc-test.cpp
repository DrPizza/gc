// gc-test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gc/gc.hpp"
#include "gc/gc-macros.hpp"

#include "gc/detail/ws-queue.hpp"

#pragma warning(push)
#pragma warning(disable: 4061 4242 4244 4365 26426 26429 26439 26440 26461 26482 26493 26494 26496 26497)
extern "C" {
#include "xed/xed-interface.h"
}
#pragma warning(pop)

void* read_write;
void* read_only;
ULARGE_INTEGER segment_size;

bool is_in_ro_segment(const void* const address) noexcept {
	if(static_cast<const byte*>(read_only) <= static_cast<const byte*>(address  )
	&& static_cast<const byte*>(address  ) <  static_cast<const byte*>(read_only) + segment_size.QuadPart) {
		return true;
	}
	return false;
}

std::size_t get_ro_offset(const void* const address) noexcept {
	if(is_in_ro_segment(address)) {
		return gsl::narrow_cast<std::size_t>(static_cast<const byte*>(address) - static_cast<const byte*>(read_only));
	} else {
		return 0;
	}
}

void* ro_to_rw(const void* const address) noexcept {
	if(is_in_ro_segment(address)) {
		return static_cast<byte*>(read_write) + get_ro_offset(address);
	} else {
		return nullptr;
	}
}

#pragma warning(push)
#pragma warning(disable: 4061) // warning C4061: enumerator '%s' in switch of enum '%s' is not explicitly handled by a case label
[[gsl::suppress(type.1)]]
LONG WINAPI exception_filter(_EXCEPTION_POINTERS* ep) noexcept {
	if(ep->ExceptionRecord->ExceptionCode != 0xc000'0005) { // access denied
		return EXCEPTION_EXECUTE_HANDLER;
	}
	if(ep->ExceptionRecord->ExceptionInformation[0] != 1) { // write
		return EXCEPTION_EXECUTE_HANDLER;
	}
	const std::size_t target = ep->ExceptionRecord->ExceptionInformation[1];
	if(!is_in_ro_segment(reinterpret_cast<void*>(target))) {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	const xed_machine_mode_enum_t mmode = XED_MACHINE_MODE_LONG_64;
	const xed_address_width_enum_t stack_addr_width = XED_ADDRESS_WIDTH_64b;
	xed_decoded_inst_t xedd[1];
	xed_decoded_inst_zero(xedd);
	xed_decoded_inst_set_mode(xedd, mmode, stack_addr_width);
	const xed_error_enum_t xed_error = xed_decode(xedd, reinterpret_cast<const unsigned char*>(ep->ContextRecord->Rip), 15);
	if(xed_error != XED_ERROR_NONE) {
		std::printf("Unhandled error code %s\n", xed_error_enum_t2str(xed_error));
		return EXCEPTION_EXECUTE_HANDLER;
	}

	const unsigned int memops = xed_decoded_inst_number_of_memory_operands(xedd);
	if(0 == memops) {
		return EXCEPTION_EXECUTE_HANDLER;
	}
	DWORD64* memory_register = nullptr;
	for(unsigned int i = 0; i < memops; ++i) {
		if(!xed_decoded_inst_mem_written(xedd, i)) {
			continue;
		}

		const xed_reg_enum_t base = xed_decoded_inst_get_base_reg(xedd, i);
		if(base == XED_REG_INVALID) {
			continue;
		}

		switch(base) {
		case XED_REG_RAX:
			memory_register = &ep->ContextRecord->Rax;
			break;
		case XED_REG_RBX:
			memory_register = &ep->ContextRecord->Rbx;
			break;
		case XED_REG_RCX:
			memory_register = &ep->ContextRecord->Rcx;
			break;
		case XED_REG_RDX:
			memory_register = &ep->ContextRecord->Rdx;
			break;
		case XED_REG_RSP:
			memory_register = &ep->ContextRecord->Rsp;
			break;
		case XED_REG_RBP:
			memory_register = &ep->ContextRecord->Rbp;
			break;
		case XED_REG_RSI:
			memory_register = &ep->ContextRecord->Rsi;
			break;
		case XED_REG_RDI:
			memory_register = &ep->ContextRecord->Rdi;
			break;
		case XED_REG_R8:
			memory_register = &ep->ContextRecord->R8;
			break;
		case XED_REG_R9:
			memory_register = &ep->ContextRecord->R9;
			break;
		case XED_REG_R10:
			memory_register = &ep->ContextRecord->R10;
			break;
		case XED_REG_R11:
			memory_register = &ep->ContextRecord->R11;
			break;
		case XED_REG_R12:
			memory_register = &ep->ContextRecord->R12;
			break;
		case XED_REG_R13:
			memory_register = &ep->ContextRecord->R13;
			break;
		case XED_REG_R14:
			memory_register = &ep->ContextRecord->R14;
			break;
		case XED_REG_R15:
			memory_register = &ep->ContextRecord->R15;
			break;
		default:
			continue;
		}
		if(memory_register != nullptr) {
			break;
		}
	}
	if(memory_register == nullptr) {
		return EXCEPTION_EXECUTE_HANDLER;
	}
	*memory_register = reinterpret_cast<DWORD64>(ro_to_rw(reinterpret_cast<const void*>(*memory_register)));
	return EXCEPTION_CONTINUE_EXECUTION;
}
#pragma warning(pop)

int filter(unsigned int, _EXCEPTION_POINTERS* ep) noexcept {
	return exception_filter(ep);
}

struct my_object : garbage_collection::object {
	virtual void hello_world() const {
		std::cout << "Hello, World!" << std::endl;
	}

	virtual void _gc_trace(gsl::not_null<garbage_collection::visitor*> visitor) const noexcept override {
		garbage_collection::object::_gc_trace(visitor);
	}
};

struct ambiguous_object : garbage_collection::object {
	ambiguous_object(size_t parameter) noexcept : value(parameter) {
	}

	void foo() const volatile {
		std::cout << "foo: " << value << std::endl;
	}

	virtual void _gc_trace(gsl::not_null<garbage_collection::visitor*> visitor) const noexcept override {
		garbage_collection::object::_gc_trace(visitor);
	}

private:
	size_t value;
};

struct left : garbage_collection::object
{
	virtual void _gc_trace(gsl::not_null<garbage_collection::visitor*> visitor) const noexcept override {
		garbage_collection::object::_gc_trace(visitor);
	}
};

struct right : garbage_collection::object
{
	virtual void _gc_trace(gsl::not_null<garbage_collection::visitor*> visitor) const noexcept override {
		garbage_collection::object::_gc_trace(visitor);
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
	combined() noexcept : ao1(garbage_collection::gcnew<ambiguous_object>(10u)),
	                      ao2(garbage_collection::gcnew<ambiguous_object>(11u))
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

struct B : garbage_collection::object {
	DEFINE_GC_MEMBERS(
		(garbage_collection::object),
		()
	)

	virtual void foo() {
		std::cout << this << " " << typeid(this).name() << std::endl;
	}

	virtual ~B() override {
		std::cout << __FUNCSIG__ << std::endl;
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

	virtual ~D1() override {
		std::cout << __FUNCSIG__ << std::endl;
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

	virtual ~D2() override {
		std::cout << __FUNCSIG__ << std::endl;
	}
};

struct D3 : D1, D2 {
	DEFINE_GC_MEMBERS(
		(D1, D2),
		()
	)

	virtual ~D3() override {
		std::cout << __FUNCSIG__ << std::endl;
	}
};

struct Node : garbage_collection::object {
	DEFINE_GC_MEMBERS(
		(garbage_collection::object),
		(
			(public, Node, next)
		)
	)
};

struct Nested : garbage_collection::object {
	Nested() : children(garbage_collection::gcnew<Nested[]>(4)) {
	}

	DEFINE_GC_MEMBERS(
		(garbage_collection::object),
		(
		(
			public, Nested[], children)
		)
	)
};

int main() {
	using namespace garbage_collection;
#if 0
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
			X() noexcept : val(0) {
			}
			X(size_t v) : val(v) {
			}
			X(const X&) = default;

			size_t val;
		};
		work_stealing_queue<X> wsq;
		wsq.push(X{ 1 });
		wsq.push(X{ 2 });
		auto popped = wsq.pop();
		if(popped) {
			std::cout << popped->val << std::endl;
		}
		auto stolen = wsq.steal();

		if(auto x = std::get_if<X>(&stolen)) {
			std::cout << x->val << std::endl;
		}
	}
#endif
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
#if 1
	the_gc.collect();
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
	the_gc.collect();
	{
		handle<Node> first = gcnew<Node>();
		handle<Node> second = gcnew<Node>();
		first->next = second;
		second->next = first;
		first = nullptr;
		second = nullptr;
	}
	the_gc.collect();
	{
		handle<Nested> n = gcnew<Nested>();
		n->children = gcnew<Nested[]>(1);
	}
	the_gc.collect();
	{
		handle<Node> root = gcnew<Node>();
		handle<Node> current = root;
		for(size_t i = 0; i < 64; ++i) {
			current->next = gcnew<Node>();
			current = current->next;
		}
		current->next = root;
		root = nullptr;
	}
	the_gc.collect();
	{
		handle<size_t[]> sa1 = gcnew<size_t[]>(1);
		handle<size_t[]> sa2 = gcnew<size_t[]>({ 1, 2, 3, 4 });
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

	{
		handle<D3> d3 = gcnew<D3>();
	}
	{
		handle<size_t[]> sa1 = gcnew<size_t[1]>();
		//handle<size_t[][]> sa2 = gcnew<size_t[][]>(5);
	}
#if 0
	{
		handle<box<int>> b1 = gcnew<box<int>>(1);
		handle<int> b2 = gcnew<int>(1);

#define TEST(type) std::cout << #type << " normalized: " << typeid(normalize_t<type>).name() << std::endl
		TEST(int);
		TEST(int[]);
		TEST(int[1][1]);
		TEST(int[][1]);
		TEST(array<int>);
		TEST(array<int[]>);
		TEST(array<int[1]>);
		TEST(array<int[][1]>);
		TEST(array<int[1][1]>);
		TEST(array<int>[]);
		TEST(array<int>[1]);
		TEST(array<int>[][1]);
		TEST(array<int>[1][1]);
		TEST(array<int[]>[]);
		TEST(array<int[]>[1]);
		TEST(array<int[1]>[1]);
		TEST(array<array<int>>);
		TEST(B[]);
		TEST(array<B>);
		TEST(array<array<B>>);
		TEST(B);
		TEST(my_object);
		TEST(ambiguous_object);
#undef TEST

		std::cout << typeid(handle<my_object>).name() << std::endl;
		std::cout << typeid(handle<ambiguous_object>).name() << std::endl;

		std::cout << sizeof(raw_reference) << std::endl;
		std::cout << sizeof(handle<my_object>) << std::endl;
		std::cout << sizeof(object) << std::endl;
	}
#endif
	the_gc.collect();

	return 0;
#endif
	xed_tables_init();
	::SetUnhandledExceptionFilter(&exception_filter);

	segment_size.QuadPart = 1 << 30;
	HANDLE section = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, segment_size.HighPart, segment_size.LowPart, nullptr);
	read_write = MapViewOfFile(section, FILE_MAP_WRITE, 0, 0, segment_size.QuadPart);
	read_only = MapViewOfFile(section, FILE_MAP_READ, 0, 0, segment_size.QuadPart);


	int* rw = static_cast<int*>(read_write);
	int* ro = static_cast<int*>(read_only);

	std::cout << *ro << std::endl;
	*rw = 1;
	std::cout << *ro << std::endl;

	*ro = 2;
	(*ro)++;
	*ro += 1;
	*ro *= 2;

	std::cout << *ro << std::endl;
}
