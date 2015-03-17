#ifndef opencl_frame
#define opencl_frame

#ifndef __CL_ENABLE_EXCEPTIONS
#define __CL_ENABLE_EXCEPTIONS 
#endif
#include "CL/cl.hpp"
#include "puzzler/puzzles/life.hpp"



#if !(defined(_WIN32) || defined(_WIN64))
#include <unistd.h>
void set_binary_io()
{}
#else
// http://stackoverflow.com/questions/341817/is-there-a-replacement-for-unistd-h-for-windows-visual-c
// http://stackoverflow.com/questions/13198627/using-file-descriptors-in-visual-studio-2010-and-windows
// Note: I could have just included <io.h> and msvc would whinge mightily, but carry on
  
#include <io.h>
#include <fcntl.h>

#define read _read
#define write _write
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

void set_binary_io()
{
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
}
#endif


#include <fstream>
#include <streambuf>


namespace puzzler {



class OpenclHelper {
protected:
  puzzler::ILog *log;
  cl::Platform platform;
  cl::Device device;
  std::vector<cl::Device> devices;
  cl::Context context;
  cl::Program program;

public:
  OpenclHelper(puzzler::ILog *log) {
    this->log = log;
    set_binary_io();
    log->LogVerbose("Setting up CL package now");
  }

  OpenclHelper& selectPlatform() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if(platforms.size()==0)
      throw std::runtime_error("No OpenCL platforms found.");

    log->LogDebug("Found %i platforms", platforms.size());
    for(unsigned i=0;i<platforms.size();i++){
      std::string vendor=platforms[i].getInfo<CL_PLATFORM_VENDOR>();
      log->LogDebug("  Platform %i: %s", i, vendor.c_str());
    }
    int selectedPlatform=0;
    if(getenv("HPCE_SELECT_PLATFORM")){
      selectedPlatform=atoi(getenv("HPCE_SELECT_PLATFORM"));
    }
    log->LogInfo("Choosing platform %i for use of OpenCL", selectedPlatform);
    this->platform=platforms.at(selectedPlatform);    

    return *this;
  }

  OpenclHelper& selectDevice() {
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);    
    if(devices.size()==0){
      throw std::runtime_error("No opencl devices found.\n");
    }

    log->LogDebug("Found %i devices", devices.size());
    for(unsigned i=0;i<devices.size();i++){
      std::string name=devices[i].getInfo<CL_DEVICE_NAME>();
      log->LogDebug("  Device %i: %s", i, name.c_str());
    }

    int selectedDevice=0;
    if(getenv("HPCE_SELECT_DEVICE")){
      selectedDevice=atoi(getenv("HPCE_SELECT_DEVICE"));
    }

    log->LogInfo("Choosing platform %i for use of OpenCL", selectedDevice);

    this->devices = devices;
    this->device  = devices.at(selectedDevice);

    return *this;
  }

  OpenclHelper& makeContext() {
    cl::Context context(this->devices);
    this->context = context;
    return *this; 
  }

  OpenclHelper& build(const char * filename) {
    std::string kernelSource=LoadSource(filename);

    cl::Program::Sources sources;   // A vector of (data,length) pairs
    sources.push_back(std::make_pair(kernelSource.c_str(), kernelSource.size()+1)); // push on our single string

    cl::Program program(context, sources);
    try{
      program.build(devices);
    }catch(...){
      for(unsigned i=0;i<devices.size();i++){
        std::cerr<<"Log for device "<<devices[i].getInfo<CL_DEVICE_NAME>()<<":\n\n";
        std::cerr<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i])<<"\n\n";
      }
      throw;
    }

    this->program = program;

    return *this;
  }

  cl::Program getProgram() {
    return this->program;
  }

  cl::Context getContext() {
    return this->context;
  }
  cl::Device getDevice() {
    return this->device;
  }

  std::string LoadSource(const char *fileName)
  {
    std::string baseDir="provider";
    if(getenv("HPCE_CL_SRC_DIR")){
      baseDir=getenv("HPCE_CL_SRC_DIR");
    }

    std::string fullName=baseDir+"/"+fileName;

    std::ifstream src(fullName, std::ios::in | std::ios::binary);
    if(!src.is_open())
      throw std::runtime_error("LoadSource : Couldn't load cl file from '"+fullName+"'.");

    return std::string(
      (std::istreambuf_iterator<char>(src)), // Node the extra brackets.
      std::istreambuf_iterator<char>()
    );
  }


};



};

#endif
