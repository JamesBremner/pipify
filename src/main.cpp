
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

cPolygon::cPolygon(const std::vector<cxy> &vertices)
    : myVertices(vertices)
{
    // ensure polygon is open
    if (myVertices[0] == myVertices.back())
        myVertices.erase(myVertices.end() - 1);

    // ensure polygon is at least a triangle
    if (myVertices.size() < 3)
        throw std::runtime_error(
            " too few vertices in polygon");

    // ensure clockwise
    if( ! isClockwise() )
        throw std::runtime_error(
            "polygon not clockwise"        );
}

eMargin cPolygon::margin(int index) const
{
    auto s = side( index );
    return margin(s.first,s.second);
}

wall_t cPolygon::side( int index ) const
{
    if (0 > index || index > myVertices.size() - 1)
        throw std::runtime_error(
            "cPolygon::margin bad parameter");
    cxy p1 = myVertices[index];
    cxy p2;
    if (index == myVertices.size() - 1)
        p2 = myVertices[0]; // wrap around for open polygon
    else
        p2 = myVertices[index + 1];
    return std::make_pair( p1,p2);
}

cxy cPolygon::vertex(int index) const
{
    if (0 > index || index > myVertices.size() - 1)
        throw std::runtime_error(
            "cPolygon::margin bad parameter");
    return myVertices[index];
}

eMargin cPolygon::margin(const cxy &p1, const cxy &p2)
{
    if (p1.x == p2.x)
        if (p1.y < p2.y)
            return eMargin::right;
        else
            return eMargin::left;
    else if (p1.y == p2.y)
        if (p1.x < p2.x)
            return eMargin::top;
        else
            return eMargin::bottom;
    else
        throw std::runtime_error(
            "margin bad parameter");
}

bool cPolygon::isClockwise() const
{
    double sum = 0;
    for( int i = 0; i < myVertices.size(); i++ )
    {
        auto s = side(i);

        //  (x2 − x1)(y2 + y1)
        sum += ( s.second.x - s.first.x ) *
            (s.second.y + s.first.y);
    }

    /* a -ve sum means polygon is clockwise
    This is the reverse of the stackoverflow example
    because using the windows standard co-ords
    where y is 0 at the top of the screen
    and y increases as it goes down the screen
    */
    return ( sum < 0 );
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
        if (cPolygon::margin(poly[ip], poly[ip + 1]) == m)
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
    eMargin m = cPolygon::margin(d1, d2);
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

const std::vector<cxy> &cRoom::getSpiralHot() const
{
    for (auto &p : myPipePoints)
    {
        if (p.myType == cPipeline::ePipe::hot &&
            p.myLineType == cPipeline::eLine::spiral)
            return p.myLine;
    }
    throw std::runtime_error(
        "getSpiralHot not found");
}
const std::vector<cxy> &cRoom::getSpiralRet() const
{
    for (auto &p : myPipePoints)
    {
        if (p.myType == cPipeline::ePipe::ret &&
            p.myLineType == cPipeline::eLine::spiral)
            return p.myLine;
    }
    throw std::runtime_error(
        "getSpiralHot not found");
}

eCorner cRoom::corner(
    const cxy &p1,
    const cxy &p2,
    const cxy &p3)
{
    eMargin m1 = cPolygon::margin(p1, p2);
    eMargin m2 = cPolygon::margin(p2, p3);
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

    // switch on margin where door is placed
    switch (cPolygon::margin(d1, d2))
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
        cPipeline::eLine::door,
        pipeSegment);

    return p2;
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
