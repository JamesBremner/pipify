
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <wex.h>

#include "pipify.h"

#include "cGUI.h"

// define static attributes

std::vector<cRoom> cRoom::theHouse;

int cRoom::theSeperation;

int cRoom::thefurnaceRoomIndex;

double angle(
    std::vector<cxy> poly)
{
    double a = cxy::clockwise(poly.back(), poly[0], poly[0]);
    for (int i = 1; i < poly.size() - 2; i++)
    {
        a = cxy::clockwise(poly[i - 1], poly[i], poly[i + 1]) / 3.142;
        // std::cout << a << " ";
    }

    // std::cout << "\n";
    return a;
}

eCorner concavefind(
    int &index,
    const std::vector<cxy> &poly)
{
    auto co = cRoom::corner(
        poly.back(),
        poly[0],
        poly[1]);
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
        index < poly.size();
        index++)
    {
        cxy p3 = poly[index + 1];
        if (index == poly.size() - 1)
        {
            p3 = poly[0];
        }
        co = cRoom::corner(
            poly[index - 1],
            poly[index],
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

    // the poly is convex
    return eCorner::error;
}

/// @brief find edge of margin
/// @param[out] firstIndex of edge found
/// @param poly
/// @param m margin to be found
/// @return edge end points

std::pair<cxy, cxy> marginFind(
    int &firstIndex,
    const std::vector<cxy> poly,
    eMargin m)
{
    for (int ip = 0; ip < poly.size(); ip++)
    {
        if (cRoom::side(poly[ip], poly[ip + 1]) == m)
        {
            firstIndex = ip;
            return std::make_pair(poly[ip], poly[ip + 1]);
        }
    }
    throw std::runtime_error(
        "cRoom::find error");
}

/// @brief Locate the center of the first door
/// @param wall walls and doors of the room
/// @param index index of first door point
/// @return location of door center

cxy doorCenter(
    const std::vector<cxy> &wall,
    int index)
{
    cxy d1 = wall[index];
    cxy d2 = wall[index + 1];
    double widthHalf = sqrt(d1.dist2(d2)) / 2;
    cxy ret = d1;
    eMargin m = cRoom::side(d1, d2);
    switch (m)
    {
    case eMargin::top:
        ret.x += widthHalf;
        break;
    case eMargin::right:
        ret.y += widthHalf;
        break;
    case eMargin::bottom:
        ret.x -= widthHalf;
        break;
    case eMargin::left:
        ret.y -= widthHalf;
        break;
    }
    return ret;
}

std::pair<cPipeline, cPipeline> connectSpiralDoor(
    const std::vector<cxy> &polygon,
    int startCornerIndex,
    const cxy &doorCenter)
{
    std::vector<cxy> spiral, spiralReturn;

    int sep = cRoom::seperation();
    int sepret = sep / 2;

    int ip2 = startCornerIndex + 1;
    if (ip2 == polygon.size())
        ip2 = 0;

    switch (cRoom::side(
        polygon[startCornerIndex],
        polygon[ip2]))
    {
    case eMargin::top:
        spiral.push_back(doorCenter);
        spiral.emplace_back(doorCenter.x, doorCenter.y + sep);
        spiral.emplace_back(
            polygon[ip2].x - sep,
            polygon[ip2].y + sep);
        spiralReturn.emplace_back(
            doorCenter.x - sep, doorCenter.y);
        spiralReturn.emplace_back(
            doorCenter.x - sep, doorCenter.y + 1.5 * sep);
        spiralReturn.emplace_back(
            polygon[ip2].x - 1.5 * sep,
            polygon[ip2].y + 1.5 * sep);
        break;

    case eMargin::right:
        spiral.push_back(doorCenter);
        spiral.emplace_back(doorCenter.x - sep, doorCenter.y);
        spiral.emplace_back(
            polygon[ip2].x - sep,
            polygon[ip2].y - sep);
        spiralReturn.emplace_back(
            doorCenter.x, doorCenter.y - sep);
        spiralReturn.emplace_back(
            doorCenter.x - 1.5 * sep, doorCenter.y - sep);
        spiralReturn.emplace_back(
            polygon[ip2].x - 1.5 * sep,
            polygon[ip2].y - 1.5 * sep);
        break;

    case eMargin::bottom:
        spiral.push_back(doorCenter);
        spiral.emplace_back(doorCenter.x, doorCenter.y - sep);
        spiral.emplace_back(
            polygon[ip2].x + sep,
            polygon[ip2].y - sep);
        spiralReturn.emplace_back(
            doorCenter.x - sep, doorCenter.y);
        spiralReturn.emplace_back(
            doorCenter.x - sep, doorCenter.y - 1.5 * sep);
        spiralReturn.emplace_back(
            polygon[ip2].x + 1.5 * sep,
            polygon[ip2].y - 1.5 * sep);
        break;

    case eMargin::left:
        spiral.push_back(doorCenter);
        spiral.emplace_back(doorCenter.x + sep, doorCenter.y);
        spiral.emplace_back(
            polygon[ip2].x + sep,
            polygon[ip2].y + sep);
        spiralReturn.emplace_back(
            doorCenter.x, doorCenter.y + sep);
        spiralReturn.emplace_back(
            doorCenter.x + 1.5 * sep, doorCenter.y + sep);
        spiralReturn.emplace_back(
            polygon[ip2].x + 1.5 * sep,
            polygon[ip2].y + 1.5 * sep);
        break;
    }

    return std::make_pair(
        cPipeline(
            cPipeline::ePipe::hot,
            spiral),
        cPipeline(
            cPipeline::ePipe::ret,
            spiralReturn));
}
/// @brief add connectiing pipelines between subrooms
/// @param subroom

void connectSpiralSpiral(
    cRoom& concaveRoom,
    cRoom &subroom)
{

    if (subroom.doorCount())
    {
        // The connection between the subrooms must be done once
        // best done for the subroom without a door
        return;
    }

    // connect to the subroom spiral start
    cxy noDoorStartHot = subroom.pipes()[0].myLine[0];
    cxy noDoorStartRet = subroom.pipes()[1].myLine[0];

    // connect to the nearest point of the other subroom spiral
    int concaveIndex;
    eCorner concaveCorner;
    concaveRoom.getConcave( concaveIndex, concaveCorner);
    cxy doorSpiralHot = concaveRoom.getWallPoints()[concaveIndex];
    cxy doorSpiralRet = doorSpiralHot;
    switch (concaveCorner)
    {
    case eCorner::tr_cav:
        doorSpiralHot.x -= cRoom::seperation();
        doorSpiralHot.y += cRoom::seperation();
        doorSpiralRet.x = doorSpiralHot.x - cRoom::seperation() / 2;
        doorSpiralRet.y = doorSpiralHot.y + cRoom::seperation() / 2;
        break;

    case eCorner::tl_cav:
        doorSpiralHot.x += cRoom::seperation();
        doorSpiralHot.y += cRoom::seperation();
        doorSpiralRet.x = doorSpiralHot.x + cRoom::seperation() / 2;
        doorSpiralRet.y = doorSpiralHot.y + cRoom::seperation() / 2;
        break;

    case eCorner::br_cav:
        doorSpiralHot.x += cRoom::seperation();
        doorSpiralHot.y -= cRoom::seperation();
        doorSpiralRet.x = doorSpiralHot.x + cRoom::seperation() / 2;
        doorSpiralRet.y = doorSpiralHot.y - cRoom::seperation() / 2;
        break;
    }

    // add connecting pipeline to subroom
    cPipeline plhot(
        cPipeline::ePipe::hot,
        {doorSpiralHot, noDoorStartHot});
    //{doorSpiralHot, cxy(0, 0)});
    subroom.add(plhot);
    cPipeline plret(
        cPipeline::ePipe::ret,
        {doorSpiralRet, noDoorStartRet});
    subroom.add(plret);
}

/// @brief Make the pipe spiral in a convex room
/// @param polygon room with doors removed ( closed polygon )
/// @param startCornerIndex polygon index where spiral wraps around
/// @param doorCenter
/// @param maxDim rom maximum dimension extent
/// @return spiral pipeline

std::pair<cPipeline, cPipeline> spiralMaker(
    const std::vector<cxy> &polygon,
    int startCornerIndex,
    const cxy &doorCenter,
    double maxDim)
{
    int sep = cRoom::seperation();
    int sepret = sep / 2;

    // spiral to be returned
    std::vector<cxy> spiral, spiralReturn;

    // spiral.push_back(startPoints.first);
    // spiralReturn.push_back(startPoints.second);

    // starting side
    int ip2 = startCornerIndex + 1;
    if (ip2 == polygon.size())
        ip2 = 0;

    // switch (cRoom::side(
    //     polygon[startCornerIndex],
    //     polygon[ip2]))
    // {
    // case eMargin::top:
    //     spiralReturn.emplace_back(
    //         doorCenter.x - sep, doorCenter.y);
    //     spiralReturn.emplace_back(
    //         startPoint.x - sep, startPoint.y + sepret);
    //     break;
    // case eMargin::right:
    //     spiralReturn.emplace_back(
    //         doorCenter.x, doorCenter.y - sep);
    //     spiralReturn.emplace_back(
    //         startPoint.x - sepret, startPoint.y - sep);
    //     break;
    // case eMargin::bottom:
    //     spiralReturn.emplace_back(
    //         doorCenter.x - sep, doorCenter.y);
    //     spiralReturn.emplace_back(
    //         startPoint.x - sep, startPoint.y - sepret);
    //     break;
    // case eMargin::left:
    //     spiralReturn.emplace_back(
    //         doorCenter.x, doorCenter.y + sep);
    //     spiralReturn.emplace_back(
    //         startPoint.x + sepret, startPoint.y + sep);
    //     break;
    // }

    int wallsep = sep;
    for (int bendIndex = ip2; true; bendIndex++)
    {
        // wrap around the polygon
        if (bendIndex == polygon.size() - 1)
            bendIndex = 0;
        int ip1 = bendIndex - 1;
        if (ip1 == -1)
            ip1 = polygon.size() - 2;
        ip2 = bendIndex + 1;
        if (ip2 == polygon.size())
            ip2 = 1;

        cxy bendPoint = polygon[bendIndex];
        cxy bendReturn;

        bool fspiralwrap = false;
        if (bendIndex == startCornerIndex)
        {
            // the next bend will complete a loop of the spiral
            fspiralwrap = true;

            // increment wall separation
            wallsep += sep;
            if (wallsep >= maxDim / 2)
            {
                // wall separation has reached the middle of the polygon
                break;
            }
        }

        // std::cout << "spralmaker"
        //     <<" "<< bendIndex
        //     <<"\n";
        auto corner = cRoom::corner(
            polygon[ip1],
            bendPoint,
            polygon[ip2]);

        switch (corner)
        {
        case eCorner::tr_vex:
            bendPoint.x -= wallsep;
            bendPoint.y += wallsep;
            if (fspiralwrap)
                bendPoint.y -= sep;
            bendReturn.x = bendPoint.x - sepret;
            bendReturn.y = bendPoint.y + sepret;
            break;
        case eCorner::br_vex:
            bendPoint.x -= wallsep;
            bendPoint.y -= wallsep;
            if (fspiralwrap)
                bendPoint.x += sep;
            bendReturn.x = bendPoint.x - sepret;
            bendReturn.y = bendPoint.y - sepret;
            break;
        case eCorner::bl_vex:
            bendPoint.x += wallsep;
            bendPoint.y -= wallsep;
            if (fspiralwrap)
                bendPoint.y += sep;
            bendReturn.x = bendPoint.x + sepret;
            bendReturn.y = bendPoint.y - sepret;
            break;
        case eCorner::tl_vex:
            bendPoint.x += wallsep;
            bendPoint.y += wallsep;
            if (fspiralwrap)
                bendPoint.x -= sep;
            bendReturn.x = bendPoint.x + sepret;
            bendReturn.y = bendPoint.y + sepret;
            break;
        default:
            throw std::runtime_error(
                "spiral NYI");
        }

        // check for spiral vanishing
        if (spiral.size())
            if (spiral.back().dist2(bendPoint) < sep * sep)
                break;

        spiral.push_back(bendPoint);
        spiralReturn.push_back(bendReturn);
    }

    // connect hot to cold at spiral center
    spiralReturn.push_back(
        spiral.back());

    return std::make_pair(
        cPipeline(
            cPipeline::ePipe::hot,
            spiral),
        cPipeline(
            cPipeline::ePipe::ret,
            spiralReturn));
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
    else
    {
        if (subRoom1Doors.size())
        {
            if (subRoom1Wall.size() != 6)
                throw std::runtime_error(
                    "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
        }
        else
        {
            if (subRoom1Wall.size() != 4)
                throw std::runtime_error(
                    "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
        }

        if (subRoom2Doors.size())
        {
            if (subRoom2Wall.size() != 6)
                throw std::runtime_error(
                    "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
        }
        else
        {
            if (subRoom2Wall.size() != 4)
                throw std::runtime_error(
                    "concaveSplit bad wallpoint count in subroom of " + ConcaveRoom.name());
        }
    }
    // construct the subrooms
    // return them with the one with a door first

    std::pair<cRoom, cRoom> ret;
    if (subRoom1Doors.size())
    {
        ret.first = cRoom(
            ConcaveRoom.name() + "_sub1",
            subRoom1Wall,
            subRoom1Doors);
        ret.second = cRoom(
            ConcaveRoom.name() + "_sub2",
            subRoom2Wall,
            subRoom2Doors);
    }
    else
    {
        ret.first = cRoom(
            ConcaveRoom.name() + "_sub2",
            subRoom2Wall,
            subRoom2Doors);
        ret.second = cRoom(
            ConcaveRoom.name() + "_sub1",
            subRoom1Wall,
            subRoom1Doors);
    }

    return ret;
}
bool insertDoor(
    std::vector<cxy> &subRoom1Wall, std::vector<int> &subRoom1Doors,
    std::vector<cxy> &subRoom2Wall, std::vector<int> &subRoom2Doors,
    const std::vector<cxy> &roomWall,
    int doorIndex)
{
    cxy doorPoint = roomWall[doorIndex];
    for (int isb = 0; isb < subRoom1Wall.size(); isb++)
    {
        int wp2 = isb + 1;
        if (isb == 3)
            wp2 = 0;

        double d = doorPoint.dis2toline(
            subRoom1Wall[isb], subRoom1Wall[wp2]);
        if (d < 0.5)
        {
            // insert door into subroom wall
            if (wp2 == 0)
            {
                subRoom1Wall.push_back(roomWall[doorIndex]);
                subRoom1Wall.push_back(roomWall[doorIndex + 1]);
                subRoom1Doors.push_back(4);
            }
            else
            {
                subRoom1Wall.insert(
                    subRoom1Wall.begin() + isb + 1,
                    roomWall.begin() + doorIndex,
                    roomWall.begin() + doorIndex + 2);
                subRoom1Doors.push_back(isb + 1);
            }
            return true;
        }
        d = doorPoint.dis2toline(
            subRoom2Wall[isb], subRoom2Wall[wp2]);
        if (d < 0.5)
        {
            // insert door into subroom wall
            if (wp2 == 0)
            {
                subRoom2Wall.push_back(roomWall[doorIndex]);
                subRoom2Wall.push_back(roomWall[doorIndex + 1]);
                subRoom2Doors.push_back(4);
            }
            else
            {
                subRoom2Wall.insert(
                    subRoom2Wall.begin() + isb + 1,
                    roomWall.begin() + doorIndex,
                    roomWall.begin() + doorIndex + 2);
                subRoom2Doors.push_back(isb + 1);
            }
            return true;
        }
    }
    return false;
}

wall_t concaveSplitWalls(
    const cRoom &ConcaveRoom,
    std::vector<cxy> &noDoors)
{
    int concaveIndex;
    eCorner corner;
    ConcaveRoom.getConcave(concaveIndex, corner);

    // find opposite wall to concave corner
    eMargin oppositeMargin;
    switch (corner)
    {
    case eCorner::tr_cav:
    case eCorner::tl_cav:
        oppositeMargin = eMargin::bottom;
        break;
    case eCorner::br_cav:
        oppositeMargin = eMargin::top;
        break;
    }
    int firstOppositeIndex;
    auto oppositeWall = marginFind(
        firstOppositeIndex,
        noDoors,
        oppositeMargin);

    // new wall shared by subrooms
    // TODO: handle horizontal shared wall
    cxy concavePoint = ConcaveRoom.getWallDoorPoint(concaveIndex);
    auto newWall = std::make_pair(
        concavePoint,
        cxy(concavePoint.x, INT_MAX));
    cxy::isIntersection(
        newWall.second,
        newWall.first, newWall.second,
        oppositeWall.first, oppositeWall.second);

    // insert new wall point
    for (int ip = 0; ip < noDoors.size() - 1; ip++)
    {
        double d = newWall.second.dis2toline(
            noDoors[ip], noDoors[ip + 1]);
        if (d < 0.5)
        {
            noDoors.insert(
                noDoors.begin() + ip + 1,
                newWall.second);
            // if( doorIndex > ip + 1 )
            break;
        }
    }

    // ensure first new wall point is first point encountered
    // when iterating through noDoors
    std::pair<int, int> newIndices;
    const auto it = std::find(
        noDoors.begin(), noDoors.end(), newWall.first);
    newIndices.first = it - noDoors.begin();
    const auto it2 = std::find(
        noDoors.begin(), noDoors.end(), newWall.second);
    newIndices.second = it2 - noDoors.begin();

    if (newIndices.first > newIndices.second)
    {
        auto temp = newWall.first;
        newWall.first = newWall.second;
        newWall.second = temp;
    }

    return newWall;
}
std::pair<cRoom, cRoom> concaveSplit(
    const cRoom &ConcaveRoom)
{
    std::pair<cRoom, cRoom> ret;
    std::vector<cxy> subRoom1Wall, subRoom2Wall;
    std::vector<int> subRoom1Doors, subRoom2Doors;

    auto wps = ConcaveRoom.getWallPoints();
    auto dps = ConcaveRoom.getDoorPoints();

    cCorners C(ConcaveRoom);
    auto noDoors = C.getCorners();

    // new wall separating the two subrooms
    auto newWall = concaveSplitWalls(
        ConcaveRoom, noDoors);

    // which new wall endpoints to include in first subroom
    // ( the subroom that includes fitst point in concave room )
    int concaveIndex;
    eCorner corner;
    ConcaveRoom.getConcave(concaveIndex, corner);
    bool fsr1new1;
    bool fsr1new2;
    switch (corner)
    {
    case eCorner::tr_cav:
        fsr1new1 = false;
        fsr1new2 = true;
        break;
    case eCorner::tl_cav:
        fsr1new1 = true;
        fsr1new2 = false;
        break;
    case eCorner::br_cav:
        fsr1new1 = true;
        fsr1new2 = false;
        break;
    default:
        std::runtime_error(
            "concaveSplit failed");
    }

    // construct wall of subrooms
    for (int ip = 0; ip < noDoors.size() - 1; ip++)
    {
        if (!(noDoors[ip] == newWall.first))
        {
            subRoom1Wall.push_back(noDoors[ip]);
        }
        else
        {
            if (noDoors[ip] == newWall.first)
            {
                if (fsr1new1)
                    subRoom1Wall.push_back(newWall.first);
                while (true)
                {
                    ip++;

                    if (noDoors[ip] == newWall.second)
                    {
                        if (fsr1new2)
                            subRoom1Wall.push_back(noDoors[ip]);
                        break;
                    }
                }
            }
        }
    }

    for (
        int ip = std::find(
                     noDoors.begin(), noDoors.end(), newWall.first) -
                 noDoors.begin();
        ip <= std::find(
                  noDoors.begin(), noDoors.end(), newWall.second) -
                  noDoors.begin();
        ip++)
    {
        std::cout << "subroom2 " << ip
                  << " " << noDoors[ip].x
                  << " " << noDoors[ip].y << "\n";
        subRoom2Wall.push_back(noDoors[ip]);
    }

    if (subRoom1Wall.size() != 4 ||
        subRoom2Wall.size() != 4)
        throw std::runtime_error(
            "concave splitting failed " + ConcaveRoom.name());

    // insert door into subroom wall
    if (!insertDoor(
            subRoom1Wall, subRoom1Doors,
            subRoom2Wall, subRoom2Doors,
            wps, dps[0]))
        throw std::runtime_error(
            "insertDoor failed " + ConcaveRoom.name());

    // create subrooms from walls
    ret = concaveMakeSubRooms(
        ConcaveRoom,
        subRoom1Wall, subRoom1Doors,
        subRoom2Wall, subRoom2Doors);

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

    if (doorPoints.size())
        myDoorCenter = doorCenter(
            myWallPoints,
            myDoorPoints[0]);

    angle(myWallPoints);

    myConcaveCorner = isConcave(myConcaveIndex);
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

eCorner cRoom::isConcave(int &index) const
{
    cCorners corners(*this);
    auto ret = concavefind(index, corners.getCorners());
    if (ret == eCorner::error)
        index = -1;
    else
        index = corners.wpIndex(index);
    return ret;
}

cxy cRoom::pipeDoor()
{
    cxy p1, p2, p3;

    // start at the first door
    // There should be only one door
    p1 = myDoorCenter;
    cxy d1 = myWallPoints[myDoorPoints[0]];
    cxy d2 = myWallPoints[myDoorPoints[0] + 1];

    // switch on side where door is placed
    switch (side(d1, d2))
    {
    case eMargin::top:
        p2.x = p1.x;
        p2.y = p1.y + theSeperation;
        p3.x = d2.x;
        p3.y = p2.y;
        break;
    case eMargin::right:
        p2.x = p1.x - theSeperation;
        p2.y = p1.y;
        p3.x = p2.x;
        p3.y = d2.y;
        break;
    case eMargin::bottom:
        p2.x = p1.x;
        p2.y = p1.y - theSeperation;
        p3.x = p2.x - theSeperation;
        p3.y = p2.y;
        break;
    case eMargin::left:
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
        cPipeline::ePipe::hot,
        pipeSegment);

    return p2;
}

void cRoom::pipeHouse()
{
    if (!theHouse.size())
        return;

    for (int roomIndex = 0; roomIndex < theHouse.size(); roomIndex++)
    {
        try
        {
            std::cout << "calculating pipe layout for room "
                      << theHouse[roomIndex].myName << "\n";

            theHouse[roomIndex].clearPipes();

            if (roomIndex == thefurnaceRoomIndex)
            {
                theHouse[roomIndex].pipefurnaceRoom();
            }
            else
            {
                theHouse[roomIndex].pipe();
            }
        }
        catch (std::runtime_error &e)
        {
            /// report exception
            std::cout << "!!! Exception !!!" << e.what() << "\n\n";

            /// clear any pipes from this room
            theHouse[roomIndex].clearPipes();

            // continue trying other rooms
        }
    }
}

void cRoom::pipefurnaceRoom()
{
    int sepret = theSeperation / 2;

    // ring pipe

    // construct closed polygon without doors
    cCorners corners(*this);
    std::vector<cxy> noDoors = corners.getCorners();

    std::vector<cxy> ring, ring_ret;
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

        cxy bendReturn;

        switch (corner(p1, bend, p2))
        {
        case eCorner::tl_vex:
            bend = cxy(
                bend.x + theSeperation,
                bend.y + theSeperation);
            bendReturn = cxy(
                bend.x + sepret,
                bend.y + sepret);
            break;
        case eCorner::tr_vex:
            bend = cxy(
                bend.x - theSeperation,
                bend.y + theSeperation);
            bendReturn = cxy(
                bend.x - sepret,
                bend.y + sepret);
            break;
        case eCorner::br_vex:
            bend = cxy(
                bend.x - theSeperation,
                bend.y - theSeperation);
            bendReturn = cxy(
                bend.x - sepret,
                bend.y - sepret);
            break;
        case eCorner::bl_vex:
            bend = cxy(
                bend.x + theSeperation,
                bend.y - theSeperation);
            bendReturn = cxy(
                bend.x + sepret,
                bend.y - sepret);
            break;
        }
        ring.push_back(bend);
        ring_ret.push_back(bendReturn);
    }
    myPipePoints.emplace_back(
        cPipeline::ePipe::hot,
        ring);
    myPipePoints.emplace_back(
        cPipeline::ePipe::ret,
        ring_ret);

    // door pipes

    for (int doorIndex : myDoorPoints)
    {
        cxy dc = doorCenter(myWallPoints, doorIndex);
        cxy p2 = dc;
        eMargin m = side(dc, myWallPoints[doorIndex + 1]);
        switch (m)
        {
        case eMargin::top:
            p2.y = dc.y + theSeperation;
            break;
        case eMargin::bottom:
            p2.y = dc.y - theSeperation;
            break;
        case eMargin::left:
            p2.x = dc.x + theSeperation;
            break;
        case eMargin::right:
            p2.x = dc.x - theSeperation;
            break;
        }

        std::vector<cxy> pipe = {dc, p2};
        myPipePoints.emplace_back(
            cPipeline::ePipe::hot,
            pipe);

        // door return

        pipe.clear();
        switch (m)
        {
        case eMargin::top:
            pipe.emplace_back(dc.x - theSeperation, dc.y + 1.5 * theSeperation);
            pipe.emplace_back(dc.x - theSeperation, dc.y);
            break;
        case eMargin::left:
            pipe.emplace_back(dc.x + 1.5 * theSeperation, dc.y - theSeperation);
            pipe.emplace_back(dc.x, dc.y - theSeperation);
            break;
        case eMargin::bottom:
            pipe.emplace_back(dc.x - theSeperation, dc.y - 1.5 * theSeperation);
            pipe.emplace_back(dc.x - theSeperation, dc.y);
            break;
        case eMargin::right:
            pipe.emplace_back(dc.x - 1.5 * theSeperation, dc.y + theSeperation);
            pipe.emplace_back(dc.x, dc.y + theSeperation);
            break;
        }
        myPipePoints.emplace_back(
            cPipeline::ePipe::ret,
            pipe);
    }
}
void cRoom::pipe()
{
    if (myConcaveIndex >= 0)
        pipeConcave();
    else
        pipeConvex();
}

void cRoom::pipeConcave()
{
    // split concave room into 2 convex rooms

    auto subrooms = concaveSplit(*this);

    // layout pipes in subrooms

    subrooms.first.pipeConvex();
    subrooms.second.pipeConvex();

    // connect pipes between subrooms
    connectSpiralSpiral(
        *this,
        subrooms.first);
    connectSpiralSpiral(
        *this,
        subrooms.second);

    // add subroom pipes to this room
    for (auto &l : subrooms.first.pipes())
        add(l);
    for (auto &l : subrooms.second.pipes())
        add(l);

}

void cRoom::pipeConvex()
{
    // std::cout << "=>pipeConvex " << myName << "\n";

    cCorners corners(*this);

    int startCornerIndex;
    if (!myDoorPoints.size())
    {
        // room has no doors

        startCornerIndex = 0;
    }
    else
    {
        // connect spiral start to pipes in door to furnace

        startCornerIndex = myDoorPoints[0] - 1;
        if (startCornerIndex == -1)
            startCornerIndex = myWallPoints.size() - 1;

        auto cnx = connectSpiralDoor(
            corners.getCorners(),
            startCornerIndex,
            myDoorCenter);
        myPipePoints.push_back(cnx.first);
        myPipePoints.push_back(cnx.second);
    }

    auto spiral = spiralMaker(
        corners.getCorners(),
        startCornerIndex,
        myDoorCenter,
        myMaxDim);

    myPipePoints.push_back(spiral.first);
    myPipePoints.push_back(spiral.second);
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
