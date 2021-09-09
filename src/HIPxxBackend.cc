#include "HIPxxBackend.hh"

void HIPxxModule::add_kernel(void *HostFunctionPtr,
                             std::string HostFunctionName) {
  // TODO
  HIPxxKernel *kernel = new HIPxxKernel();
  Kernels.push_back(kernel);
}

bool HIPxxContext::add_device(HIPxxDevice *dev) {
  logTrace("HIPxxContext.add_device() {}", dev->get_name());
  hipxx_devices.push_back(dev);
  // TODO check for success
  return true;
}

bool HIPxxDevice::add_context(HIPxxContext *ctx) {
  logTrace("HIPxxDevice.add_context() {}");
  hipxx_contexts.push_back(ctx);
  // TODO check for success
  return true;
}

HIPxxContext *HIPxxDevice::get_default_context() {
  // TODO Check for initialization
  // if (hipxx_contexts.size() == 0)
  return hipxx_contexts.at(0);
}

bool HIPxxDevice::reserve_mem(size_t bytes) {
  logTrace("HIPxxDevice->reserve_mem()");
  std::lock_guard<std::mutex> Lock(mtx);
  if (bytes <= (hip_device_props.totalGlobalMem - TotalUsedMem)) {
    TotalUsedMem += bytes;
    if (TotalUsedMem > MaxUsedMem) MaxUsedMem = TotalUsedMem;
    logDebug("Currently used memory on dev {}: {} M\n", pcie_idx,
             (TotalUsedMem >> 20));
    return true;
  } else {
    logError(
        "Can't allocate {} bytes of memory on device # {}\n. "
        "GlobalMemSize:{} TotalUsedMem: {}",
        bytes, pcie_idx, hip_device_props.totalGlobalMem, TotalUsedMem);
    return false;
  }
}

bool HIPxxDevice::release_mem(size_t bytes) {
  std::lock_guard<std::mutex> Lock(mtx);
  if (TotalUsedMem >= bytes) {
    TotalUsedMem -= bytes;
    return true;
  } else {
    return false;
  }
}

hipError_t HIPxxExecItem::launchByHostPtr(const void *hostPtr) {
  if (q == nullptr) {
    logCritical("HIPxxExecItem.launch() was called but queue pointer is null");
    // TODO better errors
    std::abort();
  }

  HIPxxDevice *dev = q->get_device();
  this->Kernel = dev->findKernelByHostPtr(hostPtr);
  logTrace("Found kernel for host pointer {} : {}", hostPtr,
           Kernel->get_name());
  // TODO verify that all is in place either here or in HIPxxQueue
  return q->launch(this);
}

HIPxxKernel *HIPxxDevice::findKernelByHostPtr(const void *hostPtr) {
  logTrace("HIPxxDevice::findKernelByHostPtr({})", hostPtr);
  std::vector<HIPxxKernel *> hipxx_kernels = get_kernels();
  logDebug("Listing Kernels for device {}", device_name);
  for (auto &kernel : hipxx_kernels) {
    logDebug("{}", kernel->get_name());
  }

  auto found_kernel = std::find_if(hipxx_kernels.begin(), hipxx_kernels.end(),
                                   [&hostPtr](HIPxxKernel *kernel) {
                                     return kernel->get_host_ptr() == hostPtr;
                                   });

  if (found_kernel == hipxx_kernels.end()) {
    logCritical("Failed to find kernel {} on device #{}:{}", hostPtr, pcie_idx,
                device_name);
    std::abort();  // Exception
  } else {
    logDebug("Found kernel {} with host pointer {}",
             (*found_kernel)->get_name(), (*found_kernel)->get_host_ptr());
  }

  return *found_kernel;
}