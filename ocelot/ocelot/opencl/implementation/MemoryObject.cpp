#include <ocelot/opencl/interface/OpenCLRuntimeInterface.h>
#include <ocelot/opencl/interface/MemoryObject.h>


opencl::MemoryObject::MemoryObject(
	std::map < executive::Device *, executive::Device::MemoryAllocation * > & a, 
	Context * context, cl_mem_object_type type, cl_mem_flags flags)
	:allocations(a), _context(context), _type(type), _flags(flags) {
}

opencl::Context * opencl::MemoryObject::context() const {
	return _context;
}

const cl_mem_object_type opencl::MemoryObject::type() const {
	return _type;
}

const cl_mem_flags opencl::MemoryObject::flags() const {
	return _flags;
}

opencl::BufferObject::BufferObject(
	std::map< executive::Device *, executive::Device::MemoryAllocation * > & allocations, 
	Context * context, cl_mem_flags flags, size_t size)
	:MemoryObject(allocations, context, CL_MEM_OBJECT_BUFFER, flags), _size(size) {
}

const size_t opencl::BufferObject::size() const {
	return _size;
}
