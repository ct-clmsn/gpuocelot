/*! \file ExecutableKernel.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 19, 2009
	\brief implements a kernel that is executable on some device
*/

#ifndef EXECUTABLE_KERNEL_H_INCLUDED
#define EXECUTABLE_KERNEL_H_INCLUDED

#include <ocelot/ir/interface/PTXKernel.h>
#include <ocelot/ir/interface/Kernel.h>
#include <ocelot/ir/interface/Texture.h>
#include <ocelot/ir/interface/Dim3.h>

namespace executive {
	class Device;
}

namespace trace {
	class TraceEvent;
	class TraceGenerator;
}

namespace executive {
	class ExecutableKernel : public ir::Kernel {
	public:
		typedef std::vector< trace::TraceGenerator* > TraceGeneratorVector;
		typedef std::vector< const ir::Texture* > TextureVector;

	public:
		executive::Device* device;

	public:
		ExecutableKernel(const ir::PTXKernel& k, executive::Device* d = 0);
		ExecutableKernel(const ir::Kernel& k, executive::Device* d = 0);
		ExecutableKernel(executive::Device* d = 0);
		virtual ~ExecutableKernel();
	
		/*!	\brief Determines whether kernel is executable */
		virtual bool executable();
		
		/*!	\brief Launch a kernel on a 2D grid */
		virtual void launchGrid(int width, int height)=0;

		/*!
			\brief compute argument offsets for argument data
			\return number of bytes required for argument memory
		*/
		virtual size_t mapArgumentOffsets();

		/*!
			\brief given a block of argument memory, sets the values of 
				each argument
			\param argument pointer to argument memory
			\param argumentSize number of bytes to write to argument memory
		*/
		virtual void setArgumentBlock(const unsigned char *argument, 
			size_t argumentSize);

		/*!
			\brief gets the values of each argument as a block of binary data
			\param argument pointer to argument memory
			\param maxSize maximum number of bytes to write to argument memory
			\return actual number of bytes required by argument memory
		*/
		virtual size_t getArgumentBlock(unsigned char *argument,
			size_t maxSize) const;
	
		/*!	\brief Sets the shape of a kernel */
		virtual void setKernelShape(int x, int y, int z)=0;

		/*! \brief Changes the amount of external shared memory */
		virtual void setExternSharedMemorySize(unsigned int)=0;
		
		/*! \brief Sets the max number of pthreads this kernel can use */
		virtual void setWorkerThreads(unsigned int workerThreadLimit)=0;
			
		/*! \brief Indicate that the kernels arguments have been updated */
		virtual void updateArgumentMemory()=0;
		
		/*! \brief Indicate that other memory has been updated */
		virtual void updateMemory()=0;
		
		/*! \brief Get a vector of all textures references by the kernel */
		virtual TextureVector textureReferences() const=0;

		/*!	Notifies all attached TraceGenerators of an event */
		void traceEvent(const trace::TraceEvent & event) const;

		/*!	Notifies all attached TraceGenerators of completion of an event */
		void tracePostEvent(const trace::TraceEvent & event) const;
		
		/*!	adds a trace generator to the EmulatedKernel */
		virtual void addTraceGenerator(trace::TraceGenerator* generator)=0;

		/*!	removes a trace generator from an EmulatedKernel */
		virtual void removeTraceGenerator(trace::TraceGenerator* generator)=0;

	public:
		/*! attribute accessors - things every executable kernel should know */
		unsigned int constMemorySize() const;
		unsigned int localMemorySize() const;
		unsigned int maxThreadsPerBlock() const;
		unsigned int registerCount() const;
		unsigned int sharedMemorySize() const;
		unsigned int externSharedMemorySize() const;
		unsigned int totalSharedMemorySize() const;
		unsigned int argumentMemorySize() const;
		unsigned int parameterMemorySize() const;
		const ir::Dim3& blockDim() const;
		const ir::Dim3& gridDim() const;

	protected:
		/*! \brief Total amount of allocated constant memory size */
		unsigned int _constMemorySize;
		/*! \brief Total amount of allocated local memory size */
		unsigned int _localMemorySize;
		/*! \brief Maxmimum number of threads launched per block */
		unsigned int _maxThreadsPerBlock;
		/*! \brief Number of registered required by each thread */
		unsigned int _registerCount;
		/*! \brief The amount of allocated static shared memory */
		unsigned int _sharedMemorySize;
		/*! \brief The amount of allocated dynamic shared memory */
		unsigned int _externSharedMemorySize;
		/*! \brief Total amount of packed parameter memory */
		unsigned int _argumentMemorySize;
        /*! \brief Kernel stack parameter memory space */
        unsigned int _parameterMemorySize;
		/*! \brief The block dimensions */
		ir::Dim3 _blockDim;
		/*!	\brief Dimension of grid in blocks */
		ir::Dim3 _gridDim;
		/*! \brief Attached trace generators */
		TraceGeneratorVector _generators;

	};
	
}

#endif
