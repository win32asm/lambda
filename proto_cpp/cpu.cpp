#include "gcc.h"
#include "ghc.h"
#include "map.h"

#include <unistd.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <ifstream>

using std::vector;
using std::string;

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


using icfpc; // всё в этот неймспейс!

int main(int argc, const char *argv[])
{
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

    map<tick,vector<Move>> theMoves; // <tick, moves by source>
    tick ticker=0;
    do {
        // #1 - apply moves
        auto tickMoves = theMoves.find(ticker);
        if (tickMoves != theMoves.end()) {
            for (const Move& mv:*tickMoves) {
                unsigned nextDelta = theMap.move(mv);
                if (mv.Source == Source::LMan) {
                    theLMan.schedNextMove(theMoves[ticker+nextDelta], theMap); // MAY schedule something
                } else {
                    theGhosts.schedNextMove(mv.Source, theMoves[ticker+nextDelta], theMap); // MUST schedule something!
                }
            }
            theMoves.erase(tickMoves);
        }

        // #2 - timed actions.
        theMap.apply(ticker);

        // #3 - effects. #4 - ghost processing
        theMap.effects();

        // #5 - win condition
        if (theMap.cleanedUp()) {
            theMap.adjustScore();
            printf("Winner\n");
            break;
        }

        // #6 - lose conditions
        if ( theMap.lives() == 0 ) {
            printf("Dead\n");
            break;
        }
        if ( ticker == 127*theMap.mapSize()*16 ) {
            printf("Time out\n");
            break;
        }

        // # inc ticks
        ++ticker;
    } while (true);

    printf("Final score: %u; %lu ticks\n", theMap.score(), ticker);
    return 0;
}
