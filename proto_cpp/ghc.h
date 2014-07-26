#ifndef __GHC_H__
#define __GHC_H__

#include <ifstream>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>

namespace icfpc {

    using std::vector;
    using std::array;
    using std::string;
    using std::exception;

    typedef byte unsigned char;

    class GhostPU {

        struct arg { // !isReg -> imm; !isReg&isInd -> [imm]
            bool isReg;
            bool isInd;
            byte data;
        };

        enum class GPUop{
            mov_, inc_, dec_, add_, sub_, mul_, div_, and_, or_, xor_, jlt_, jeq_, jgt_, int_, hlt_
        };

        class GPUInstr {
            GPUop instr;
            arg arg1;
            arg arg2;
            byte addr; // if needed;

            byte& op(arg& a, GhostPU& ref) const {
                if (a.isReg) { return a.isInd?ref._data[ref._regs[a.data]]:ref._regs[a.data]; }
                return a.isInd?ref._data[a.data]:a.data;
            }
            byte& op1(GhostPU& ref)const {return op(arg1, ref);}
            byte& op2(GhostPU& ref)const {return op(arg2, ref);}
        public:
            bool exec(GhostPU& ref, const Map& map) {
                switch (instr) {
                    case GPUop::mov_: op1(ref) = op2(ref); break;
                    case GPUop::inc_: ++op1(ref); break;
                    case GPUop::dec_: --op1(ref); break;
                    case GPUop::add_: op1(ref)+=op2(ref); break;
                    case GPUop::sub_: op1(ref)-=op2(ref); break;
                    case GPUop::mul_: op1(ref)*=op2(ref); break;
                    case GPUop::div_: op1(ref)/=op2(ref); break;
                    case GPUop::and_: op1(ref)&=op2(ref); break;
                    case GPUop::or_:  op1(ref)|=op2(ref); break;
                    case GPUop::xor_: op1(ref)^=op2(ref); break;
                    case GPUop::jlt_: if (op1(ref) < op2(ref)) { ref.PC = addr; return true;} break;
                    case GPUop::jeq_: if (op1(ref) == op2(ref)){ ref.PC = addr; return true;} break;
                    case GPUop::jgt_: if (op1(ref) > op2(ref)) { ref.PC = addr; return true;} break;
                    case GPUop::int_: processInt(addr, ref, map); break;
                    case GPUop::hlt_: return false;
                    default:
                        throw new std::exception("Unknown instruction!");
                }
                ++ref.PC;
                return true;
            }
            void processInt(byte id, GhostPU& ref, const Map& map){
                switch (id) {
                    case 0: if (ref._regs[0] < 4) { ref._dir = (Direction)(ref._regs[0]); } break;
                    case 1: ref._regs[0] = map._lmCurrent.x; ref._regs[1] = map._lmCurrent.y; break;
                    case 2: throw new std::exception("Can not get 2nd lman coords!");
                    case 3: ref._regs[0] = ref._ghostId; break;
                    case 4:
                        if (ref._regs[0] < map._ghBase.size()) {
                            ref._regs[0] = map._ghBase[ref._regs[0]].x;
                            ref._regs[1] = map._ghBase[ref._regs[0]].y;
                        }
                        break;
                    case 5:
                        if (ref._regs[0] < map._ghCurrent.size()) {
                            ref._regs[0] = map._ghCurrent[ref._regs[0]].pos.x;
                            ref._regs[1] = map._ghCurrent[ref._regs[0]].pos.y;
                        }
                        break;
                    case 6:
                        if (ref._regs[0] < map._ghCurrent.size()) {
                            ref._regs[0] = (byte)map._ghCurrent[ref._regs[0]].vita;
                            ref._regs[1] = (byte)map._ghCurrent[ref._regs[0]].dir;
                        }
                        break;
                    case 7:
                        if (ref._regs[0] < map._wid && ref._regs[0] < map._hei) {
                            ref._regs[0] = (byte)map._statmap[ref._regs[0] + ref._regs[1]*map._wid];
                        } else {
                            ref._regs[0] = 0;
                        }
                        break;
                    default:
                        throw new std::exception("Unknown interrupt!");
                }
            }
        };


        array<GPUInstr, 256> _code;
        array<byte, 256> _data;
        array<byte, 8> _regs;
        byte PC;
        byte _ghostId;

    public:
        Direction _dir;
        GhostPU(const string& program, byte gid) { // implement loading
        }
        void ResetPC(Direction dir){PC = 0; _dir = dir;}
        bool exec(const Map& theMap){return _code[PC].exec(*this, theMap); }
    };

    class Ghosts {
        vector<GhostPU> _gpus;
    public:
        Ghosts(vector<string> pgms, int NGhosts) {
        }
        //schedNextMove(mv.Source, theMoves[ticker+nextDelta], theMap);
        void schedNextMove(unsigned ghIdx, vector<Move> &moves, const Map& theMap) {
            Move nextMove;
            _gpus[ghIdx].ResetPC(theMap._ghCurrent[ghIdx].dir);
            for (int gpTick=0; gpTick <1024; ++gpTick) {
                if (!_gpus[ghIdx].exec(theMap)) break;
            }
            nextMove.src = (Source)ghIdx;
            nextMove.dir = _gpus[ghIdx]._dir;
            moves.push_back(nextMove);
        }
    };
}

#endif