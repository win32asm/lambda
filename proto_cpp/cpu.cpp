#include "gcc.h"
#include "ghc.h"
#include "map.h"

#include <unistd.h>
#include <vector>
#include <string>

namespace icfpc {

    typedef byte unsigned char;
    typedef tick unsigned long;

    enum class Effects:unsigned { // bitmap
	Power=1,	//got power pill, fright mode & counters must be reset
	Reserved=14
    };  // anything (&(~0xf))>>4 is a ghost overlap mask

    enum class Source { // anything >=Ghost1 is ghost index
	LMan=0, Ghost1
    };

    enum class Direction {
	Up = 0, Right, Down, Left
    };

    enum class Cell {
	Wall=0, Empty, Pill, Power, Fruit, LMan, Ghost
    };

    struct Move {
	Source src;
	Direction dir;
	tick time;
    };

    struct Coord {
	byte x;
	byte y;
	bool operator=(const Coord& other) {
	    return x == other.x && y == other.y;
	}
	bool operator<(const Coord& other) { // ghost ordering
	    if (y < other.y) return true;
	    if (y > other.y) return false;
	    return x < other.x;
	}
    };

    struct Fruit {
	Coord pos;
	tick timeLeft;
    };

    class Map {
	vector<Coord> _current;
	vector<Coord> _base;
	vector<Fruit> _fruits;
	vector<Cell> _statmap;    // static map
	unsigned short _hei, _wid;
	unsigned _score;

	unsigned _opt_move;

	int map_level() {
	    if ((_statmap.size()%100)==0) return _statmap.size()/100;
	    else return 1+(_statmap.size()/100);
	}

	int fruit_score() {
	    switch (map_level()) {
		case 1: return 100;
		case 2: return 300;
		case 3:
		case 4: return 500;
		case 5:
		case 6:	return 700;
		case 7:
		case 8: return 1000;
		case 9:
		case 10: return 2000;
		case 11:
		case 12: return 3000;
		default: return 5000;
	    }
	}
    public:
	// load map or DIE DIE DIE!
	Map(const string& map_file) { _opt_move = 7; }
	//
	vector<Coord> ghostData() const {return vector<Coord>(_current.begin()+1,_current.end());}
	void adjustScore(byte x) {_score*=x;};

	// main cycle functions
	bool cleanedUp() {
	    for (const Cell& i:_statmap) {
		if (Cell==Pill) return false;
	    }
	    return true;
	}

	Effects effects() {
	    unsigned fx=0;
	    if (_opt_move & 1) { // LMan moved during this tick - check cell
		
	    }
	    if (_opt_move & 5) { // LMan moved during this tick OR fruit appeared - check fruits
		for (int i=0; i<_fruits.size(); ++i) {
		    if (_fruits[i] == _current[0]) {
			_fruits.erase(_fruits.begin()+i);
			fx |= (unsigned)Effects::Power;
			score += 
		    }
		}
	    }
	    if (_opt_move & 2) { // ghosts moved
	    }
	}

    };
}

namespace icfpc {
    class Ghosts {
    public:
	
    };
}

namespace icfpc {
    class LMan {
    public:
	
    };
}

using std::vector;
using std::string;

using icfpc; // всё в этот неймспейс! 

int main(int argc, const char *argv[]) {
    vector<string> ghost_f;
    string map_f;
    string lman_f;
    bool compileOnly=false;
    while ((opt = getopt(argc, argv, "m:g:l:")) != -1) {
        switch (opt) {
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

    if (compileOnly) {
	theLMan.compile();
	return 0;
    }

    Map theMap(map_f);
    Ghosts theGhosts(ghost_f, theMap.ghostData());

    map<tick, vector<Move>> theMoves;
    tick ticker=0;
    do {
	// #1 - all moves take place.
	theMap.apply(ticker, theMoves);
	// #1.1 - add new moves if necessary
	theGhosts.move(ticker, theMoves, theMap);
	theLMan.move(ticker, theMoves, theMap);

	// #2 - timed actions.
	theGhosts.apply(ticker);
	theMap.apply(ticker);

	// #3 - effects. #4 - ghost processing
	Effects effects = theMap.effects();
	theLMan.processEffects(effects);
	theGhosts.processEffects(effects);

	// #5 - win condition
	if (theMap.cleanedUp()) {
	    theMap.adjustScore(theLMan.lives());
	    break;
	}

	// #6 - lose condition
	if (theLMan.lives() == 0) {
	    break;
	}
	
	// # inc ticks
	++ticker;
    } while (true);

    printf("Final score: %u; %lu ticks\n", theMap.score(), ticker);
    return 0;
}
