#ifndef __GHC_H__
#define __GHC_H__

#include <ifstream>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <cctype>

namespace icfpc {

    using std::vector;
    using std::array;
    using std::string;
    using std::exception;

    typedef byte unsigned char;

    class GhostPU {

        enum class opRestr {
            None = 0, notImm=1, notReg=2, notRef=4, notPC=8
        }

        struct arg { // !isReg -> imm; !isReg&isInd -> [imm]
            bool isReg;
            bool isInd;
            byte data;
            string operator() {
                string res;
                if (isReg) {
                    if (data == 255) res="pc";
                    else res = 'a' + data;
                }
                else {char temp[10]; snprintf(temp,10,"%i", data); res = temp;}
                if (isInd) res = '['+res+']';
                return res;
            }
        };

        enum class GPUop{
            mov_=1, inc_, dec_, add_, sub_, mul_, div_, and_, or_, xor_, jlt_, jeq_, jgt_, int_, hlt_
        };

        class GPUInstr {
            GPUop instr;
            arg arg1;
            arg arg2;
            byte addr; // if needed;

            byte& op(arg& a, GhostPU& ref) const {
                if (a.isReg) {
                    if (a.data == 255)  return a.isInd?ref._data[ref.PC]:ref.PC;
                    else                return a.isInd?ref._data[ref._regs[a.data]]:ref._regs[a.data];
                }
                return a.isInd?ref._data[a.data]:a.data;
            }
            byte& op1(GhostPU& ref)const {return op(arg1, ref);}
            byte& op2(GhostPU& ref)const {return op(arg2, ref);}
            string addr_s(){char temp[10]; snprintf(temp,10,"%i", addr); return string(temp);}
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
            string print() {
                switch (instr) {
                    case GPUop::mov_: return string("mov ")+ arg1()+"," +arg2();
                    case GPUop::inc_: return string("inc ")+ arg1();
                    case GPUop::dec_: return string("dec ")+ arg1();
                    case GPUop::add_: return string("add ")+ arg1()+"," +arg2();
                    case GPUop::sub_: return string("sub ")+ arg1()+"," +arg2();
                    case GPUop::mul_: return string("mul ")+ arg1()+"," +arg2();
                    case GPUop::div_: return string("div ")+ arg1()+"," +arg2();
                    case GPUop::and_: return string("and ")+ arg1()+"," +arg2();
                    case GPUop::or_:  return  string("or ")+ arg1()+"," +arg2();
                    case GPUop::xor_: return string("xor ")+ arg1()+"," +arg2();
                    case GPUop::jlt_: return string("jlt ")+ addr_s()+","+ arg1()+"," +arg2();
                    case GPUop::jeq_: return string("mov ")+ addr_s()+","+ arg1()+"," +arg2();
                    case GPUop::jgt_: return string("mov ")+ addr_s()+","+ arg1()+"," +arg2();
                    case GPUop::int_: return string("int ")+ addr_s();
                    default: return string("hlt");
            }
        };


        array<GPUInstr, 256> _code;
        array<byte, 256> _data;
        array<byte, 8> _regs;
        byte PC;
        byte _ghostId;

        string parseto(const string& str, string::size_type& startidx, char ch) const {
            startidx = line.find(ch, startidx);
            if (startidx != string::npos) {
                string::size_type from=0, to=startidx;
                while (from < to && isspace(str[from])) ++from;
                while (to > from && isspace(str[to]))   --to;
                ++startidx;
                if (to == from) return string("");
                return str.substr(from, to-from);
            }
            return str;
        }

        arg parseArg(const string& _str, opRestr restr = opRestr::None) {
            arg out;
            string str;

            if (_str[0] == "@") {out.isInd = true; str=_str.substr(1); }
            else str = _str;

            if (str.size() == 1 && tolower(str[0]) >= 'a' && tolower(str[0]) <= 'h') {
                out.isReg = true;
                out.data = tolower(str[0])-'a';
            } else {
                if (str == "pc" || str == "PC") {
                    out.isReg = true;
                    out.data = 255;
                } else {
                    out.isReg = false;
                    auto isLabel = _datarefs.find(str); 
                    if (isLabel != _datarefs.end()) out.data = *isLabel;
                    else {
                        string::size_type pos = string:npos;
                        unsigned data = stoul(str, &pos, 0);
                        if (pos != str.size() && pos != string::npos || data > 255) {
                            throw std::exception("failed to parse argument '"+_str+"'");
                        }
                    }
                }
            }
            if ((restr & opRestr::notImm && !arg.isReg && !arg.isInd)||
                (restr & opRestr::notReg &&  arg.isReg && !arg.isInd)||
                (restr & opRestr::notRef &&  arg.isInd) || 
                (restr & opRestr::notPC  &&  arg.isReg && arg.data == 255)) {
                throw std::exception("Argument restrictions failed '"+_str+"'");
            }
            return out;
        }

        byte parseConst(const string& str, byte coderef) {
            auto isLabel = _coderefs.find(str);
            if (isLabel != _coderefs.end()) return *isLabel;

            string::size_type pos = string:npos;
            unsigned data = stoul(str, &pos, 0);
            if (pos == str.size() && data < 256) {
                return data;
            }

            _fwdrefs[str].push_back(coderef);
            return 0;
        }

        bool parseline(string& line, byte addr) {
            string::size_type idx = 0;
            string nocomments = parseto(line, idx, ';');
            idx = 0;
            string label = parseto(nocomments, idx, ':');
            if (idx != string::npos) {
                if (label.find_first_of("\n\t\r ",0,4) != string::npos) {
                    throw std::exception("Label '"+label+"' must not have spaces");
                }
                auto frfs = _fwdrefs.find(label);
                if (frfs != _fwdrefs.end()) {
                    for (byte b:frfs) {
                        _code[b].addr = _code.size();
                    }
                    _fwdrefs.erase(frfs);
                }
                _coderefs[label] = addr;
            } else {
                idx = 0;
            }
            string instr=parseto(nocomments, idx, ' ');
            if (instr.size() == 0) return false;

            for_each(instr.begin(), instr.end(), [](char& c){ c = ::tolower(c); });
            if (instr == "db") {
                string dbname = parseto(nocomments, idx, ' ');
                if (dbname.size() == 0) throw std::exception("db declaration must have a name");
                _datarefs[dbname] = (byte)_datarefs.size();
            } else if (instr == "mov") {
                _code[addr].instr = GPUop::mov;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "inc") {
                _code[addr].instr = GPUop::inc;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
            } else if (instr == "dec") {
                _code[addr].instr = GPUop::dec;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
            } else if (instr == "add") {
                _code[addr].instr = GPUop::add;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "sub") {
                _code[addr].instr = GPUop::sub;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "mul") {
                _code[addr].instr = GPUop::mul;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "div") {
                _code[addr].instr = GPUop::div;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "and") {
                _code[addr].instr = GPUop::and;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "or") {
                _code[addr].instr = GPUop::or;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "xor") {
                _code[addr].instr = GPUop::xor;
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','), opRestr::notImm | opRestr::notPC);
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "hlt") {
                _code[addr].instr = GPUop::hlt;
            } else if (instr == "jlt") {
                _code[addr].instr = GPUop::jlt;
                _code[addr].addr  = parseConst(parseto(nocomments, idx, ','), addr);
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','));
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "jgt") {
                _code[addr].instr = GPUop::jgt;
                _code[addr].addr  = parseConst(parseto(nocomments, idx, ','), addr);
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','));
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "jeq") {
                _code[addr].instr = GPUop::jeq;
                _code[addr].addr  = parseConst(parseto(nocomments, idx, ','), addr);
                _code[addr].arg1  = parseArg(parseto(nocomments, idx, ','));
                _code[addr].arg2  = parseArg(parseto(nocomments, idx, ','));
            } else if (instr == "int") {
                _code[addr].instr = GPUop::int;
                _code[addr].addr  = parseConst(parseto(nocomments, idx, ','), 255);
            } else {
                throw std::exception(instr+ " is not an instruction!");
            }
            return true;
        }
        map<string, list<byte>> _fwdrefs;
        map<string, byte> _datarefs;
        map<string, byte> _coderefs;

    public:
        Direction _dir;
        GhostPU(const string& program, byte gid) { // implement loading
            _ghostId = gid;
            //load code
            int lineidx = 0;
            byte iidx = 0;
            for (string line; getline(ifs, line); ) {
                if (line.empty()) { continue; }
                if (parseLine(line, iidx)) {
                    ++iidx;
                }
                ++idx;
            }
            if (_fwdrefs.size() != 0) {
                throw new exception("reference to label "+ _fwdrefs.begin()->first +"not provided");
            }
        }
        void ResetPC(Direction dir){PC = 0; _dir = dir;}
        bool exec(const Map& theMap){return _code[PC].exec(*this, theMap); }
        void compile() const {
            printf("Ghost program %i\n------------------------\n", _ghostId);
            for (const GPUInstr& gpi:_code) {
                printf("%s\n", gpi.print().c_str());
            }
        }
    };

    class Ghosts {
        vector<GhostPU> _gpus;
    public:
        Ghosts(vector<string> pgms, int NGhosts) {
            for (int i=0; i<NGhosts; ++i) {
                _gpus.push_back(GhostPU(pgms%4, i));
            }
        }

        void compile() {
            for (int i=0; i<4 && i<_gpus.size(); ++i) {
                _gpus[i].compile();
            }
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