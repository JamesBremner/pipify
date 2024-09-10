#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include <cxy.h>

#include "pipify.h"

#include "cGUI.h"

// define static attributes

std::vector<cRoom> cRoom::theHouse;

int cRoom::theSeperation;

int cRoom::thefurnaceRoomIndex;

cRoom::cRoom(
    const std::string &name,
    const std::vector<cxy> wallPoints,
    const std::vector<int> doorPoints)
    : myName(name)
{
    myWallPoints = wallPoints;
    myDoorPoints = doorPoints;
    // pipe();
}

void cRoom::add(
    const std::string &name,
    const std::vector<cxy> wallPoints,
    const std::vector<int> doorPoints)
{
    theHouse.emplace_back(name, wallPoints, doorPoints);
}
void cRoom::clear()
{
    theHouse.clear();
}

std::vector<std::vector<cxy>> cRoom::wallSegments()
{
    std::vector<std::vector<cxy>> ret;
    std::vector<cxy> segment;
    int doorIndex = INT_MAX;
    if (myDoorPoints.size())
        doorIndex = 0;
    for (int idw = 0; idw < myWallPoints.size(); idw++)
    {
        segment.push_back(myWallPoints[idw]);
        if (idw == myDoorPoints[doorIndex])
        {
            // door point
            ret.push_back(segment);
            segment.clear();
            doorIndex++;
            if (doorIndex > myDoorPoints.size())
            {
                // found all the doors
                doorIndex = myWallPoints.size() + 10;
            }
        }
        if (idw == myWallPoints.size() - 1)
        {
            // wall that closes the polygon
            segment.push_back(myWallPoints[0]);
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
    if (m1 == m2)
    {
        switch (m1)
        {
        case eMargin::top:
            return eCorner::top;
        case eMargin::right:
            return eCorner::right;
        case eMargin::bottom:
            return eCorner::bottom;
        case eMargin::left:
            return eCorner::left;
        }
    }

    return eCorner::error;
}

bool cRoom::isPipeCrossing(const cxy &p1, const cxy &p2) const
{
    // cxy intersection;

    // // loop over previous pipe segments
    // for (
    //     int i = myPipePoints.size() - 1;
    //     i > 0;
    //     i--)
    // {
    //     // find intersection
    //     if (cxy::isIntersection(
    //             intersection,
    //             p1, p2,
    //             myPipePoints[i - 1], myPipePoints[i]))
    //     {
    //         // check that intersection is not at either end of pipe segment
    //         if ((!(intersection == p1)) &&
    //             (!(intersection == p2)))
    //             return true;
    //     }
    // }
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

void cRoom::pipeDoor(std::vector<cxy> &pipeSegment)
{
    cxy p1, p2, p3;

    // start at the first door
    cxy d1 = myWallPoints[myDoorPoints[0]];
    cxy d2 = myWallPoints[myDoorPoints[0] + 1];
    double doorWidth = sqrt(d1.dist2(d2));

    // switch on side where door is placed
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
    case eMargin::right:
        p1.x = d1.x;
        p1.y = d1.y + 2 * theSeperation;
        p2.x = p1.x - theSeperation;
        p2.y = p1.y;
        p3.x = p2.x;
        p3.y = d2.y;
        break;
    case eMargin::bottom:
        p1.x = d1.x - 2 * theSeperation;
        p1.y = d1.y;
        p2.x = p1.x;
        p2.y = p1.y - theSeperation;
        p3.x = p2.x - theSeperation;
        p3.y = p2.y;
        break;
    case eMargin::left:
        p1.x = d1.x - 2 * theSeperation;
        p1.y = d1.y;
        p2.x = p1.x;
        p2.y = p1.y - theSeperation;
        p3.x = p2.x - theSeperation;
        p3.y = p2.y;
        break;
    }
    pipeSegment.push_back(p1);
    pipeSegment.push_back(p2);
    pipeSegment.push_back(p3);
}

void cRoom::pipeHouse()
{
    for (int ri = 0; ri < theHouse.size(); ri++)
    {
        std::cout << "calculating pipe layout for room "
                  << theHouse[ri].myName << "\n";
        if (ri == thefurnaceRoomIndex)
        {
            theHouse[ri].pipefurnaceRoom();
            continue;
        }

        theHouse[ri].pipe();
    }
}

void cRoom::pipefurnaceRoom()
{
    cxy p1, p2, p3;
    const int wallSeperation = 5;
    std::vector<cxy> spiral;

    for (int i = 0;
         i < myWallPoints.size();
         i++)
    {
        if (i == 0)
            p1 = myWallPoints.back();
        else
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
        case eCorner::top:
            p2.y += wallSeperation;
            break;
        case eCorner::right:
            p2.x -= wallSeperation;
            break;
        case eCorner::bottom:
            p2.y -= wallSeperation;
            break;
        case eCorner::left:
            p2.x += wallSeperation;
            break;
        case eCorner::error:
            break;
        }
        spiral.push_back(p2);
    }
    myPipePoints.push_back(spiral);

    for (int doorIndex : myDoorPoints)
    {
        spiral.clear();
        p1 = myWallPoints[doorIndex];
        p2 = myWallPoints[doorIndex + 1];
        switch (side(p1, p2))
        {
        case eMargin::top:
            p1.x += (p2.x - p1.x) / 2;
            p1.y += wallSeperation;
            p2.x = p1.x;
            break;
        case eMargin::bottom:
            p1.x += (p2.x - p1.x) / 2;
            p1.y -= wallSeperation;
            p2.x = p1.x;
            break;
        case eMargin::left:
            p1.y -= (p1.y - p2.y) / 2;
            p2.x += wallSeperation;
            p2.y = p1.y;
            break;
        }
        spiral.push_back(p1);
        spiral.push_back(p2);
        myPipePoints.push_back(spiral);
    }
}
void cRoom::pipe()
{
    int concaveIndex;
    switch (isConcave(concaveIndex))
    {
    case eCorner::bl_cav:
    case eCorner::br_cav:
    case eCorner::tl_cav:
    case eCorner::tr_cav:
        pipeConcave(concaveIndex);
        break;

    case eCorner::error:
    default:
        myPipePoints.push_back(this->pipeConvex());
        break;
    }
}
void cRoom::pipeConcave(int concaveIndex)
{
    cxy eWall1, eWallMax, eWall2;
    std::pair<cxy, cxy> oppositeWall;

    switch (isConcave(concaveIndex))
    {

    case eCorner::tr_cav:
    {

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
        std::cout << "constructing subroom \n";
        cRoom subRoom(
            "",
            subRoomWallPoints,
            myDoorPoints);
        myPipePoints.push_back(subRoom.pipeConvex());

        subRoomWallPoints.clear();
        std::vector<int> subRoomDoorPoints;
        for (int ip = concaveIndex; true; ip++)
        {
            subRoomWallPoints.push_back(myWallPoints[ip]);

            if (myWallPoints[ip] == oppositeWall.first)
            {
                subRoomWallPoints.push_back(eWall2);
                break;
            }
        }
        cRoom subRoom2(
            "",
            subRoomWallPoints,
            subRoomDoorPoints);
        cxy startPoint = myWallPoints[concaveIndex];
        startPoint.x -= theSeperation;
        startPoint.y += theSeperation;
        myPipePoints.push_back(subRoom2.pipeConvex(startPoint.x, startPoint.y));
    }
    break;
    }
}
std::vector<cxy> cRoom::pipeConvex(int x, int y)
{
    std::vector<cxy> spiral;

    int spiralStartCornerIndex;

    cxy spiralStartPoint(x, y);
    if (myDoorPoints.size())
    {
        pipeDoor(spiral);
        spiralStartCornerIndex = myDoorPoints[0] + 2;
        if (spiralStartCornerIndex == myWallPoints.size() - 1)
            spiralStartCornerIndex = 1;
        spiralStartPoint = myWallPoints[myDoorPoints[0] + 1];
        spiralStartPoint.y += theSeperation;
    }
    else
    {
        // there is no door
        // this must be a subroom, part of a concave room
        // start at nearest pipe in first subroom

        spiralStartCornerIndex = 1;
    }

    spiral = pipeSpiral(
        spiralStartPoint,
        spiralStartCornerIndex);

    return spiral;
}
std::vector<cxy> cRoom::pipeSpiral(
    const cxy &startPoint,
    int startIndex)
{
    // construct closed polygon without doors
    std::vector<cxy> noDoors;
    int nextDoorIndex = 0;
    for (int i = 0; i < myWallPoints.size(); i++)
    {
        if (i == myDoorPoints[nextDoorIndex])
        {
            // skip door points
            i++;
            i++;
            if (i == startIndex)
                startIndex = noDoors.size();
            nextDoorIndex++;
            if (nextDoorIndex == myDoorPoints.size() - 1)
                nextDoorIndex = -1;
        }
        noDoors.push_back(myWallPoints[i]);
    }
    // close polygon
    noDoors.push_back(myWallPoints[0]);

    std::vector<cxy> spiral;

    // start from startPoint parameter
    spiral.push_back(startPoint);

    int wallSeperation = theSeperation;

    for (int cornerIndex = startIndex; true; cornerIndex++)
    {
        if (cornerIndex == noDoors.size() - 1)
        {
            // wrap around noDoors
            cornerIndex = 0;
        }

        bool fspiralwrap = false;
        if (cornerIndex == startIndex - 1)
        {
            // wrap around spiral
            wallSeperation += theSeperation;
            fspiralwrap = true;
        }

        cxy p1, bend, p3;
        if (cornerIndex == 0)
            p1 = p1 = noDoors[noDoors.size() - 2];
        else
        {
            p1 = noDoors[cornerIndex - 1];
        }
        bend = noDoors[cornerIndex];
        p3 = noDoors[cornerIndex + 1];

        // std::cout << cornerIndex << " " << bend.x << " " << bend.y
        //           << " " << wallSeperation << "\n";

        switch (corner(p1, bend, p3))
        {
        case eCorner::tr_vex:
            bend.x -= wallSeperation;
            bend.y += wallSeperation;
            break;
        case eCorner::br_vex:
            bend.x -= wallSeperation;
            bend.y -= wallSeperation;
            break;
        case eCorner::bl_vex:
            bend.x += wallSeperation;
            bend.y -= wallSeperation;
            break;
        case eCorner::tl_vex:
            bend.x += wallSeperation;
            bend.y += wallSeperation;
            if (fspiralwrap)
                bend.x -= theSeperation;
            break;
        default:
            throw std::runtime_error(
                "pipeSpiral bad corner");
        }
        // check if spiral has become vanishingly small
        if (isSpiralComplete(spiral, bend))
            return spiral;
    }

    return spiral;
}
bool cRoom::isSpiralComplete(
    std::vector<cxy> &spiral,
    const cxy &nextbend) const
{
    // distance squared from last pipe point to new point
    double d2 = spiral.back().dist2(nextbend);
    if (d2 < theSeperation * theSeperation)
        return true;

    // keep going
    spiral.push_back(bend);
    return false;
}

void cRoom::readfile(const std::string &fname)
{
    std::ifstream ifs(fname);
    if (!ifs.is_open())
        throw std::runtime_error(
            "Cannot open file " + fname);

    std::string line, name, furnaceRoomName;
    std::vector<cxy> wallPoints;
    std::vector<int> doorPoints;

    clear();

    while (getline(ifs, line))
    {
        if (!line.length())
            continue;
        if (line.find("//") == 0)
            break;

        int p = line.find("room");
        if (p == 0)
        {
            // new room specified

            if (wallPoints.size())
            {
                // save previous room
                add(name, wallPoints, doorPoints);
            }

            // clear new room specs
            wallPoints.clear();
            doorPoints.clear();
            name = line.substr(5);
            continue;
        }

        p = line.find("furnace");
        if (p == 0)
        {
            p = line.find(" ");
            furnaceRoomName = line.substr(p + 1);
            continue;
        }

        p = line.find(" ");
        if (p != -1)
        {
            wallPoints.emplace_back(
                atof(line.c_str()), atof(line.substr(p + 1).c_str()));
        }
        else
        {
            doorPoints.push_back(atoi(line.c_str()));
        }
    }

    // save last room
    add(name, wallPoints, doorPoints);

    set(6); // pipe seperation

    furnaceRoom(furnaceRoomName);
}

void cRoom::furnaceRoom(const std::string &name)
{
    if (!name.length())
        throw std::runtime_error("Unspecified furnace room");
    if (name == "test")
    {
        // test house with no furnace room
        thefurnaceRoomIndex = -1;
        return;
    }
    auto it = std::find_if(theHouse.begin(), theHouse.end(),
                           [&](cRoom &room) -> bool
                           {
                               return (room.myName == name);
                           });
    if (it == theHouse.end())
        throw std::runtime_error("Unspecified furnace room " + name);
    thefurnaceRoomIndex = it - theHouse.begin();

    // check that there are enough doors in the furnace room
    // to reach every other room
    if (theHouse.size() - 1 > theHouse[thefurnaceRoomIndex].doorCount())
        throw std::runtime_error("Not enough doors in furnace room");

    // TODO: check every room connected directly to furnace room TID3
}

void cGUI::drawHousePipes(
    wex::shapes &S)
{
    S.penThick(1);
    for (int ir = 0; ir < cRoom::roomCount(); ir++)
    {
        if (ir == cRoom::furnaceRoomIndex())
        {
            drawFurnacePipes(S, ir);
            continue;
        }

        // loop over pipe segments
        for (auto &pipesegment : cRoom::getRooms()[ir].pipes())
        {
            drawPipeSegment(S, pipesegment);
        }
    }
}

void cGUI::drawWalls(
    wex::shapes &S,
    const std::vector<std::vector<cxy>> &walls)
{
    // loop over wall segments
    for (auto &s : walls)
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
    }
}

void cGUI::drawFurnacePipes(
    wex::shapes &S,
    int ir)
{

    cxy p1, p2, p3, p4;
    auto pipes = cRoom::getRooms()[ir].pipes();
    auto seg = pipes[0];
    {
        for (int ip = 0; ip < seg.size(); ip++)
        {
            p1 = p2;
            p2.x = off + scale * seg[ip].x;
            p2.y = off + scale * seg[ip].y;
            if (ip == 0)
                continue;
            S.color(0x0000FF);
            S.line({p1.x, p1.y, p2.x, p2.y});
            switch (cRoom::side(p1, p2))
            {
            case cRoom::eMargin::top:
                p3.x = p1.x + 3;
                p3.y = p1.y + 3;
                p4.x = p2.x - 3;
                p4.y = p2.y + 3;
                break;
            case cRoom::eMargin::right:
                p3.x = p1.x - 3;
                p3.y = p1.y + 3;
                p4.x = p2.x - 3;
                p4.y = p2.y - 3;
                break;
            case cRoom::eMargin::bottom:
                p3.x = p1.x - 3;
                p3.y = p1.y - 3;
                p4.x = p2.x + 3;
                p4.y = p2.y - 3;
                break;
            case cRoom::eMargin::left:
                p3.x = p1.x + 3;
                p3.y = p1.y - 3;
                p4.x = p2.x + 3;
                p4.y = p2.y + 3;
                break;
            }
            S.color(0xFF0000);
            S.line({p3.x, p3.y, p4.x, p4.y});
        }
        p1 = p2;
        p2.x = off + scale * pipes[0][0].x;
        p2.y = off + scale * pipes[0][0].y;
        S.color(0x0000FF);
        S.line({p1.x, p1.y, p2.x, p2.y});
        p3.x = p1.x + 3;
        p3.y = p1.y - 3;
        p4.x = p2.x + 3;
        p4.y = p2.y + 3;
        S.color(0xFF0000);
        S.line({p3.x, p3.y, p4.x, p4.y});
    }

    for (int i = 1; i < pipes.size(); i++)
    {
        p1.x = off + scale * pipes[i][0].x;
        p1.y = off + scale * pipes[i][0].y;
        p2.x = off + scale * pipes[i][1].x;
        p2.y = off + scale * pipes[i][1].y;
        S.color(0x0000FF);
        S.line({p1.x, p1.y, p2.x, p2.y});

        // return pipe
        switch (cRoom::side(p1, p2))
        {
        case cRoom::eMargin::left:
            p1.x += 5;
            p1.y += 4;
            p2.x += 5;
            break;
        case cRoom::eMargin::bottom:
            p1.x += 5;
            p1.y -= 4;
            p2.x += 5;
            break;
        case cRoom::eMargin::right:
            p1.x += 5;
            p1.y -= 4;
            p2.x += 5;
            break;
        case cRoom::eMargin::top:
            p1.y -= 5;
            p2.x += 5;
            p2.y = p1.y;
            break;
        }
        S.color(0xFF0000);
        S.line({p1.x, p1.y, p2.x, p2.y});
    }
}

void cGUI::drawPipeSegment(
    wex::shapes &S,
    const std::vector<cxy> &pipesegment)
{
    const int outInSep = cRoom::seperation() + 2;
    int x1, y1, x2, y2, x3, y3, x4, y4;
    x2 = INT_MAX;
    cxy lastReturn;

    // draw in doorway pipe
    x1 = off + scale * pipesegment[0].x;
    y1 = off + scale * pipesegment[0].y;
    x2 = off + scale * pipesegment[1].x;
    y2 = off + scale * pipesegment[1].y;
    x3 = off + scale * pipesegment[2].x;
    y3 = off + scale * pipesegment[2].y;
    S.color(0x0000FF);
    S.line({x1, y1, x2, y2});
    S.line({x2, y2, x3, y3});
    S.color(0);

    // draw out doorway pipe
    int sep = cRoom::seperation();
    switch (
        cRoom::side(pipesegment[1], pipesegment[2]))
    {
    case cRoom::eMargin::top:
        x1 -= sep;
        x2 -= sep;
        y2 += sep;
        x3 += sep + 1;
        y3 += sep + 1;
        break;
    case cRoom::eMargin::right:
        y1 -= sep;
        x2 -= sep;
        y2 -= sep;
        x3 -= sep;
        y3 += sep + 1;
        break;
    case cRoom::eMargin::bottom:
        x1 += sep;
        x2 += sep;
        y2 -= 2 * sep;
        y3 -= 2 * sep;
        break;
    }
    S.color(0xFF0000);
    S.line({x1, y1, x2, y2});
    S.line({x2, y2, x3, y3});
    S.color(0);

    // loop over pipe bends
    for (int ip = 2; ip < pipesegment.size() - 1; ip++)
    {
        cxy p = pipesegment[ip];

        if (ip == 2)
        {
            x2 = off + scale * p.x;
            y2 = off + scale * p.y;
        }
        else
        {
            x1 = x2;
            y1 = y2;
            x2 = off + (int)(scale * p.x);
            y2 = off + (int)(scale * p.y);
            S.color(0x0000FF);
            S.line({x1, y1, x2, y2});
            S.color(0);

            // draw return pipe
            int rx1, ry1, rx2, ry2;
            switch (cRoom::corner(pipesegment[ip - 1], p, pipesegment[ip + 1]))
            {
            case cRoom::eCorner::tl_vex:
                rx1 = x1 + outInSep;
                ry1 = y1 - outInSep;
                rx2 = x2 + outInSep;
                ry2 = y2 + outInSep;
                break;
            case cRoom::eCorner::tr_vex:
                rx1 = x1 + outInSep;
                ry1 = y1 + outInSep;
                rx2 = x2 - outInSep;
                ry2 = y2 + outInSep;
                break;
            case cRoom::eCorner::br_vex:
                rx1 = x1 - outInSep;
                ry1 = y1 + outInSep;
                rx2 = x2 - outInSep;
                ry2 = y2 - outInSep;
                break;
            case cRoom::eCorner::bl_vex:
                rx1 = x1 - outInSep;
                ry1 = y1 - outInSep;
                rx2 = x2 + outInSep;
                ry2 = y2 - outInSep;
                break;
            case cRoom::eCorner::error:
                if (p.y == pipesegment[ip - 1].y)
                {
                    S.line({x1, y1 + outInSep, x2 + outInSep, y2 + outInSep});
                }
                break;
            }
            lastReturn = cxy(rx2, ry2);
            S.color(0xFF0000);
            S.line({rx1, ry1, rx2, ry2});
            S.color(0);
        }
    }
    // connect spiral centers
    int lastbendindex = pipesegment.size() - 2;
    x1 = off + scale * pipesegment[lastbendindex].x;
    y1 = off + scale * pipesegment[lastbendindex].y;
    x2 = lastReturn.x;
    y2 = lastReturn.y;
    S.line({x1, y1, x2, y2});
}

main()
{
    cGUI theGUI;
    return 0;
}
