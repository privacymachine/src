/*==============================================================================
        Copyright (c) 2013-2017 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175

                     Licensed under the EUPL, Version 1.1
     European Commission - subsequent versions of the EUPL (the "Licence");
        You may not use this work except in compliance with the Licence.
                  You may obtain a copy of the Licence at:
                        http://ec.europa.eu/idabc/eupl

 Unless required by applicable law or agreed to in writing, software distributed
              under the Licence is distributed on an "AS IS" basis,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
      See the Licence for the specific language governing permissions and
                        limitations under the Licence.
==============================================================================*/

// Compile by using: /EHsc /W4

#include "CpuFeatures.h"
#include "utils.h"

#ifndef _WIN32_WINNT
  #define  _WIN32_WINNT   0x0501
#endif
#include <SDKDDKVer.h>


#include <iostream>
#include <string>

using namespace std;


bool CpuFeatures::Virtualization(void)
{  
  // TODO: Detection does not work!
  return true;
  
  // Good Example: https://gist.github.com/macton/4dd5fec2113be284796e
  //
  // Intel: http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-system-programming-manual-325384.pdf
  //        If CPUID.1:ECX.VMX[bit 5] = 1, then VMX operation is supported.
  //        [macton-example] print_feature( ecx & (1<< 5), "VMX           (Virtual Machine Extensions)");
  //
  // AMD: http://developer.amd.com/wordpress/media/2012/10/254811.pdf
  //      SVM: secure virtual machine. See “Secure Virtual Machine” in APM2. 
  //      CPUID Fn8000_0001_ECX Bit-2
  //      [macton-example] print_feature( ecx & (1<< 2), "SVM           (Secure Virtual Machine)");

  if (CPU_Rep.isIntel_) 
    return CPU_Rep.f_81_ECX_[5];
    
  if (CPU_Rep.isAMD_)
    return CPU_Rep.f_81_ECX_[2];
  
  return false;
}

CpuFeatures::CpuFeatures_Internal::CpuFeatures_Internal()
    : nIds_{ 0 },
    nExIds_{ 0 },
    isIntel_{ false },
    isAMD_{ false },
    f_1_ECX_{ 0 },
    f_1_EDX_{ 0 },
    f_7_EBX_{ 0 },
    f_7_ECX_{ 0 },
    f_81_ECX_{ 0 },
    f_81_EDX_{ 0 },
    data_{},
    extdata_{}
  {
    std::array<int, 4> cpui;

    // Calling __cpuid with 0x0 as the function_id argument
    // gets the number of the highest valid function ID.
    __cpuid(cpui.data(), 0);
    nIds_ = cpui[0];

    for (int i = 0; i <= nIds_; ++i)
    {
      __cpuidex(cpui.data(), i, 0);
      data_.push_back(cpui);
    }

    // Capture vendor string
    char vendor[0x20];
    memset(vendor, 0, sizeof(vendor));
    *reinterpret_cast<int*>(vendor) = data_[0][1];
    *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
    *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
    vendor_ = vendor;
    if (vendor_ == "GenuineIntel")
    {
      isIntel_ = true;
    }
    else if (vendor_ == "AuthenticAMD")
    {
      isAMD_ = true;
    }

    // load bitset with flags for function 0x00000001
    if (nIds_ >= 1)
    {
      f_1_ECX_ = data_[1][2];
      f_1_EDX_ = data_[1][3];
    }

    // load bitset with flags for function 0x00000007
    if (nIds_ >= 7)
    {
      f_7_EBX_ = data_[7][1];
      f_7_ECX_ = data_[7][2];
    }

    // Calling __cpuid with 0x80000000 as the function_id argument
    // gets the number of the highest valid extended ID.
    __cpuid(cpui.data(), 0x80000000);
    nExIds_ = cpui[0];

    char brand[0x40];
    memset(brand, 0, sizeof(brand));

    for (int i = 0x80000000; i <= nExIds_; ++i)
    {
      __cpuidex(cpui.data(), i, 0);
      extdata_.push_back(cpui);
    }

    // load bitset with flags for function 0x80000001
    if (nExIds_ >= 0x80000001)
    {
      f_81_ECX_ = extdata_[1][2];
      f_81_EDX_ = extdata_[1][3];
    }

    // Interpret CPU brand string if reported
    if (nExIds_ >= 0x80000004)
    {
      memcpy(brand, extdata_[2].data(), sizeof(cpui));
      memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
      memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
      brand_ = brand;
    }
  };

// Initialize static member data
const CpuFeatures::CpuFeatures_Internal CpuFeatures::CPU_Rep;
