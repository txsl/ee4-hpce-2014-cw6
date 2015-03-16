
#include "puzzler/puzzler.hpp"

#include <iostream>
#include "../provider/opencl_frame.hpp"

int main()
{
   std::shared_ptr<puzzler::ILog> logDest = 
      std::make_shared<puzzler::LogDest>("opencl_test", 4);
   logDest->Log(puzzler::Log_Info, "Created log.");

   puzzler::OpenclHelper opencl(logDest.get());
   opencl.selectPlatform().selectDevice().makeContext();
   opencl.build("test.cl");

   cl::Kernel kernel(opencl.getProgram(), "kernel_testcl");

   cl::CommandQueue queue(opencl.getContext(), opencl.getDevice());

   cl::NDRange offset(0, 0);               // Always start iterations at x=0, y=0
   cl::NDRange globalSize(2, 2);   // Global size must match the original loops
   cl::NDRange localSize=cl::NullRange;    // We don't care about local size
   
   kernel.setArg(0, 50);
   queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);
   queue.enqueueBarrier(); // <- new barrier here

   return 0;
}

