#include "pipify.h"


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

    switch (side(
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
            cPipeline::eLine::door,
            spiral),
        cPipeline(
            cPipeline::ePipe::ret,
            cPipeline::eLine::door,
            spiralReturn));
}


cPipeLayer::cPipeLayer(std::vector<cRoom> &house)
    : myHouse(house)
{
    for (int roomIndex = 0; roomIndex < myHouse.size(); roomIndex++)
    {
        try
        {
            std::cout << "calculating pipe layout for room "
                      << myHouse[roomIndex].name() << "\n";

            myHouse[roomIndex].clearPipes();

            if (roomIndex == cRoom::furnaceRoomIndex())
            {
                furnaceRoom(myHouse[roomIndex]);
            }
            else if (myHouse[roomIndex].isConcave())
            {
                concave(myHouse[roomIndex]);
            }
            else
            {
                convex(myHouse[roomIndex] );
            }
        }
        catch (std::runtime_error &e)
        {
            /// report exception
            std::cout << "!!! Exception !!!" << e.what() << "\n\n";

            /// clear any pipes from this room
            myHouse[roomIndex].clearPipes();

            // continue trying other rooms
        }
    }
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
            cPipeline::eLine::spiral,
            spiral),
        cPipeline(
            cPipeline::ePipe::ret,
            cPipeline::eLine::spiral,
            spiralReturn));
}
cxy closestOnSpiral(
    cxy &point,
    const std::vector<cxy> &spiral)
{
    double dmin = INT_MAX;
    cxy ret;
    for (int i = 0; i < 5; i++)
    {
        cxy vector = spiral[i].vect(spiral[i + 1]);
        for (double f = 0; f < 1.2; f += 0.2)
        {
            cxy tst = spiral[i];
            tst.x += f * vector.x;
            tst.y += f * vector.y;
            double d = point.dist2(tst);
            if (d < dmin)
            {
                dmin = d;
                ret = tst;
            }
        }
    }
    return ret;
}


/// @brief add connecting pipelines between subrooms
/// @param subroom

void connectSpiralSpiral(
    cRoom &concaveRoom,
    std::pair<cRoom, cRoom> &subrooms)
{
    cxy noDoorStartHot = subrooms.second.getSpiralHot()[0];
    cxy noDoorStartRet = subrooms.second.getSpiralRet()[0];

    cxy doorSpiralHot = closestOnSpiral(
        noDoorStartHot,
        subrooms.first.getSpiralHot());
    cPipeline plhot(
        cPipeline::ePipe::hot,
        cPipeline::eLine::spiral2spiral,
        {doorSpiralHot, noDoorStartHot});
    concaveRoom.add(plhot);

    cxy doorSpiralRet = closestOnSpiral(
        noDoorStartRet,
        subrooms.first.getSpiralRet());
    cPipeline plret(
        cPipeline::ePipe::ret,
        cPipeline::eLine::spiral2spiral,
        {doorSpiralRet, noDoorStartRet});
    concaveRoom.add(plret);
}

void cPipeLayer::convex(cRoom &room)
{
    // std::cout << "=>pipeConvex " << myName << "\n";

    cCorners corners(room);

    int startCornerIndex;
    if (!room.doorCount())
    {
        // room has no doors

        startCornerIndex = 0;
    }
    else
    {
        // connect spiral start to pipes in door to furnace

        startCornerIndex = room.getDoorPoints()[0] - 1;
        if (startCornerIndex == -1)
            startCornerIndex = room.getWallPoints().size() - 1;

        auto cnx = connectSpiralDoor(
            corners.getCorners(),
            startCornerIndex,
            room.getDoorCenter());
        room.add(cnx.first);
        room.add(cnx.second);
    }

    auto spiral = spiralMaker(
        corners.getCorners(),
        startCornerIndex,
        room.getDoorCenter(),
        room.getMaxDim());
    room.add(spiral.first);
    room.add(spiral.second);
}


void cPipeLayer::concave( cRoom& room)
{
    // split concave room into 2 convex rooms

    auto subrooms = concaveSplit(room);

    // layout pipes in subrooms

    convex( subrooms.first );
    convex( subrooms.second );

    // connect pipes between subrooms
    connectSpiralSpiral(
        room,
        subrooms);

    // add subroom pipes to this room
    for (auto &l : subrooms.first.pipes())
        room.add(l);
    for (auto &l : subrooms.second.pipes())
        room.add(l);
}

void cPipeLayer::furnaceRoom(cRoom& room )
{
    int sepret = cRoom::seperation() / 2;

    // ring pipe

    // construct closed polygon without doors
    cCorners corners(room);
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

        switch (room.corner(p1, bend, p2))
        {
        case eCorner::tl_vex:
            bend = cxy(
                bend.x + cRoom::seperation(),
                bend.y + cRoom::seperation());
            bendReturn = cxy(
                bend.x + sepret,
                bend.y + sepret);
            break;
        case eCorner::tr_vex:
            bend = cxy(
                bend.x - cRoom::seperation(),
                bend.y + cRoom::seperation());
            bendReturn = cxy(
                bend.x - sepret,
                bend.y + sepret);
            break;
        case eCorner::br_vex:
            bend = cxy(
                bend.x - cRoom::seperation(),
                bend.y - cRoom::seperation());
            bendReturn = cxy(
                bend.x - sepret,
                bend.y - sepret);
            break;
        case eCorner::bl_vex:
            bend = cxy(
                bend.x + cRoom::seperation(),
                bend.y - cRoom::seperation());
            bendReturn = cxy(
                bend.x + sepret,
                bend.y - sepret);
            break;
        }
        ring.push_back(bend);
        ring_ret.push_back(bendReturn);
    }
    room.add(cPipeline(
        cPipeline::ePipe::hot,
        cPipeline::eLine::ring,
        ring));
    room.add(cPipeline(
        cPipeline::ePipe::ret,
        cPipeline::eLine::ring,
        ring_ret));

    // door pipes

    for (int doorIndex : room.getDoorPoints())
    {
        cxy dc = room.getDoorCenter();
        cxy p2 = dc;
        eMargin m = side(dc, room.getWallPoints()[doorIndex + 1]);
        switch (m)
        {
        case eMargin::top:
            p2.y = dc.y + cRoom::seperation();
            break;
        case eMargin::bottom:
            p2.y = dc.y - cRoom::seperation();
            break;
        case eMargin::left:
            p2.x = dc.x + cRoom::seperation();
            break;
        case eMargin::right:
            p2.x = dc.x - cRoom::seperation();
            break;
        }

        std::vector<cxy> pipe = {dc, p2};
        room.add(cPipeline(
            cPipeline::ePipe::hot,
            cPipeline::eLine::door,
            pipe));

        // door return

        pipe.clear();
        switch (m)
        {
        case eMargin::top:
            pipe.emplace_back(dc.x - cRoom::seperation(), dc.y + 1.5 * cRoom::seperation());
            pipe.emplace_back(dc.x - cRoom::seperation(), dc.y);
            break;
        case eMargin::left:
            pipe.emplace_back(dc.x + 1.5 * cRoom::seperation(), dc.y - cRoom::seperation());
            pipe.emplace_back(dc.x, dc.y - cRoom::seperation());
            break;
        case eMargin::bottom:
            pipe.emplace_back(dc.x - cRoom::seperation(), dc.y - 1.5 * cRoom::seperation());
            pipe.emplace_back(dc.x - cRoom::seperation(), dc.y);
            break;
        case eMargin::right:
            pipe.emplace_back(dc.x - 1.5 * cRoom::seperation(), dc.y + cRoom::seperation());
            pipe.emplace_back(dc.x, dc.y + cRoom::seperation());
            break;
        }
        room.add(cPipeline(
            cPipeline::ePipe::ret,
            cPipeline::eLine::door,
            pipe));
    }
}

