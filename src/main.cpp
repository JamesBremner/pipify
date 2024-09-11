
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <wex.h>

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
    boundingRectangle();
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

void cRoom::pipeDoor()
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
        p1.x = d1.x;
        p1.y = d1.y - theSeperation;
        p2.x = p1.x + theSeperation;
        p2.y = p1.y;
        p3.x = p2.x;
        p3.y = p2.y - theSeperation;
        break;
    }
    std::vector<cxy> pipeSegment;
    pipeSegment.push_back(p1);
    pipeSegment.push_back(p2);
    pipeSegment.push_back(p3);
    myPipePoints.emplace_back(
        cPipeline::ePipe::door,
        pipeSegment);
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
    // ring pipe

    // construct closed polygon without doors
    cCorners corners(*this);
    std::vector<cxy> noDoors = corners.getCorners();

    std::vector<cxy> ring;
    for (int icorner = 0; icorner < noDoors.size(); icorner++)
    {
        cxy p1, bend, p2;
        if (icorner == 0)
            p1 = noDoors[noDoors.size() - 2];
        else
            p1 = noDoors[icorner - 1];

        bend = noDoors[icorner];

        if (icorner == noDoors.size() - 1)
            p2 = noDoors[1];
        else
            p2 = noDoors[icorner + 1];

        switch (corner(p1, bend, p2))
        {
        case cRoom::eCorner::tl_vex:
            bend = cxy(
                bend.x + theSeperation,
                bend.y + theSeperation);
            break;
        case cRoom::eCorner::tr_vex:
            bend = cxy(
                bend.x - theSeperation,
                bend.y + theSeperation);
            break;
        case cRoom::eCorner::br_vex:
            bend = cxy(
                bend.x - theSeperation,
                bend.y - theSeperation);
            break;
        case cRoom::eCorner::bl_vex:
            bend = cxy(
                bend.x + theSeperation,
                bend.y - theSeperation);
            break;
        }
        ring.push_back(bend);
    }
    myPipePoints.emplace_back(
        cPipeline::ePipe::ring,
        ring);

    // door pipes

    for (int doorIndex : myDoorPoints)
    {
        cxy p1 = myWallPoints[doorIndex];
        cxy p2 = myWallPoints[doorIndex + 1];
        switch (side(p1, p2))
        {
        case eMargin::top:
            p1.x += (p2.x - p1.x) / 2;
            p1.y += theSeperation;
            p2.x = p1.x;
            break;
        case eMargin::bottom:
            p1.x += (p2.x - p1.x) / 2;
            p1.y -= theSeperation;
            p2.x = p1.x;
            break;
        case eMargin::left:
            p1.y -= (p1.y - p2.y) / 2;
            p2.x += theSeperation;
            p2.y = p1.y;
            break;
        }

        std::vector<cxy> pipe = { p1, p2};
        myPipePoints.emplace_back(
            cPipeline::ePipe::door,
            pipe);
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
        pipeConvex();
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
        // myPipePoints.push_back(subRoom.pipeConvex());

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
        // myPipePoints.push_back(subRoom2.pipeConvex(startPoint.x, startPoint.y));
    }
    break;
    }
}
void cRoom::pipeConvex(int x, int y)
{
    int spiralStartCornerIndex;

    cxy spiralStartPoint(x, y);
    if (myDoorPoints.size())
    {
        pipeDoor();
        spiralStartCornerIndex = myDoorPoints[0] + 2;
        if (spiralStartCornerIndex == myWallPoints.size())
            spiralStartCornerIndex = 0;
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

    auto spiral = pipeSpiral(
        spiralStartCornerIndex);

    myPipePoints.emplace_back(
        cPipeline::ePipe::spiral,
        spiral);
}

void cRoom::boundingRectangle()
{
    myXmin = INT_MAX;
    myXmax = -1;
    myYmin = INT_MAX;
    myYmax = -1;
    for (auto &p : myWallPoints)
    {
        if (p.x < myXmin)
            myXmin = p.x;
        if (p.x > myXmax)
            myXmax = p.x;
        if (p.y < myYmin)
            myYmin = p.y;
        if (p.y > myYmax)
            myYmax = p.y;
    }
    double xrange = myXmax - myXmin;
    double yrange = myYmax - myYmin;
    myMaxDim = xrange;
    if (yrange > xrange)
        myMaxDim = yrange;
}

std::vector<cxy> cRoom::pipeSpiral(
    int startIndex)
{
    // construct closed polygon without doors
    cCorners corners(*this);
    startIndex = corners.index(startIndex);
    std::vector<cxy> noDoors = corners.getCorners();

    std::vector<cxy> spiral;

    // start from previous pipe point
    spiral.push_back(myPipePoints.back().last());

    int wallSeperation = theSeperation;

    for (int cornerIndex = startIndex; true; cornerIndex++)
    {
        if (cornerIndex == noDoors.size() - 1)
        {
            // wrap around noDoors
            cornerIndex = 0;
        }

        bool fspiralwrap = false;
        if (startIndex == 0)
        {
            if (cornerIndex == 0)
                fspiralwrap = true;
        }
        else
        {
            if (cornerIndex == startIndex - 1)
                fspiralwrap = true;
        }
        if (fspiralwrap)
        {
            // wrap around spiral
            wallSeperation += theSeperation;
            fspiralwrap = true;
        }

        cxy p1, bend, p3;
        if (cornerIndex == 0)
            p1 = noDoors[noDoors.size() - 1];
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
            if (fspiralwrap)
                bend.y -= theSeperation;
            break;
        case eCorner::br_vex:
            bend.x -= wallSeperation;
            bend.y -= wallSeperation;
            if (fspiralwrap)
                bend.x += theSeperation;
            break;
        case eCorner::bl_vex:
            bend.x += wallSeperation;
            bend.y -= wallSeperation;
            if (fspiralwrap)
                bend.y += wallSeperation;
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
        if (isSpiralComplete(spiral, wallSeperation, bend))
            return spiral;
    }

    return spiral;
}
bool cRoom::isSpiralComplete(
    std::vector<cxy> &spiral,
    int wallSeperation,
    const cxy &nextbend) const
{
    // distance squared from last pipe point to new point
    double d2 = spiral.back().dist2(nextbend);

    // std::cout << d2 <<" "<< wallSeperation<< "\n";

    if (d2 < theSeperation * theSeperation)
        return true;

    if (wallSeperation > myMaxDim / 2)
        return true;

    // keep going
    spiral.push_back(nextbend);
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

    // check that other rooms have exactly one door TID5
    for (auto &r : theHouse)
        if (r.myName != name)
            if (r.doorCount() != 1)
                throw std::runtime_error(
                    "room has more than one door");

    // TODO: check every room connected directly to furnace room TID3
}

cCorners::cCorners(const cRoom &room)
{
    myCorners.clear();
    const auto &wps = room.getWallPoints();
    const auto &dps = room.getDoorPoints();
    int nextDoorIndex = 0;

    // loop over wall points
    for (int i = 0; i < wps.size(); i++)
    {
        if (i == dps[nextDoorIndex])
        {
            // skip door points
            i++;
            i++;
            if (i >= wps.size())
                break;
            nextDoorIndex++;
            if (nextDoorIndex == dps.size())
            {
                // no more doors
                nextDoorIndex = -1;
            }
        }
        myIndices.push_back(i);
        myCorners.push_back(wps[i]);
    }
    // close polygon
    myCorners.push_back(wps[0]);
}

int cCorners::index(int wp) const
{
    auto it = std::find(
        myIndices.begin(), myIndices.end(),
        wp);
    if (it == myIndices.end())
        return -1;
    return it - myIndices.begin();
}

main()
{
    cGUI theGUI;
    return 0;
}
