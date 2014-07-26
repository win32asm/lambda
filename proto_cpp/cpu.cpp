#include "gcc.h"
#include "ghc.h"
#include "map.h"

#include <unistd.h>
#include <vector>
#include <string>

namespace icfpc
{

    typedef byte unsigned char;
    typedef tick unsigned long;

    enum class Source   // anything >=Ghost1 is ghost index
    {
        LMan=0, Ghost1
    };

    enum class Direction
    {
        Up = 0, Right, Down, Left
    };

    enum class GhostVitality
    {
        Standard, Fright, Invisible
    };

    enum class Cell
    {
        Wall=0, Empty, Pill, Power, Fruit, LMan, Ghost
    };

    struct Move
    {
        Source src;
        Direction dir;
        tick time;
    };

    struct Coord
    {
        byte x;
        byte y;
        unsigned idx(unsigned width) const
        {
            return (y*width)+x;
        };
        bool operator=(const Coord& other) const
        {
            return x == other.x && y == other.y;
        }
        bool operator<(const Coord& other) const   // ghost ordering
        {
            if (y < other.y) return true;
            if (y > other.y) return false;
            return x < other.x;
        }
    };

    struct GhostState
    {
        GhostVitality vita;
        Coord pos;
        Direction dir;
        void InvertDir()
        {
            dir = (Direction)(((unsigned)dir+2)%4);
        }
    };

    struct Fruit
    {
        Coord pos;
        tick timeLeft;
    };

    class Map
    {
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

        int map_level()
        {
            if ((_statmap.size()%100)==0) return _statmap.size()/100;
            else return 1+(_statmap.size()/100);
        }

        unsigned fruit_score()
        {
            switch (map_level())
                {
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
        unsigned ghost_score()
        {
            switch (_ghostKillIndex)
                {
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
        Map(const string& map_file)
        {
            _opt_move = 7;
            _lmLives = 3;
        }
        byte lives() const
        {
            return _lmLives;
        }
        unsigned mapSize() const
        {
            return _wid * _hei;
        }
        const vector<Coord> ghostData() const
        {
            return _ghBase;
        }
        void adjustScore()
        {
            _score*=_lmLives;
        };

        // main cycle functions
        bool cleanedUp()
        {
            for (const Cell& i:_statmap)
                {
                    if (i == Cell::Pill) return false;
                }
            return true;
        }

        void effects()
        {
            unsigned lmPos = _lmCurrent.idx(_wid);

            if (_opt_move & 1 )   // LMan moved during this tick - check cell & power
                {
                    Cell = _statmap[lmPos];
                    if (Cell == Cell::Power)
                        {
                            powerPillExpiry = 127*20;
                            _ghostKillIndex = 0;
                            score += 50;
                            _statmap[lmPos] = Cell::Empty;
                            for (GhostState &ghr:_ghCurrent)
                                {
                                    ghr.vita=GhostVitality::Standard;
                                    ghr.pos=_ghBase[i];
                                    ghr.InvertDir();
                                }
                        }
                    if (Cell == Cell::Pill)
                        {
                            score += 10;
                            _statmap[lmPos] = Cell::Empty;
                        }
                }
            if (_opt_move & 3 )   // LMan or ghosts moved
                {
                    for ( int i=0; i<_ghCurrent.size(); ++i)
                        {
                            GhostState& gh = _ghCurrent[i];
                            if (gh.pos == _lmCurrent)
                                {
                                    if (powerPillExpiry || gh.vita == GhostVitality::Fright)   // ghost eaten
                                        {
                                            gh.vita=GhostVitality::Invisible;
                                            gh.pos=_ghBase[i];
                                            gh.dir = Direction::Down;
                                            score += ghost_score();
                                        }
                                    else if (gh.vita == GhostVitality::Standard)     // pacman eaten - reset all
                                        {
                                            _lmCurrent = _lmBase;
                                            --_lmLives;
                                            for (GhostState &ghr:_ghCurrent)
                                                {
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
            if (_opt_move & 5 )   // LMan moved during this tick OR fruit appeared - check fruits
                {
                    if (_fruit.pos == _lmCurrent && _fruit.timeLeft != 0)
                        {
                            _fruit.timeLeft = 0;
                            score += fruit_score();
                        }
                }
            _opt_move = 0;
        };

        void apply(const tick ticker)
        {
            --_powerPollExpiry;
            if (_powerPillExpiry != 0)
                {
                    for (GhostState &ghr:_ghCurrent)
                        {
                            ghr.vita = GhostVitality::Standard;
                        }
                    _opt_move |= 2;
                }
            if (ticker == 127*200 || ticker == 127*400)
                {
                    _fruit.timeLeft = 127*80;
                    _opt_move |= 4;
                }
            else
                {
                    if (_fruit.timeLeft)
                        {
                            --_fruit.timeLeft;
                        }
                }
        }
    }
}

namespace icfpc
{
    class Ghosts
    {
    public:

    };
}

namespace icfpc
{
    class LMan
    {
    public:

    };
}

using std::vector;
using std::string;

using icfpc; // всё в этот неймспейс!

int main(int argc, const char *argv[])
{
    vector<string> ghost_f;
    string map_f;
    string lman_f;
    bool compileOnly=false;
    while ((opt = getopt(argc, argv, "m:g:l:")) != -1)
        {
            switch (opt)
                {
                case 'm':
                    map_f = string(optarg);
                    break;
                case 'g':
                    ghost_f.push_back(string(optarg));
                    break;
                case 'l':
                    lman_f = string(optarg);
                    break;
                case 'c':
                    compileOnly = true;
                    break;
                }
        }

    LMan theLMan(lman_f);

    if (compileOnly)
        {
            theLMan.compile();
            return 0;
        }

    Map theMap(map_f);
    Ghosts theGhosts(ghost_f, theMap.ghostData());

    map<tick, vector<Move>> theMoves;
    tick ticker=0;
    do
        {
            // #1 - all moves take place.
            theMap.move(theMoves);
            // #1.1 - add new moves if necessary
            theGhosts.nextMove(ticker, theMoves, theMap);
            theLMan.nextMove(ticker, theMoves, theMap);

            // #2 - timed actions.
            theGhosts.apply(ticker);
            theMap.apply(ticker);

            // #3 - effects. #4 - ghost processing
            theMap.effects();

            // #5 - win condition
            if (theMap.cleanedUp())
                {
                    theMap.adjustScore();
                    printf("Winner\n");
                    break;
                }

            // #6 - lose conditions
            if ( theMap.lives() == 0 )
                {
                    printf("Dead\n");
                    break;
                }
            if ( ticker == 127*theMap.mapSize()*16 )
                {
                    printf("Time out\n");
                    break;
                }

            // # inc ticks
            ++ticker;
        }
    while (true);

    printf("Final score: %u; %lu ticks\n", theMap.score(), ticker);
    return 0;
}
