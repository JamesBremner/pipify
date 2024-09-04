#include <vector>
#include <iostream>
#include <cxy.h>
#include "pipify.h"

bool unitTest()
{
    gen1();
    auto segs = cRoom::houseWallSegments();

    // there is one room in the house
    if (segs.size() != 1)
        return false;
    // the room has two wall segments
    if (segs[0].size() != 2)
        return false;
    if (segs[0][0].size() != 2)
        return false;
    if (segs[0][1].size() != 4)
        return false;


    cxy p1(0,1);
    cxy p2(0,0);
    cxy p3(1,0);
    if( cRoom::corner( p1,p2,p3) != cRoom::eCorner::tl_vex )
        return false;
    p1 = cxy(0,0);
    p2 = cxy(1,0);
    p3 = cxy(1,1);
    if( cRoom::corner( p1,p2,p3) != cRoom::eCorner::tr_vex )
        return false;
    p1 = cxy(1,0);
    p2 = cxy(1,1);
    p3 = cxy(0,1);
    if( cRoom::corner( p1,p2,p3) != cRoom::eCorner::br_vex )
        return false;
    p1 = cxy(1,1);
    p2 = cxy(0,1);
    p3 = cxy(0,0);
    if( cRoom::corner( p1,p2,p3) != cRoom::eCorner::bl_vex )
        return false;
    p1 = cxy(0,0);
    p2 = cxy(0,1);
    p3 = cxy(1,1);
    if( cRoom::corner( p1,p2,p3) != cRoom::eCorner::tr_cav )
        return false;

    p1 = cxy(94,94);
    p2 = cxy( 6,94);
    cxy a( 16,0 );
    cxy b( 16,6 );
    cxy intersection;
    cxy::isIntersection(
                intersection,
                p1, p2,
                a,b);

    return true;
}
