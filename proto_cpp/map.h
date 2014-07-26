#ifndef __MAP_H__
#define __MAP_H__
#include <vector>
#include <ifstream>
#include <stdexcept>
#include <array>

namespace icfpc {

    typedef byte unsigned char;
    typedef tick unsigned long;
    using std::ifstream;
    using std::getline;
    using std::vector;
    using std::exception;
    using std::array;

    enum class Source { // anything >=Ghost1 is ghost index
        LMan=0, Ghost1
    };

    enum class Direction {
        Up = 0, Right, Down, Left
    };

    enum class GhostVitality {
        Standard, Fright, Invisible
    };

    enum class Cell {
        Wall=0, Empty, Pill, Power, Fruit, LMan, Ghost
    };

    struct Move {
        Source src;
        Direction dir;
    };

    struct Coord {
        byte x;
        byte y;
        unsigned idx(unsigned width) const {
            return (y*width)+x;
        };
        bool operator=(const Coord& other) const {
            return x == other.x && y == other.y;
        }
        bool operator<(const Coord& other) const { // ghost ordering
            if (y < other.y) return true;
            if (y > other.y) return false;
            return x < other.x;
        }
        Coord applyDir(Direction dir) {
            switch (dir) {
                case Direction::Up:   --y; break;
                case Direction::Down: ++y; break;
                case Direction::Left: --x; break;
                case Direction::Right:++x; break;
            }
            return newC;
        }
    };

    struct GhostState {
        GhostVitality vita;
        Coord pos;
        Direction dir;
        void InvertDir() {
            dir = (Direction)(((unsigned)dir+2)%4);
        }
        bool canTake(Direction newdir){
            return newdir != (Direction)(((unsigned)dir+2)%4);
        }
    };

    struct Fruit {
        Coord pos;
        tick timeLeft;
    };

    class Map {
        vector<GhostState> _ghCurrent;
        vector<Coord> _ghBase;
        Coord _lmCurrent;
        Coord _lmBase;
        byte _lmLives;

        Fruit _fruit;
        vector<Cell> _statmap;    // static map
        unsigned short _hei, _wid;
        unsigned _score;
        unsigned _opt_move;
        tick _powerPillExpiry;
        int _ghostKillIndex;

        int map_level() {
            if ((_statmap.size()%100)==0) return _statmap.size()/100;
            else return 1+(_statmap.size()/100);
        }

        unsigned fruit_score() {
            switch (map_level()) {
            case 1:
                return 100;
            case 2:
                return 300;
            case 3:
            case 4:
                return 500;
            case 5:
            case 6:
                return 700;
            case 7:
            case 8:
                return 1000;
            case 9:
            case 10:
                return 2000;
            case 11:
            case 12:
                return 3000;
            default:
                return 5000;
            }
        }
        unsigned ghost_score() {
            switch (_ghostKillIndex) {
            case 0:
                return 200;
            case 1:
                return 400;
            case 2:
                return 800;
            default:
                return 1600;
            }
        }

    public:
        // load map or DIE DIE DIE!
        Map(const string& map_file) {
            _opt_move = 7;
            _lmLives = 3;
            _wid = _hei = 0;
            ifstream ifs(map_file);
            for (string line; getline(ifs, line); ) {
                if (line.empty()) { continue; }

                if (_wid == 0) _wid = line.size();
                else if (_wid != line.size()) {
                    printf("Expected width %i; got %i (ln %i)\n", _wid, line.size(), _hei);
                    throw std::exception("Bad line: '"+line+"'");
                }

                int fIdx = _statmap.size();
                _statmap.resize(_statmap.size() + line.size());
                for(int i=0; i<line.size(); ++i) {
                    switch (line[i]) {
                    case '#': _statmap[i+fIdx] = Cell::Wall; break;
                    case '.': _statmap[i+fIdx] = Cell::Pill; break;
                    case 'o': _statmap[i+fIdx] = Cell::Power; break;
                    case ' ': _statmap[i+fIdx] = Cell::Empty; break;
                    case '%': _statmap[i+fIdx] = Cell::Empty; _fruit.pos = Coord(i, _hei); break;
                    case '\': _statmap[i+fIdx] = Cell::Empty; _lmBase = _lmCurrent = Coord(i, _hei); break;
                    case '=':
                        _statmap[i+fIdx] = Cell::Empty;
                        _ghBase.push_back(Coord(i, _hei));
                        _ghCurrent.push_back(GhostState{.vita=GhostVitality::Standard, .pos = _ghBase.back(), .dir=Direction.Down});
                        break;
                    default:
                        printf("Unexpected symbol 0x%x(%c) in ln %i\n", line[i], line[i], _hei);
                        throw std::exception("Bad symbol in line: '"+line+"'");
                    }
                }

                ++_hei;
            }
        }
        byte lives() const { return _lmLives; }
        unsigned mapSize() const { return _wid * _hei; }
        const vector<Coord> ghostData() const { return _ghBase.size(); }
        void adjustScore() { _score*=_lmLives; };

        // main cycle functions
        bool cleanedUp() {
            for (const Cell& i:_statmap) {
                if (i == Cell::Pill) return false;
            }
            return true;
        }

        void effects() {
            unsigned lmPos = _lmCurrent.idx(_wid);

            if (_opt_move & 1 ) { // LMan moved during this tick - check cell & power
                Cell = _statmap[lmPos];
                if (Cell == Cell::Power) {
                    powerPillExpiry = 127*20;
                    _ghostKillIndex = 0;
                    score += 50;
                    _statmap[lmPos] = Cell::Empty;
                    for (GhostState &ghr:_ghCurrent) {
                        ghr.vita=GhostVitality::Standard;
                        ghr.pos=_ghBase[i];
                        ghr.InvertDir();
                    }
                }
                if (Cell == Cell::Pill) {
                    score += 10;
                    _statmap[lmPos] = Cell::Empty;
                }
            }
            if (_opt_move & 3 ) { // LMan or ghosts moved
                for ( int i=0; i<_ghCurrent.size(); ++i) {
                    GhostState& gh = _ghCurrent[i];
                    if (gh.pos == _lmCurrent) {
                        if (powerPillExpiry || gh.vita == GhostVitality::Fright) { // ghost eaten
                            gh.vita=GhostVitality::Invisible;
                            gh.pos=_ghBase[i];
                            gh.dir = Direction::Down;
                            score += ghost_score();
                        } else if (gh.vita == GhostVitality::Standard) { // pacman eaten - reset all
                            _lmCurrent = _lmBase;
                            --_lmLives;
                            for (GhostState &ghr:_ghCurrent) {
                                ghr.vita=GhostVitality::Standard;
                                ghr.pos=_ghBase[i];
                                ghr.dir = Direction::Down;
                            }
                            _opt_move = 0; // nothing more to check.
                            break;
                        }
                    }
                }
            }
            if (_opt_move & 5 ) { // LMan moved during this tick OR fruit appeared - check fruits
                if (_fruit.pos == _lmCurrent && _fruit.timeLeft != 0) {
                    _fruit.timeLeft = 0;
                    score += fruit_score();
                }
            }
            _opt_move = 0;
        };

        // timer effects
        void apply(const tick ticker) {
            --_powerPollExpiry;
            if (_powerPillExpiry != 0) {
                for (GhostState &ghr:_ghCurrent) {
                    ghr.vita = GhostVitality::Standard;
                }
                _opt_move |= 2;
            }
            if (ticker == 127*200 || ticker == 127*400) {
                _fruit.timeLeft = 127*80;
                _opt_move |= 4;
            } else {
                if (_fruit.timeLeft) {
                    --_fruit.timeLeft;
                }
            }
        }

        unsigned move(const Move& move) { // returns delta to next move
            if (move.src == Source::LMan) {
                Coord newC = _lmCurrent;
                newC.applyDir(move.dir);
                if (_statmap[newC.idx(_wid)] != Cell::Wall){
                    _lmCurrent = newC;
                    _opt_move |= 1;
                    if (_statmap[newC.idx(_wid)] == Cell::Empty) {
                        return 127;
                    }
                    return 137;
                }
                return 127; // 137? never?
            } else {
                unsigned ghIdx = (unsigned)move.src - (unsigned)Source::Ghost1;
                // test order: move, last, 
                Direction realDir;
                array<Direction, 6> allDirs = {move.dir, _ghCurrent[ghIdx].dir, Direction::Up, Direction::Right, Direction::Down, Direction::Left};
                
                for (Direction d:allDirs) {
                    if (canMoveGhost(_ghCurrent[ghIdx], d)) {
                        realDir = d;
                        break;
                    }
                }

                _ghCurrent[ghIdx].pos.applyDir(realDir);
                _ghCurrent[ghIdx].dir = realDir;
                } else { // ghosts STILL must roll!
                    
                }
                _opt_move |= 2;
                return (65 + (ghIdx%4)) * ((_powerPillExpiry == 0)?2:3); // вот так вот хитро мы указали ффсе константы...
            }
        }
    private:
        bool canMoveGhost(const GhostState & gs, Direction dir) const {
            Coord newC = gs.pos;
            newC.applyDir(dir);
            return _statmap[newC.idx(_wid)] != Cell::Wall && gs.canTake(dir);
        }
    }
}

#endif