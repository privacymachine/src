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
#pragma once

#ifdef PM_WINDOWS
  #include <string>
  #include <iostream>
  #include <vector>
  #include <bitset>
  #include <array>
  #include <intrin.h>
#endif

/// Determine CPU-Features i.e. "VT-x/AMD-V"-Support
/// Current status: works only under Linux
class CpuFeatures
{
  // forward declarations
  class CpuFeatures_Internal;

  public:
    // getters
    static bool Virtualization(void);
    
    #ifdef PM_WINDOWS
    static std::string Vendor(void) { return CPU_Rep.vendor_; }  
    static std::string Brand(void) { return CPU_Rep.brand_; }  
  
    static bool SSE3(void) { return CPU_Rep.f_1_ECX_[0]; }  
    static bool PCLMULQDQ(void) { return CPU_Rep.f_1_ECX_[1]; }  
    static bool MONITOR(void) { return CPU_Rep.f_1_ECX_[3]; }  
    static bool SSSE3(void) { return CPU_Rep.f_1_ECX_[9]; }  
    static bool FMA(void) { return CPU_Rep.f_1_ECX_[12]; }  
    static bool CMPXCHG16B(void) { return CPU_Rep.f_1_ECX_[13]; }  
    static bool SSE41(void) { return CPU_Rep.f_1_ECX_[19]; }  
    static bool SSE42(void) { return CPU_Rep.f_1_ECX_[20]; }  
    static bool MOVBE(void) { return CPU_Rep.f_1_ECX_[22]; }  
    static bool POPCNT(void) { return CPU_Rep.f_1_ECX_[23]; }  
    static bool AES(void) { return CPU_Rep.f_1_ECX_[25]; }  
    static bool XSAVE(void) { return CPU_Rep.f_1_ECX_[26]; }  
    static bool OSXSAVE(void) { return CPU_Rep.f_1_ECX_[27]; }  
    static bool AVX(void) { return CPU_Rep.f_1_ECX_[28]; }  
    static bool F16C(void) { return CPU_Rep.f_1_ECX_[29]; }  
    static bool RDRAND(void) { return CPU_Rep.f_1_ECX_[30]; }  
  
    static bool MSR(void) { return CPU_Rep.f_1_EDX_[5]; }  
    static bool CX8(void) { return CPU_Rep.f_1_EDX_[8]; }  
    static bool SEP(void) { return CPU_Rep.f_1_EDX_[11]; }  
    static bool CMOV(void) { return CPU_Rep.f_1_EDX_[15]; }  
    static bool CLFSH(void) { return CPU_Rep.f_1_EDX_[19]; }  
    static bool MMX(void) { return CPU_Rep.f_1_EDX_[23]; }  
    static bool FXSR(void) { return CPU_Rep.f_1_EDX_[24]; }  
    static bool SSE(void) { return CPU_Rep.f_1_EDX_[25]; }  
    static bool SSE2(void) { return CPU_Rep.f_1_EDX_[26]; }  
  
    static bool FSGSBASE(void) { return CPU_Rep.f_7_EBX_[0]; }  
    static bool BMI1(void) { return CPU_Rep.f_7_EBX_[3]; }  
    static bool HLE(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4]; }  
    static bool AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }  
    static bool BMI2(void) { return CPU_Rep.f_7_EBX_[8]; }  
    static bool ERMS(void) { return CPU_Rep.f_7_EBX_[9]; }  
    static bool INVPCID(void) { return CPU_Rep.f_7_EBX_[10]; }  
    static bool RTM(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11]; }  
    static bool AVX512F(void) { return CPU_Rep.f_7_EBX_[16]; }  
    static bool RDSEED(void) { return CPU_Rep.f_7_EBX_[18]; }  
    static bool ADX(void) { return CPU_Rep.f_7_EBX_[19]; }  
    static bool AVX512PF(void) { return CPU_Rep.f_7_EBX_[26]; }  
    static bool AVX512ER(void) { return CPU_Rep.f_7_EBX_[27]; }  
    static bool AVX512CD(void) { return CPU_Rep.f_7_EBX_[28]; }  
    static bool SHA(void) { return CPU_Rep.f_7_EBX_[29]; }  
  
    static bool PREFETCHWT1(void) { return CPU_Rep.f_7_ECX_[0]; }  
  
    static bool LAHF(void) { return CPU_Rep.f_81_ECX_[0]; }  
    static bool LZCNT(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_ECX_[5]; }  
    static bool ABM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[5]; }  
    static bool SSE4a(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6]; }  
    static bool XOP(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11]; }  
    static bool TBM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21]; }  
  
    static bool SYSCALL(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[11]; }  
    static bool MMXEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22]; }  
    static bool RDTSCP(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27]; }  
    static bool _3DNOWEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30]; }  
    static bool _3DNOW(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31]; }  
    #endif
    
  private:  
    static const CpuFeatures_Internal CPU_Rep; 

    class CpuFeatures_Internal
    {
      public:
        CpuFeatures_Internal();

        bool hasVirtualizationSupport_;
        #ifdef PM_WINDOWS
        int nIds_;
        int nExIds_;
        std::string vendor_;
        std::string brand_;
        bool isIntel_;
        bool isAMD_;
        std::bitset<32> f_1_ECX_;
        std::bitset<32> f_1_EDX_;
        std::bitset<32> f_7_EBX_;
        std::bitset<32> f_7_ECX_;
        std::bitset<32> f_81_ECX_;
        std::bitset<32> f_81_EDX_;
        std::vector<std::array<int, 4>> data_;
        std::vector<std::array<int, 4>> extdata_;
        #endif
    };

};
