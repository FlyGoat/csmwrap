#ifndef _IO_H
#define _IO_H

#include <efi.h>
#include <printf.h>

#define barrier() __asm__ __volatile__("": : :"memory")

static inline void writeq(void *addr, uint32_t val) {
    barrier();
    *(volatile uint64_t *)addr = val;
}
static inline void writel(void *addr, uint32_t val) {
    barrier();
    *(volatile uint32_t *)addr = val;
}
static inline void writew(void *addr, uint16_t val) {
    barrier();
    *(volatile uint16_t *)addr = val;
}
static inline void writeb(void *addr, uint8_t val) {
    barrier();
    *(volatile uint8_t *)addr = val;
}
static inline uint64_t readq(const void *addr) {
    uint64_t val = *(volatile const uint64_t *)addr;
    barrier();
    return val;
}
static inline uint32_t readl(const void *addr) {
    uint32_t val = *(volatile const uint32_t *)addr;
    barrier();
    return val;
}
static inline uint16_t readw(const void *addr) {
    uint16_t val = *(volatile const uint16_t *)addr;
    barrier();
    return val;
}
static inline uint8_t readb(const void *addr) {
    uint8_t val = *(volatile const uint8_t *)addr;
    barrier();
    return val;
}

#ifdef __OPTIMIZE__

#define	__use_immediate_port(port) \
	(__builtin_constant_p(((int)port)) && ((int)port) < 0x100)

#else

#define	__use_immediate_port(port)	0

#endif


#define	inb(port) \
	(__use_immediate_port((int)port) ? __inbc((int)port) : __inb((int)port))

static inline uint8_t
__inbc(int port)
{
	uint8_t data;
	asm volatile("inb %w1,%0" : "=a" (data) : "id" (port));
	return data;
}

static inline uint8_t
__inb(int port)
{
	uint8_t data;
	asm volatile("inb %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void
insb(int port, void *addr, int cnt)
{
	asm volatile("cld\n\trepne\n\tinsb"
	    : "+D" (addr), "+c" (cnt) : "d" (port) : "memory", "cc");
}

#define	inw(port) \
	(__use_immediate_port((int)port) ? __inwc((int)port) : __inw((int)port))

static inline uint16_t
__inwc(int port)
{
	uint16_t data;
	asm volatile("inw %w1,%0" : "=a" (data) : "id" (port));
	return data;
}

static inline uint16_t
__inw(int port)
{
	uint16_t data;
	asm volatile("inw %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void
insw(int port, void *addr, int cnt)
{
	asm volatile("cld\n\trepne\n\tinsw"
	    : "+D" (addr), "+c" (cnt) : "d" (port) : "memory", "cc");
}

#define	inl(port) \
	(__use_immediate_port(port) ? __inlc((int)port) : __inl((int)port))

static inline uint32_t
__inlc(int port)
{
	uint32_t data;
	asm volatile("inl %w1,%0" : "=a" (data) : "id" (port));
	return data;
}

static inline uint32_t
__inl(int port)
{
	uint32_t data;
	asm volatile("inl %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void
insl(int port, void *addr, int cnt)
{
	asm volatile("cld\n\trepne\n\tinsl"
	    : "+D" (addr), "+c" (cnt) : "d" (port) : "memory", "cc");
}

#define	outb(port, data) \
	(__use_immediate_port(port) ? __outbc((int)port, data) : __outb((int)port, data))

static inline void
__outbc(int port, uint8_t data)
{
	asm volatile("outb %0,%w1" : : "a" (data), "id" (port));
}

static inline void
__outb(int port, uint8_t data)
{
	asm volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static inline void
outsb(int port, const void *addr, int cnt)
{
	asm volatile("cld\n\trepne\n\toutsb"
	    : "+S" (addr), "+c" (cnt) : "d" (port) : "cc");
}

#define	outw(port, data) \
	(__use_immediate_port(port) ? __outwc((int)port, data) : __outw((int)port, data))

static inline void
__outwc(int port, uint16_t data)
{
	asm volatile("outw %0,%w1" : : "a" (data), "id" (port));
}

static inline void
__outw(int port, uint16_t data)
{
	asm volatile("outw %0,%w1" : : "a" (data), "d" (port));
}

static inline void
outsw(int port, const void *addr, int cnt)
{
	asm volatile("cld\n\trepne\n\toutsw"
	    : "+S" (addr), "+c" (cnt) : "d" (port) : "cc");
}

#define	outl(port, data) \
	(__use_immediate_port(port) ? __outlc((int)port, data) : __outl((int)port, data))

static inline void
__outlc(int port, uint32_t data)
{
	asm volatile("outl %0,%w1" : : "a" (data), "id" (port));
}

static inline void
__outl(int port, uint32_t data)
{
	asm volatile("outl %0,%w1" : : "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt)
{
	asm volatile("cld\n\trepne\n\toutsl"
	    : "+S" (addr), "+c" (cnt) : "d" (port) : "cc");
}

static inline uint64_t rdmsr(uint32_t index)
{
    uint64_t ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    return ret;
}

static inline void wrmsr(uint32_t index, uint64_t val)
{
    asm volatile ("wrmsr" : : "c"(index), "A"(val));
}


#endif
