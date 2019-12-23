#include "./binary.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

/*struct C0_binary_file {
    u4              magic; // must be 0x43303A29
    u4              version;
    u2              constants_count;
    Constant_info   constants[constants_count];
    Start_code_info start_code;
    u2              functions_count;
    Function_info   functions[functions_count];
};*/

namespace c0 {
    void Binary::writeNBytes (std::ostream &out, void *addr, int count) {
        char bytes[8] = {0};
        char *p = (char*)addr + (count - 1);
        for (int i = 0; i < count; ++i) {
            bytes[i] = *p--;
        }
        out.write(bytes, count);
    }

    void Binary::to_binary (std::ostream &out, const std::vector<c0::Instruction> &v) {
        vm::u2 instructions_count = v.size();
        writeNBytes(out, &instructions_count, sizeof instructions_count);

        for (auto &ins : v) {
            vm::u1 op = static_cast<vm::u1>(ins.getOpr());
            writeNBytes(out, &op, sizeof(vm::u1));

            auto it = oprPara.find(ins.getOpr());
            if ( it != oprPara.end()) {
                auto paras = it->second;

                if (paras[0] == 1) {
                    vm::u1 x = ins.getX();
                    writeNBytes(out, &x, 1);
                } else if (paras[0] == 2) {
                    vm::u2 x = ins.getX();
                    writeNBytes(out, &x, 2);
                } else if (paras[0] == 4) {
                    vm::u4 x = ins.getX();
                    writeNBytes(out, &x, 4);
                }

                if (paras.size() == 2) {
                    if (paras[1] == 1) {
                        vm::u1 y = ins.getY();
                        writeNBytes(out, &y, 1);
                    } else if (paras[1] == 2) {
                        vm::u2 y = ins.getY();
                        writeNBytes(out, &y, 2);
                    } else if (paras[1] == 4) {
                        vm::u4 y = ins.getY();
                        writeNBytes(out, &y, 4);
                    }
                }
            }
        }
    }

    void Binary::output_binary(std::ofstream &out) {
        // magic
        out.write("\x43\x30\x3A\x29", 4);
        // version
        out.write("\x00\x00\x00\x01", 4);
        // constants_count
        vm::u2 constants_count = _constants.size();
        writeNBytes(out, &constants_count, sizeof(vm::u2));
        // constants
        for (auto &pair : _constants) {
            out.write("\x00", 1);
            std::string s = pair.second;
            vm::u2 len = s.length();
            writeNBytes(out, &len, sizeof len);
            out.write(s.c_str(), len);
        }

        // start
        to_binary(out, _start);

        // functions_count
        vm::u2 functions_count = _functions.size();
        writeNBytes(out, &functions_count, sizeof(vm::u2));
        // functions
        for (int i = 0; i < (int)_functions.size(); i++) {
            auto fun = _functions[i];
            vm::u2 n;
            n = fun.name_index;
            writeNBytes(out, &n, sizeof(vm::u2));
            n = fun.params_size;
            writeNBytes(out, &n, sizeof(vm::u2));
            n = fun.level;
            writeNBytes(out, &n, sizeof(vm::u2));

            to_binary(out, _instructions[i]);
        }
    }
}