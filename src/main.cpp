#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include <cxy.h>
#include "cStarterGUI.h"

#include "pipify.h"

std::vector<cRoom> cRoom::theHouse;

int cRoom::theSeperation;

cRoom::cRoom(
    const std::vector<cxy> wallPoints,
    const std::vector<int> doorPoints)
{
    myWallPoints = wallPoints;
    myDoorPoints = doorPoints;
    // pipe();
}

void cRoom::add(
    const std::vector<cxy> wallPoints,
    const std::vector<int> doorPoints)
{
    theHouse.emplace_back(wallPoints, doorPoints);
}
void cRoom::clear()
{
    theHouse.clear();
}

std::vector<std::vector<cxy>> cRoom::wallSegments()
{
    std::vector<std::vector<cxy>> ret;
    std::vector<cxy> segment;
    int idp = myDoorPoints[0];
    for (int idw = 0; idw < myWallPoints.size(); idw++)
    {
        segment.push_back(myWallPoints[idw]);
        if (idw == idp)
        {
            // door point
            ret.push_back(segment);
            segment.clear();
            idp++;
            if (idp > myDoorPoints.size())
            {
                // found all the doors
                idp = myWallPoints.size() + 10;
            }
        }
    }
    if (segment.size())
        ret.push_back(segment);

    return ret;
}

cRoom::eMargin cRoom::side(const cxy &p1, const cxy &p2)
{
    if (p1.x == p2.x)
        if (p1.y < p2.y)
            return eMargin::right;
        else
            return eMargin::left;
    else if (p1.x < p2.x)
        return eMargin::top;
    else
        return eMargin::bottom;
}

cRoom::eCorner cRoom::corner(
    const cxy &p1,
    const cxy &p2,
    const cxy &p3)
{
    eMargin m1 = side(p1, p2);
    eMargin m2 = side(p2, p3);
    if (m1 == eMargin::left && m2 == eMargin::top)
        return eCorner::tl_vex;
    if (m1 == eMargin::top && m2 == eMargin::left)
        return eCorner::tl_cav;
    if (m1 == eMargin::top && m2 == eMargin::right)
        return eCorner::tr_vex;
    if (m1 == eMargin::right && m2 == eMargin::top)
        return eCorner::tr_cav;
    if (m1 == eMargin::right && m2 == eMargin::bottom)
        return eCorner::br_vex;
    if (m1 == eMargin::bottom && m2 == eMargin::right)
        return eCorner::br_cav;
    if (m1 == eMargin::bottom && m2 == eMargin::left)
        return eCorner::bl_vex;
    if (m1 == eMargin::left && m2 == eMargin::bottom)
        return eCorner::bl_cav;

    return eCorner::error;
}

bool cRoom::isPipeCrossing(const cxy &p1, const cxy &p2) const
{
    cxy intersection;

    // loop over previous pipe segments
    for (
        int i = myPipePoints.size() - 1;
        i > 0;
        i--)
    {
        // find intersection
        if (cxy::isIntersection(
                intersection,
                p1, p2,
                myPipePoints[i - 1], myPipePoints[i]))
        {
            // check that intersection is not at either end of pipe segment
            if ((!(intersection == p1)) &&
                (!(intersection == p2)))
                return true;
        }
    }
    return false;
}

std::pair<cxy, cxy> cRoom::find(eMargin m) const
{
    for (int ip = 0; ip < myWallPoints.size(); ip++)
        if (side(myWallPoints[ip], myWallPoints[ip + 1]) == m)
            return std::make_pair(myWallPoints[ip], myWallPoints[ip + 1]);
    throw std::runtime_error(
        "cRoom::find errorrr");
}

cRoom::eCorner cRoom::isConcave(int &index) const
{
    auto co = corner(
        myWallPoints.back(),
        myWallPoints[0],
        myWallPoints[1]);
    switch (co)
    {
    case eCorner::bl_cav:
    case eCorner::br_cav:
    case eCorner::tl_cav:
    case eCorner::tr_cav:
        index = 0;
        return co;
    }
    for (
        index = 1;
        index < myWallPoints.size() - 1;
        index++)
    {
        if (std::find(myDoorPoints.begin(), myDoorPoints.end(), index) != myDoorPoints.end())
        {
            index++;
            continue;
        }
        co = corner(
            myWallPoints[index - 1],
            myWallPoints[index],
            myWallPoints[index + 1]);
        switch (co)
        {
        case eCorner::bl_cav:
        case eCorner::br_cav:
        case eCorner::tl_cav:
        case eCorner::tr_cav:
            return co;
        }
    }

    // the room is convex
    return eCorner::error;
}

void cRoom::pipe()
{
    cxy eWall1, eWallMax, eWall2;
    std::pair<cxy, cxy> oppositeWall;
    int concaveIndex;
    switch (isConcave(concaveIndex))
    {

    case eCorner::tr_cav:
    {
        std::cout << "room is concave\n";

        oppositeWall = find(eMargin::bottom);
        eWall1 = myWallPoints[concaveIndex];
        eWallMax = cxy(myWallPoints[concaveIndex].x, INT_MAX);
        eWall2;
        cxy::isIntersection(
            eWall2,
            eWall1, eWallMax,
            oppositeWall.first, oppositeWall.second);

        std::vector<cxy> subRoomWallPoints;
        for (int ip = 0; ip < myWallPoints.size(); ip++)
        {
            if (ip != concaveIndex)
                subRoomWallPoints.push_back(myWallPoints[ip]);
            else
            {
                while (true)
                {
                    ip++; // skip wall points to be cut off by extra wall
                    if (myWallPoints[ip] == oppositeWall.first)
                    {
                        subRoomWallPoints.push_back(eWall2);
                        break; // continue for remaining wall points
                    }
                }
                std::cout << "include remaining wallpoints\n";
            }
        }
        std::cout << "constructing subroom `\n";
        cRoom subRoom(
            subRoomWallPoints,
            myDoorPoints);
        subRoom.pipeConvex();
        myPipePoints = subRoom.myPipePoints;

        subRoomWallPoints.clear();
        std::vector<int> subRoomDoorPoints;
        for (int ip = concaveIndex; true; ip++)
        {
            subRoomWallPoints.push_back(myWallPoints[ip]);
            if (ip == concaveIndex)
            {
                subRoomWallPoints.push_back(cxy(myWallPoints[ip].x + 5, myWallPoints[ip].y));
                subRoomWallPoints.push_back(cxy(myWallPoints[ip].x + 10, myWallPoints[ip].y));
                subRoomDoorPoints.push_back(1);
            }
            if (myWallPoints[ip] == oppositeWall.first) {
                //subRoomWallPoints.push_back(myWallPoints[ip]);
                subRoomWallPoints.push_back(eWall2);
                break;
            }
        }
        cRoom subRoom2(
            subRoomWallPoints,
            subRoomDoorPoints);
        subRoom2.pipeConvex();
        myPipePoints.insert(myPipePoints.end(), subRoom2.myPipePoints.begin(), subRoom2.myPipePoints.end());
    }
    break;

    case eCorner::error:
    default:
        std::cout << "room is convex\n";
        this->pipeConvex();
        break;
    }
}

void cRoom::pipeConvex()
{
    int indexDoor2;
    cxy p1, p2, p3;
    if (myDoorPoints.size())
    {
        // start at the first door
        cxy d1 = myWallPoints[myDoorPoints[0]];
        cxy d2 = myWallPoints[myDoorPoints[0] + 1];

        switch (side(d1, d2))
        {
        case eMargin::top:
            p1.x = d1.x + theSeperation;
            p1.y = d1.y;
            p2.x = p1.x;
            p2.y = p1.y + theSeperation;
            p3.x = d2.x;
            p3.y = p2.y;
            break;
        }
        myPipePoints.push_back(p1);
        myPipePoints.push_back(p2);
        myPipePoints.push_back(p3);
        indexDoor2 = myDoorPoints[0] + 1;
    }
    else
    {
        indexDoor2 = 0;
    }

    // for (int L = 1; L < 5; L++)
    for (int L = 1; true; L++)
    {
        int wallSeperation = L * theSeperation;

        for (int i = indexDoor2 + 1;
             i < myWallPoints.size();
             i++)
        {
            p1 = myWallPoints[i - 1];
            p2 = myWallPoints[i];
            if (i == myWallPoints.size() - 1)
                p3 = myWallPoints[0];
            else
                p3 = myWallPoints[i + 1];

            switch (corner(p1, p2, p3))
            {
            case eCorner::tr_vex:
                p2.x -= wallSeperation;
                p2.y += wallSeperation;
                break;
            case eCorner::br_vex:
                p2.x -= wallSeperation;
                p2.y -= wallSeperation;
                break;
            case eCorner::bl_vex:
                p2.x += wallSeperation;
                p2.y -= wallSeperation;
                break;
            case eCorner::tl_vex:
                p2.x += wallSeperation;
                p2.y += wallSeperation;
                break;
            case eCorner::error:
                throw std::runtime_error(
                    " pipe corner error");
            }

            // cross = isPipeCrossing(myPipePoints.back(), p2);
            // if (cross)
            //     break;

                    // check if spiral has become vanishingly small
        if (myPipePoints.back().dist2(p2) < theSeperation)
            return;

            myPipePoints.push_back(p2);
        }

        // if (cross)
        //     break;

        // close the polygon
        p1 = myWallPoints.back();
        p2 = myWallPoints[0];
        switch (side(p1, p2))
        {
        case eMargin::top:
            p2.x -= wallSeperation;
            p2.y += wallSeperation;
            break;
        case eMargin::right:
            p2.x -= wallSeperation;
            p2.y -= wallSeperation;
            break;
        case eMargin::bottom:
            p2.x += wallSeperation;
            p2.y -= wallSeperation;
            break;
        case eMargin::left:
            p2.x += wallSeperation;
            p2.y += wallSeperation + theSeperation;
            break;
        }

        // check if spiral has become vanishingly small
        if (myPipePoints.back().dist2(p2) < theSeperation)
            break;

        myPipePoints.push_back(p2);
    }
}

// generate a one room house
void gen1()
{
    std::vector<cxy> wallPoints = {{0, 0}, {10, 0}, {30, 0}, {100, 0}, {100, 100}, {0, 100}};
    std::vector<int> doorPoints = {1};

    cRoom::clear();
    cRoom::set(6); // pipe seperation
    cRoom::add(wallPoints, doorPoints);
}

// generate a house with one L shaped room
void genL()
{
    std::vector<cxy> wallPoints = {
        {0, 0},
        {10, 0},
        {30, 0},
        {100, 0},
        {100, 50},
        {200, 50},
        {200, 100},
        {0, 100},
    };
    std::vector<int> doorPoints = {1};

    cRoom::clear();
    cRoom::set(6); // pipe seperation
    cRoom::add(wallPoints, doorPoints);
}

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "Pipify",
              {50, 50, 1000, 500})
    {

        genL();
        cRoom::pipeHouse();

        fm.events().draw(
            [](PAINTSTRUCT &ps)
            {
                const int scale = 3;
                const int off = 20;
                wex::shapes S(ps);
                S.color(0);
                S.penThick(4);

                // loop over rooms
                for (auto &r : cRoom::houseWallSegments())
                {

                    // loop over wall segments
                    for (auto &s : r)
                    {
                        int x1, y1, x2, y2;
                        x2 = INT_MAX;

                        // loop over points
                        for (auto &p : s)
                        {
                            if (x2 == INT_MAX)
                            {
                                x2 = off + scale * p.x;
                                y2 = off + scale * p.y;
                            }
                            else
                            {
                                x1 = x2;
                                y1 = y2;
                                x2 = off + scale * p.x;
                                y2 = off + scale * p.y;
                                S.line({x1, y1, x2, y2});
                            }
                        }

                        // close the polygon
                        x1 = x2;
                        y1 = y2;
                        x2 = off + scale * r[0][0].x;
                        y2 = off + scale * r[0][0].y;
                        S.line({x1, y1, x2, y2});
                    }
                }

                S.penThick(1);
                for (auto &r : cRoom::housePipes())
                {
                    int x1, y1, x2, y2;
                    x2 = INT_MAX;
                    for (auto &p : r)
                    {
                        if (x2 == INT_MAX)
                        {
                            x2 = off + scale * p.x;
                            y2 = off + scale * p.y;
                        }
                        else
                        {
                            x1 = x2;
                            y1 = y2;
                            x2 = off + scale * p.x;
                            y2 = off + scale * p.y;
                            S.line({x1, y1, x2, y2});
                        }
                    }
                }
            });

        show();
        run();
    }

private:
};

main()
{
    // if (!unitTest())
    // {
    //     std::cout << "unit test failed\n";
    //     exit(1);
    // }

    cGUI theGUI;
    return 0;
}
