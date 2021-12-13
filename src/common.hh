#ifndef HIP_COMMON_H
#define HIP_COMMON_H

#include <map>
#include <vector>
#include <stdint.h>
#include <string>

enum class OCLType : unsigned { POD = 0, Pointer = 1, Image = 2, Sampler = 3 };

enum class OCLSpace : unsigned {
  Private = 0,
  Global = 1,
  Constant = 2,
  Local = 3,
  Unknown = 1000
};

struct OCLArgTypeInfo {
  OCLType type;
  OCLSpace space;
  size_t size;
};

struct OCLFuncInfo {
  std::vector<OCLArgTypeInfo> ArgTypeInfo;
  OCLArgTypeInfo retTypeInfo;
};

typedef std::map<int32_t, OCLFuncInfo *> OCLFuncInfoMap;

typedef std::map<std::string, OCLFuncInfo *> OpenCLFunctionInfoMap;

bool parseSPIR(int32_t *stream, size_t numWords, OpenCLFunctionInfoMap &output);

/// A prefix given to lowered global scope device variables.
constexpr char ChipVarPrefix[] = "__chip_var_";
/// A prefix used for a shadow kernel used for querying device
/// variable properties.
constexpr char ChipVarInfoPrefix[] = "__chip_var_info_";
/// A prefix used for a shadow kernel used for binding storage to
/// device variables.
constexpr char ChipVarBindPrefix[] = "__chip_var_bind_";
/// A prefix used for a shadow kernel used for initializing device
/// variables.
constexpr char ChipVarInitPrefix[] = "__chip_var_init_";
/// A structure to where properties of a device variable are written.
/// CHIPVarInfo[0]: Size in bytes.
/// CHIPVarInfo[1]: Requested alignment.
/// CHIPVarInfo[2]: Non-zero if variable has initializer. Otherwise zero.
using CHIPVarInfo = int64_t[3];

#endif
