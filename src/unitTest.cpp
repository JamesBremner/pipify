#include <vector>
#include <iostream>
#include <cxy.h>
#include "pipify.h"

bool unitTest()
{
    cRoom::readfile("../dat/r1.text");
    auto segs = cRoom::houseWallSegments();

    // there is one room in the house
    if (segs.size() != 1)
        return false;
    // the room has two wall segments
    if (segs[0].size() != 2)
        return false;
    if (segs[0][0].size() != 2)
        return false;
    if (segs[0][1].size() != 5)
        return false;

    cCorners C(cRoom::getRooms()[0]);
    const auto &vc = C.getCorners();
    if (vc.size() != 5)
        return false;
    if (C.index(3) != 1)
        return false;

     cRoom::readfile("../dat/L.text");
     cxy jointPoint;
    // auto subRooms = ConcaveSplit(
    //     cRoom::getRooms()[0],
    //     jointPoint );
    // subRooms.first.pipeConvex();
    // subRooms.second.pipeConvex();

    cRoom::readfile("../dat/L2.txt");
    auto subRooms = concaveSplit(
        cRoom::getRooms()[0],
        jointPoint);
    subRooms.second.pipeConvex();

    cxy p1(0, 1);
    cxy p2(0, 0);
    cxy p3(1, 0);
    if (cRoom::corner(p1, p2, p3) != eCorner::tl_vex)
        return false;
    p1 = cxy(0, 0);
    p2 = cxy(1, 0);
    p3 = cxy(1, 1);
    if (cRoom::corner(p1, p2, p3) != eCorner::tr_vex)
        return false;
    p1 = cxy(1, 0);
    p2 = cxy(1, 1);
    p3 = cxy(0, 1);
    if (cRoom::corner(p1, p2, p3) != eCorner::br_vex)
        return false;
    p1 = cxy(1, 1);
    p2 = cxy(0, 1);
    p3 = cxy(0, 0);
    if (cRoom::corner(p1, p2, p3) != eCorner::bl_vex)
        return false;
    p1 = cxy(0, 0);
    p2 = cxy(0, 1);
    p3 = cxy(1, 1);
    if (cRoom::corner(p1, p2, p3) != eCorner::tr_cav)
        return false;

    p1 = cxy(94, 94);
    p2 = cxy(6, 94);
    cxy a(16, 0);
    cxy b(16, 6);
    cxy intersection;
    cxy::isIntersection(
        intersection,
        p1, p2,
        a, b);

    return true;
}
