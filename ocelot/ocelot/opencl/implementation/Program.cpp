// C standard library includes

// C++ standard library includes

// Ocelot includes
#include <ocelot/opencl/interface/Program.h>

unsigned int opencl::Program::_id = 0;


std::string opencl::Program::_loadBackDoor() {

	//Temorarily load ptx file as built binary
	std::ifstream binary("buildout.ptx", std::ifstream::in); 
	if(binary.fail()) {
		assertM(false, "using build backdoor but buildout.ptx not found");
		throw CL_BUILD_PROGRAM_FAILURE;
	}
	

	//Get file size
	binary.seekg(0, std::ios::end);
	size_t size = binary.tellg();
	binary.seekg(0, std::ios::beg);
	
	if(!size) {
		assertM(false, "buildout.ptx is empty!");
		throw CL_BUILD_PROGRAM_FAILURE;
	}

	//Read file to temporary buffer	
	std::string temp;
	temp.resize(size);
	binary.read((char*)temp.data(), size);

	return temp;

}

bool opencl::Program::_isBuiltOnDevice(Device * device) {
	
	if(_deviceBuiltInfo.find(device) == _deviceBuiltInfo.end())
		return false;

	if(_deviceBuiltInfo[device]._status == STATUS_EXECUTABLE)
		return true;

	return false;
}

bool opencl::Program::_isAllBuilt() {

	for(std::map<Device *, deviceBuiltInfoT>::iterator it = _deviceBuiltInfo.begin();
		it != _deviceBuiltInfo.end(); it++) {

		if(it->second._status != STATUS_EXECUTABLE)
			return false;
	}

	return true;

}

bool opencl::Program::_hasModuleOnDevice(Device * device, std::string moduleName) {
	if(_deviceBuiltInfo.find(device) == _deviceBuiltInfo.end())
		return false;

	if(_deviceBuiltInfo[device]._moduleName == moduleName)
		return true;

	return false;
}

bool opencl::Program::_hasBinaryOnDevice(Device * device) {
	if(_deviceBuiltInfo.find(device) == _deviceBuiltInfo.end())
		return false;

	if(_deviceBuiltInfo[device]._binary.empty())
		return false;

	return true;
}

void opencl::Program::_buildOnDevice(Device * device, std::string backdoorBinary) {

	std::stringstream moduleName;
	moduleName << _name;
	report("Loading module (ptx) - " << moduleName.str());

	try {

		if(!_hasModuleOnDevice(device, moduleName.str())) {

			_deviceBuiltInfo[device]._moduleName = moduleName.str();
			_deviceBuiltInfo[device]._module = new ir::Module();
		}

		if(_deviceBuiltInfo[device]._binary.empty()) {
			assert(!backdoorBinary.empty());
			_deviceBuiltInfo[device]._binary = backdoorBinary;
		}
	
		_deviceBuiltInfo[device]._module->lazyLoad(_deviceBuiltInfo[device]._binary,
			moduleName.str());

		device->load(_deviceBuiltInfo[device]._module);
				
		_deviceBuiltInfo[device]._status = STATUS_EXECUTABLE;
	}
	catch(...) {
		throw CL_BUILD_PROGRAM_FAILURE;
	}

}

char * opencl::Program::_getBinarySizes(size_t & bufferLen) {

	char * sizes;
	size_t entries;

	if(_type == PROGRAM_SOURCE) {
		entries = _context->validDevices.size();
		sizes = new char[entries * sizeof(size_t)];
	
		size_t i = 0;
		for(Device::DeviceList::iterator device = _context->validDevices.begin();
				device != _context->validDevices.end(); device++) {
		
			if(_deviceBuiltInfo.find(*device) != _deviceBuiltInfo.end())
				((size_t *)sizes)[i] = _deviceBuiltInfo[*device]._binary.size();
			else
				((size_t *)sizes)[i] = 0;
			i++;
		}
		bufferLen = entries * sizeof(size_t);
		return sizes;
	}

	else { //binary

		entries = _deviceBuiltInfo.size();
		sizes = new char[entries * sizeof(size_t)];

		size_t i = 0;
		for(std::map <Device *, deviceBuiltInfoT> ::iterator it = _deviceBuiltInfo.begin();
			it != _deviceBuiltInfo.end(); it++) {
			((size_t *)sizes)[i] = it->second._binary.size();
			i++;
		}
		bufferLen = entries * sizeof(size_t);
		return sizes;
	}
}

char * opencl::Program::_getBinaries(size_t & bufferLen) {

	char * binaries;
	size_t entries;

	if(_type == PROGRAM_SOURCE) {
		entries = _context->validDevices.size();
		binaries = new char[entries * sizeof(unsigned char *)];
	
		size_t i = 0;
		for(Device::DeviceList::iterator device = _context->validDevices.begin();
				device != _context->validDevices.end(); device++) {
		
			if(_deviceBuiltInfo.find(*device) != _deviceBuiltInfo.end())
				((const char **)binaries)[i] = _deviceBuiltInfo[*device]._binary.c_str();
			else
				((const char **)binaries)[i] = NULL;
			i++;
		}
		bufferLen = entries * sizeof(unsigned char *);
		return binaries;
	}

	else { //binary

		entries = _deviceBuiltInfo.size();
		binaries = new char [entries * sizeof(unsigned char *)];

		size_t i = 0;
		for(std::map <Device *, deviceBuiltInfoT> ::iterator it = _deviceBuiltInfo.begin();
			it != _deviceBuiltInfo.end(); it++) {
			((const char **)binaries)[i] = it->second._binary.c_str();
			i++;
		}
		bufferLen = entries * sizeof(unsigned char *);
		return binaries;
	}
}

bool opencl::Program::_hasKernelAll(const char * kernelName) {

	for(std::map < Device *, deviceBuiltInfoT>::iterator it = _deviceBuiltInfo.begin();
		it != _deviceBuiltInfo.end(); it++) {

		if(it->second._module->getKernel(kernelName) == 0) { //kernel not found
			return false;
		}
	}

	return true;

}

opencl::Program::Program(Context * context, cl_uint count, const char ** strings, 
	const size_t * lengths, programT type):
	Object(OBJTYPE_PROGRAM), 
	_type(type), _context(context) {

	//program name = __clmodule_id
	std::stringstream name;
	name << "__clmodule_" << _id;
	_name = std::move(name.str());
	_id++;

	//put all sources together
	std::stringstream sources;
	for(cl_uint i = 0; i < count; i++) {

		if(strings[i] == 0)
			throw CL_INVALID_VALUE;

		if(lengths == 0 || lengths[i] == 0)
			sources << strings[i];
		else
			sources.write(strings[i], std::min(lengths[i], strlen(strings[i])));

		_source = std::move(sources.str());

	}

	_context->retain();
}

opencl::Program::~Program() {
	
	for(std::map <Device *, deviceBuiltInfoT>::iterator it = _deviceBuiltInfo.begin();
		it != _deviceBuiltInfo.end(); it++) {
		delete it->second._module;
	}

	if(_context->release())
		delete _context;
}

bool opencl::Program::isValidContext(Context * context) {
	return (_context == context);
}

bool opencl::Program::setupDevices(cl_uint num_devices, const cl_device_id * device_list) {

	_buildDevices.clear();

	if(num_devices == 0) { //build on all devices
		_buildDevices = _context->validDevices;
		return true;
	}
	
	for(cl_uint i = 0; i < num_devices; i++) {
		if(std::find(_context->validDevices.begin(), 
			_context->validDevices.end(),
			(Device *)device_list[i]) == _context->validDevices.end()) {//Not found
			_buildDevices.clear();
			return false;
		}
		_buildDevices.push_back((Device *)device_list[i]);

	}

	return true;
}

void opencl::Program::build(const char * options,
		void (CL_CALLBACK * pfn_notify)(cl_program, void *),
		void * user_data) {

	if(!_kernels.empty())
		throw CL_INVALID_OPERATION;

	assert(!_buildDevices.empty());

	if(_type == PROGRAM_SOURCE) {//source code
		
		// Load backdoor ptx
		std::string binary = _loadBackDoor();	

		for(Device::DeviceList::iterator d = _buildDevices.begin(); d != _buildDevices.end(); d++) {

			if(!_isBuiltOnDevice(*d)) {
				_buildOnDevice(*d, binary);
			}
		}
	}
	else { //binary code
		
		for(Device::DeviceList::iterator d = _buildDevices.begin(); d != _buildDevices.end(); d++) {

			if(_hasBinaryOnDevice(*d)) //binary not availble for device
				throw CL_INVALID_BINARY;

			if(!_isBuiltOnDevice(*d)) {
				_buildOnDevice(*d);
			}
		}
	}
}


void opencl::Program::getInfo(cl_program_info param_name,
	size_t param_value_size,
	void * param_value,
	size_t * param_value_size_ret) {

	union infoUnion {
		cl_uint uint;
		cl_context context;
		size_t size;
	};

	infoUnion info;
	char * ptr = (char *)&info;
	size_t infoLen = 0;
	bool isMalloc = false;

	switch(param_name) {
		case CL_PROGRAM_BINARY_SIZES: 
			ptr = _getBinarySizes(infoLen);
			isMalloc = true;
			break;

		case CL_PROGRAM_BINARIES:
			ptr = _getBinaries(infoLen);
			isMalloc = true;
			break;
		
		default:
			assertM(false, "unsupported program info");
			throw CL_UNIMPLEMENTED;
			break;
	}
	
	if(param_value_size < infoLen)
		throw CL_INVALID_VALUE;

	if(param_value)
		std::memcpy(param_value, ptr, infoLen);

	if(param_value_size_ret)
		*param_value_size_ret = infoLen;

	if(isMalloc)
		delete[] ptr;
}

opencl::Kernel * opencl::Program::createKernel(const char * kernelName) {
	if(!_isAllBuilt())
		throw CL_INVALID_PROGRAM_EXECUTABLE;

	if(!kernelName)
		throw CL_INVALID_VALUE;

	if(!_hasKernelAll(kernelName))
		throw CL_INVALID_KERNEL_NAME;

	report("Registered kernel - " << kernelName
		<< " in program '" << _name << "'");

	Kernel * kernel = NULL;
	kernel = new Kernel(kernelName, this);
	_kernels.push_back(kernel);

	//add device info to kernel
	for(std::map < Device *, deviceBuiltInfoT>::iterator it = _deviceBuiltInfo.begin();
		it != _deviceBuiltInfo.end(); it++) {

		ir::Kernel * irKernel = it->second._module->getKernel(kernelName);

		kernel->addDeviceInfo(it->first, it->second._moduleName, it->second._module, irKernel);
	}

	return kernel;

}

void opencl::Program::removeKernel(Kernel * kernel) {
	KernelList::iterator it = std::find(_kernels.begin(), _kernels.end(), kernel);
	assert(it != _kernels.end());
	_kernels.erase(it);
}
