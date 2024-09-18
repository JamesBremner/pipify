
#include <fstream>
#include <sstream>
#include <iostream>

#include <wex.h>

#include "pipify.h"

#include "cGUI.h"

// define static attributes

std::vector<cRoom> cRoom::theHouse;

int cRoom::theSeperation;

int cRoom::thefurnaceRoomIndex;

std::vector<cxy> spiralMaker(
    const std::vector<cxy> &polygon,
    int startCornerIndex,
    const cxy &startWallPoint,
    double maxDim)
{
    // spiral to be returned
    std::vector<cxy> ret;

    // starting side
    cxy p1, p2;
    int ip2 = startCornerIndex + 1;
    if (ip2 == polygon.size())
        ip2 = 0;
    eMargin m = cRoom::side(
        polygon[startCornerIndex],
        polygon[ip2]);

    // first spiral point
    int sep = cRoom::seperation();
    cxy first(startWallPoint);
    switch (m)
    {
    case eMargin::top:
        first.x += sep;
        first.y += sep;
        break;
    default:
        throw std::runtime_error(
            "spiral NYI");
    }
    ret.push_back(first);

    int wallsep = sep;
    for (int bendIndex = ip2; true; bendIndex++)
    {
        if (bendIndex == polygon.size() - 1)
            bendIndex = 0;
        int ip1 = bendIndex - 1;
        if (ip1 == -1)
            ip1 = polygon.size() - 1;
        ip2 = bendIndex + 1;
        if (ip2 == polygon.size())
            ip2 = 1;

        cxy bendPoint = polygon[bendIndex];

        bool fspiralwrap = false;
        if (bendIndex == startCornerIndex)
        {
            // the next bend will complete a loop of the spiral
            fspiralwrap = true;

            // increment wall separation
            wallsep += sep;
            if (wallsep > maxDim / 2)
            {
                // wall separation has reached the middle of the polygon
                break;
            }
        }

        auto corner = cRoom::corner(
            polygon[ip1],
            bendPoint,
            polygon[ip2]);

        switch (corner)
        {
        case eCorner::tr_vex:
            bendPoint.x -= wallsep;
            bendPoint.y += wallsep;
            break;
        case eCorner::br_vex:
            bendPoint.x -= wallsep;
            bendPoint.y -= wallsep;
            break;
        case eCorner::bl_vex:
            bendPoint.x += wallsep;
            bendPoint.y -= wallsep;
            break;
        case eCorner::tl_vex:
            bendPoint.x += wallsep;
            bendPoint.y += wallsep;
            if (fspiralwrap)
                bendPoint.x -= sep;
            break;
        default:
            throw std::runtime_error(
                "spiral NYI");
        }

        ret.push_back(bendPoint);

        // break;
    }

    return ret;
}

cRoom::cRoom(
    const std::string &name,
    const std::vector<cxy> wallPoints,
    const std::vector<int> doorPoints)
    : myName(name)
{
    if (wallPoints.size() < 3)
    {
        throw std::runtime_error(
            "cRoom constructer bad wall point count");
    }
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

void cRoom::clearHousePipes()
{
    for (auto &room : theHouse)
        room.clearPipes();
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

eMargin cRoom::side(const cxy &p1, const cxy &p2)
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

eCorner cRoom::corner(
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

eCorner cRoom::isConcave(int &index) const
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
        index < myWallPoints.size();
        index++)
    {
        if (std::find(myDoorPoints.begin(), myDoorPoints.end(), index) != myDoorPoints.end())
        {
            // skip doors
            index++;
            continue;
        }
        cxy p3 = myWallPoints[index + 1];
        if (index == myWallPoints.size() - 1)
        {
            p3 = myWallPoints[0];
        }
        co = corner(
            myWallPoints[index - 1],
            myWallPoints[index],
            p3);
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
    // There should be only one door
    cxy d1 = myWallPoints[myDoorPoints[0]];
    cxy d2 = myWallPoints[myDoorPoints[0] + 1];

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
        p1.y = d1.y + theSeperation;
        p2.x = p1.x - theSeperation;
        p2.y = p1.y;
        p3.x = p2.x;
        p3.y = d2.y;
        break;
    case eMargin::bottom:
        p1.x = d1.x - theSeperation;
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
    if (!theHouse.size())
        return;

    for (int ri = 0; ri < theHouse.size(); ri++)
    {
        try
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
        catch (std::runtime_error &e)
        {
            std::cout << "Exception " << e.what() << "\n";
            theHouse[ri].clearPipes();
        }
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
        case eCorner::tl_vex:
            bend = cxy(
                bend.x + theSeperation,
                bend.y + theSeperation);
            break;
        case eCorner::tr_vex:
            bend = cxy(
                bend.x - theSeperation,
                bend.y + theSeperation);
            break;
        case eCorner::br_vex:
            bend = cxy(
                bend.x - theSeperation,
                bend.y - theSeperation);
            break;
        case eCorner::bl_vex:
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
        case eMargin::right:
            p1.y -= (p1.y - p2.y) / 2;
            p2.x -= theSeperation;
            p2.y = p1.y;
            break;
        }

        std::vector<cxy> pipe = {p1, p2};
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
        concavePipe(concaveIndex);
        break;

    case eCorner::error:
    default:
        pipeConvex();
        break;
    }
}
std::pair<cRoom, cRoom> concaveMakeSubRooms(
    const cRoom &ConcaveRoom,
    const std::vector<cxy> &subRoom1Wall,
    const std::vector<int> &subRoom1Doors,
    const std::vector<cxy> &subRoom2Wall,
    const std::vector<int> &subRoom2Doors)
{
    // check subroom sanity
    if ((!subRoom1Doors.size()) && (!subRoom2Doors.size()))
        throw std::runtime_error(
            "concaveSplit no door in either subroom of " + ConcaveRoom.name());
    if (subRoom1Doors.size())
    {
        if (subRoom1Wall.size() != 6)
            throw std::runtime_error(
                "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
    }
    else if (subRoom1Wall.size() != 4)
        throw std::runtime_error(
            "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
    if (subRoom2Doors.size())
    {
        if (subRoom2Wall.size() != 6)
            throw std::runtime_error(
                "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
    }
    else if (subRoom2Wall.size() != 4)
        throw std::runtime_error(
            "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());

    // construct the subrooms
    std::pair<cRoom, cRoom> ret;
    ret.first = cRoom(
        ConcaveRoom.name() + "_sub1",
        subRoom1Wall,
        subRoom1Doors);
    ret.second = cRoom(
        ConcaveRoom.name() + "_sub2",
        subRoom2Wall,
        subRoom2Doors);

    return ret;
}

std::pair<cRoom, cRoom> concaveSplit(
    const cRoom &ConcaveRoom,
    cxy &joinPoint)
{
    std::pair<cRoom, cRoom> ret;
    std::pair<cxy, cxy> oppositeWall, newWall;
    int concaveIndex;
    std::vector<cxy> subRoom1Wall, subRoom2Wall;
    std::vector<int> subRoom1Doors, subRoom2Doors;

    auto wps = ConcaveRoom.getWallPoints();
    auto dps = ConcaveRoom.getDoorPoints();

    eCorner corner = ConcaveRoom.isConcave(concaveIndex);
    cxy concavePoint = ConcaveRoom.getWallDoorPoint(concaveIndex);
    switch (corner)
    {
    case eCorner::tr_cav:
        oppositeWall = ConcaveRoom.find(eMargin::bottom);
        newWall = std::make_pair(
            concavePoint,
            cxy(concavePoint.x, INT_MAX));
        cxy::isIntersection(
            newWall.second,
            newWall.first, newWall.second,
            oppositeWall.first, oppositeWall.second);

        for (int ip = 0; ip < wps.size(); ip++)
        {
            if (!(wps[ip] == newWall.first))
            {
                subRoom1Wall.push_back(wps[ip]);
                if (ip == dps[0])
                {
                    subRoom1Doors.push_back(subRoom1Wall.size() - 1);
                    std::cout << "tr_cav subroom1 door " << subRoom1Wall.back().x << " " << subRoom1Wall.back().y << "\n";
                }
            }
            else
            {
                while (true)
                {
                    ip++; // skip wall points to be cut off by extra wall
                    if (ip >= wps.size())
                        throw std::runtime_error("pipeConcave subroom error");

                    if (wps[ip] == oppositeWall.first)
                    {
                        subRoom1Wall.push_back(newWall.second);
                        break; // continue for remaining wall points
                    }
                }
            }
        }

        for (int ip = concaveIndex; true; ip++)
        {
            subRoom2Wall.push_back(wps[ip]);

            if (wps[ip] == oppositeWall.first)
            {
                subRoom2Wall.push_back(newWall.second);
                break;
            }
        }

        joinPoint = wps[concaveIndex];
        joinPoint.x -= cRoom::seperation();
        joinPoint.y += cRoom::seperation();

        break;

    case eCorner::tl_cav:
        oppositeWall = ConcaveRoom.find(eMargin::bottom);
        newWall = std::make_pair(
            concavePoint,
            cxy(concavePoint.x, INT_MAX));
        cxy::isIntersection(
            newWall.second,
            newWall.first, newWall.second,
            oppositeWall.first, oppositeWall.second);

        for (int ip = 0; ip < wps.size(); ip++)
        {
            subRoom1Wall.push_back(wps[ip]);
            if (ip == dps[0])
            {
                subRoom1Doors.push_back(subRoom1Wall.size() - 1);
            }

            if (wps[ip] == oppositeWall.first) // !!!!!!
            {

                while (true)
                {
                    ip++; // skip wall points to be cut off by extra wall
                    if (ip >= wps.size())
                        throw std::runtime_error("pipeConcave subroom error");

                    if (ip == concaveIndex)
                    {
                        subRoom1Wall.push_back(newWall.second);
                        break;
                    }
                }
                if (ip == concaveIndex)
                    break;
            }
        }

        subRoom2Wall.clear();
        subRoom2Wall.push_back(newWall.first);
        subRoom2Wall.push_back(newWall.second);
        for (
            int ip = ConcaveRoom.getWallDoorIndex(oppositeWall.second);
            ip < concaveIndex;
            ip++)
        {
            subRoom2Wall.push_back(ConcaveRoom.getWallDoorPoint(ip));
            if (ip == dps[0])
            {
                subRoom2Doors.push_back(subRoom2Wall.size() - 1);
            }
        }

        joinPoint = newWall.second;
        joinPoint.x += cRoom::seperation();
        joinPoint.y -= cRoom::seperation();

        break;

    default:
        throw std::runtime_error(
            "Concave room pipe layout NYI.  see https://github.com/JamesBremner/pipify/issues/8");
    }

    return concaveMakeSubRooms(
        ConcaveRoom,
        subRoom1Wall, subRoom1Doors,
        subRoom2Wall, subRoom2Doors);
}

void cRoom::concavePipe(int concaveIndex)
{
    // split concave room into 2 convex rooms
    cxy joinPoint;
    auto subrooms = concaveSplit(*this, joinPoint);

    // pipe first subroom
    subrooms.first.pipeConvex();
    for (auto &l : subrooms.first.myPipePoints)
        myPipePoints.push_back(l);

    // pipe 2nd subroom
    subrooms.second.pipeConvex(joinPoint.x, joinPoint.y);
    for (auto &l : subrooms.second.myPipePoints)
        myPipePoints.push_back(l);

    return;
}

void cRoom::pipeConvex(int x, int y)
{
    int spiralStartCornerIndex;

    cxy spiralStartPoint(x, y);
    if (myDoorPoints.size())
    {
        // layout pipes through the door to the furnace room
        pipeDoor();

        spiralStartCornerIndex = myDoorPoints[0] + 2;
        if (spiralStartCornerIndex == myWallPoints.size())
            spiralStartCornerIndex = 0;

        // start where the door pipes finished
        spiralStartPoint = myPipePoints.back().last();
    }
    else
    {
        // there is no door
        // this must be a subroom, part of a concave room
        // start at nearest pipe in first subroom

        spiralStartCornerIndex = 1;
    }
    cCorners corners(*this);

    int startCornerIndex = myDoorPoints[0] - 1;
    if (startCornerIndex == -1)
        startCornerIndex = myWallPoints.size() - 1;

    auto spiral = spiralMaker(
        corners.getCorners(),
        startCornerIndex,
        myWallPoints[myDoorPoints[0]],
        myMaxDim);

    // startIndex = corners.index(startIndex);
    // std::vector<cxy> noDoors = corners.getCorners();
    // )
    //     spiralStartCornerIndex,
    //     spiralStartPoint);

    // auto spiral = pipeSpiral(
    //     spiralStartCornerIndex,
    //     spiralStartPoint);

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
    int startIndex,
    const cxy &startPoint)
{
    std::vector<cxy> spiral;

    // construct closed polygon without doors
    cCorners corners(*this);
    startIndex = corners.index(startIndex);
    std::vector<cxy> noDoors = corners.getCorners();

    // start spiral

    spiral.push_back(startPoint);

    int wallSeperation = theSeperation;

    for (int cornerIndex = startIndex; true; cornerIndex++)
    {
        if (cornerIndex == noDoors.size() - 1)
        {
            // std::cout << "// wrap around noDoors";
            cornerIndex = 0;
        }

        bool fspiralwrap = false;
        if (spiral.size() > 1)
        {
            // the spiral contains more than just the start point

            if (startIndex == 0)
            {
                if (cornerIndex == noDoors.size() - 2)
                    fspiralwrap = true;
            }
            else
            {
                if (cornerIndex == startIndex - 1)
                    fspiralwrap = true;
            }
        }
        if (fspiralwrap)
        {
            // std::cout << "// wrap around spiral\n";
            wallSeperation += theSeperation;
            fspiralwrap = true;
        }

        cxy p1, bend, p3;
        if (cornerIndex == 0)
            p1 = noDoors[noDoors.size() - 2];
        else
        {
            p1 = noDoors[cornerIndex - 1];
        }
        bend = noDoors[cornerIndex];
        p3 = noDoors[cornerIndex + 1];

        // std::cout << cornerIndex
        //           << " " << myName << " " << wallSeperation
        //           << " " << p1.x << " " << p1.y
        //           << ", " << bend.x << " " << bend.y
        //           << ", " << p3.x << " " << p3.y
        //           << "\n";

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
                bend.y += theSeperation;
            break;
        case eCorner::tl_vex:
            bend.x += wallSeperation;
            bend.y += wallSeperation;
            if (fspiralwrap)
                bend.x -= theSeperation;
            break;
        default:
            throw std::runtime_error(
                "pipeSpiral bad corner " + name());
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
    if (!spiral.size())
    {
        spiral.push_back(nextbend);
        return false;
    }

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

    furnaceRoom(furnaceRoomName);
}

void cRoom::furnaceRoom(const std::string &name)
{
    // error if no furnace room specified
    if (!name.length())
        throw std::runtime_error("Unspecified furnace room");

    // allow house with no furnace room only when testing
    if (name == "test")
    {
        thefurnaceRoomIndex = -1;
        return;
    }

    // check that the furnace room walls and doors have been specified
    auto it = std::find_if(theHouse.begin(), theHouse.end(),
                           [&](cRoom &room) -> bool
                           {
                               return (room.myName == name);
                           });
    if (it == theHouse.end())
        throw std::runtime_error("Unspecified furnace room " + name);

    // store index to furnace room
    thefurnaceRoomIndex = it - theHouse.begin();

    // check that there are enough doors in the furnace room
    // to reach every other room
    if (theHouse.size() - 1 > theHouse[thefurnaceRoomIndex].doorCount())
        throw std::runtime_error("Not enough doors in furnace room");

    // check that other rooms have exactly one door that matched a furnace room door
    for (auto &r : theHouse)
        if (r.myName != name)
        {
            if (r.doorCount() != 1)
                throw std::runtime_error(
                    "every room must have exactly one door");
            cxy roomDoor = r.getWallPoints()[r.getDoorPoints()[0]];
            for (
                int ifurnaceDoor = theHouse[thefurnaceRoomIndex].getDoorPoints()[0];
                ifurnaceDoor < theHouse[thefurnaceRoomIndex].getDoorPoints().size();
                ifurnaceDoor++)
            {
                cxy furnaceDoor = theHouse[thefurnaceRoomIndex].getWallPoints()[ifurnaceDoor];
                if (roomDoor == furnaceDoor)
                {
                }
            }
        }

    // TODO: check every room connected directly to furnace room TID3
}

cCorners::cCorners(const cRoom &room)
{
    myCorners.clear();

    auto dps = room.getDoorPoints();
    int nextDoorIndex = 0;
    if (!dps.size())
    {
        // room has no doors
        // e.g subroom in a concave room
        dps.push_back(-1);
    }

    const auto &wps = room.getWallPoints();

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
