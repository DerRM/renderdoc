#pragma once

#include "shader/usse_types.h"

#include <fstream>
#include <memory>
#include <string>

#include <common/common.h>

namespace shader
{
namespace usse
{
namespace disasm
{
extern std::string *disasm_storage;

//
// Disasm helpers
//

const std::string &opcode_str(const Opcode &e);
const char *e_predicate_str(ExtPredicate p);
const char *s_predicate_str(ShortPredicate p);
const char *data_type_str(DataType p);
std::string reg_to_str(RegisterBank bank, uint32_t reg_num);
std::string operand_to_str(Operand op, Imm4 write_mask, int32_t shift = 0);
template <std::size_t s>
std::string swizzle_to_str(Swizzle<s> swizz, const Imm4 write_mask);

}    // namespace disasm
}    // namespace usse
}    // namespace shader

// TODO: make LOG_RAW
#define LOG_DISASM(fmt_str, ...)                            \
  do                                                        \
  {                                                         \
    char buff[256];                                         \
    snprintf(buff, sizeof(buff), fmt_str, __VA_ARGS__);     \
    RDCLOG("%s", buff);                                     \
    if(shader::usse::disasm::disasm_storage)                \
      *shader::usse::disasm::disasm_storage += std::string(buff) + '\n'; \
  } while(0)
