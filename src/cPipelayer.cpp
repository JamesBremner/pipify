#include <cmath>
#include "pipify.h"

/// @brief Connect pipe spiral with the furnace pipes in the doorway
/// @param polygon room walls without doors
/// @param startCornerIndex
/// @param doorCenter
/// @return hot and return pipes making the connection

std::pair<cPipeline, cPipeline> connectSpiralDoor(
    const cPolygon &polygon,
    int startCornerIndex,
    const cxy &doorCenter)
{
    std::vector<cxy> hotPipe, retPipe;

    // separation between hot and return pipes
    int sep = cRoom::seperation();
    int sep2 = sep / 2;

    // room corner on other side of door
    cxy vertex2 = polygon.vertex(startCornerIndex + 1);

    switch (polygon.margin(startCornerIndex))
    {
    case eMargin::top:
        hotPipe.push_back(doorCenter);
        hotPipe.emplace_back(doorCenter.x, doorCenter.y + sep);
        hotPipe.emplace_back(
            vertex2.x - sep,
            vertex2.y + sep);
        retPipe.emplace_back(
            doorCenter.x - sep2, doorCenter.y);
        retPipe.emplace_back(
            doorCenter.x - sep2, doorCenter.y + 1.5 * sep);
        retPipe.emplace_back(
            vertex2.x - 1.5 * sep,
            vertex2.y + 1.5 * sep);
        break;

    case eMargin::right:
        hotPipe.push_back(doorCenter);
        hotPipe.emplace_back(doorCenter.x - sep, doorCenter.y);
        hotPipe.emplace_back(
            vertex2.x - sep,
            vertex2.y - sep);
        retPipe.emplace_back(
            doorCenter.x, doorCenter.y - sep2);
        retPipe.emplace_back(
            doorCenter.x - 1.5 * sep, doorCenter.y - sep2);
        retPipe.emplace_back(
            vertex2.x - 1.5 * sep,
            vertex2.y - 1.5 * sep);
        break;

    case eMargin::bottom:
        hotPipe.push_back(doorCenter);
        hotPipe.emplace_back(doorCenter.x, doorCenter.y - sep);
        hotPipe.emplace_back(
            vertex2.x + sep,
            vertex2.y - sep);
        retPipe.emplace_back(
            doorCenter.x - sep2, doorCenter.y);
        retPipe.emplace_back(
            doorCenter.x - sep2, doorCenter.y - 1.5 * sep);
        retPipe.emplace_back(
            vertex2.x + 1.5 * sep,
            vertex2.y - 1.5 * sep);
        break;

    case eMargin::left:
        hotPipe.push_back(doorCenter);
        hotPipe.emplace_back(doorCenter.x + sep, doorCenter.y);
        hotPipe.emplace_back(
            vertex2.x + sep,
            vertex2.y + sep);
        retPipe.emplace_back(
            doorCenter.x, doorCenter.y + sep2);
        retPipe.emplace_back(
            doorCenter.x + 1.5 * sep, doorCenter.y + sep2);
        retPipe.emplace_back(
            vertex2.x + 1.5 * sep,
            vertex2.y + 1.5 * sep);
        break;
    }

    return std::make_pair(
        cPipeline(
            cPipeline::ePipe::hot,
            cPipeline::eLine::door,
            hotPipe),
        cPipeline(
            cPipeline::ePipe::ret,
            cPipeline::eLine::door,
            retPipe));
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
            else if (cRoom::loop())
            {
                loop(myHouse[roomIndex]);
            }
            else
            {
                if (myHouse[roomIndex].isConcave())
                {
                    concave(myHouse[roomIndex]);
                }
                else
                {
                    convex(myHouse[roomIndex]);
                }
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

    // starting margin
    int ip2 = startCornerIndex + 1;
    if (ip2 == polygon.size())
        ip2 = 0;

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
    int closestSideIndex,
    const cxy &point,
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
                closestSideIndex = i;
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
    cxy secondSpiralHotStart = subrooms.second.getSpiralHot()[0];
    cxy noDoorStartRet = subrooms.second.getSpiralRet()[0];
    int closestSideIndex;
    cxy firstSpiralHotNearest = closestOnSpiral(
        closestSideIndex,
        secondSpiralHotStart,
        subrooms.first.getSpiralHot());
    if (firstSpiralHotNearest.x != secondSpiralHotStart.x)
    {
        firstSpiralHotNearest.y = secondSpiralHotStart.y;
    }
    cPipeline plhot(
        cPipeline::ePipe::hot,
        cPipeline::eLine::spiral2spiral,
        {firstSpiralHotNearest, secondSpiralHotStart});
    concaveRoom.add(plhot);

    cxy doorSpiralRet = closestOnSpiral(
        closestSideIndex,
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
// add subroom pipes to a room
void addSubrooms(
    cRoom &room,
    std::pair<cRoom, cRoom> &subrooms)
{
    for (auto &l : subrooms.first.pipes())
        room.add(l);
    for (auto &l : subrooms.second.pipes())
        room.add(l);
}

void cPipeLayer::concave(cRoom &room)
{
    // split concave room into 2 convex rooms
    auto subrooms = concaveSplit(room);

    // layout pipes in subrooms
    convex(subrooms.first);
    convex(subrooms.second);

    // connect pipes between subrooms
    connectSpiralSpiral(
        room,
        subrooms);

    // add subroom pipes to this room
    addSubrooms(room, subrooms);
}

void connectLoopDoor(
    cRoom &room,
    std::vector<cxy> &loop)
{
    if (!room.doorCount())
        return;

    cxy dc = room.getDoorCenter();

    double df = dc.dist2(loop[0]);
    double dl = dc.dist2(loop.back());
    double dd = df;
    cxy cnxhot = loop[0];
    cxy cnxret = loop.back();
    if (dl < df)
    {
        dd = dl;
        cnxhot = loop.back();
        cnxret = loop[0];
    }

    auto doorIndex = room.getDoorPoints()[0];

    // location of return pipe in doorway
    cxy dret = dc;
    eMargin doorMargin = cPolygon::margin(
        room.getWallPoints()[doorIndex],
        room.getWallPoints()[doorIndex + 1]);
    switch (doorMargin)
    {
    case eMargin::top:
        dret.x -= room.seperation() / 2;
        break;
    case eMargin::bottom:
        dret.x -= room.seperation() / 2;
        break;
    case eMargin::left:
        dret.y += room.seperation() / 2;
        break;
    case eMargin::right:
        dret.y -= room.seperation() / 2;
        break;
    default:
        throw std::runtime_error("connectLoopDoor error");
    }

    // corner closest to door and return loop end
    cxy doorWall1 = room.getWallPoints()[doorIndex - 1];
    cxy doorWall2 = room.getWallPoints()[doorIndex + 2];
    double dist1 = doorWall1.dist2(dret);
    double dist2 = doorWall2.dist2(dret);
    cxy cornerRet = doorWall1;
    eMargin retMargin;
    switch (doorMargin)
    {
    case eMargin::top:
        retMargin = eMargin::left;
        break;
    case eMargin::right:
        retMargin = eMargin::top;
        break;
    case eMargin::bottom:
        retMargin = eMargin::right;
        break;
    case eMargin::left:
        retMargin = eMargin::bottom;
        break;
    }
    if (dist2 < dist1)
    {
        cornerRet = doorWall2;
        switch (doorMargin)
        {
        case eMargin::top:
            retMargin = eMargin::right;
            break;
        case eMargin::right:
            retMargin = eMargin::bottom;
            break;
        case eMargin::bottom:
            retMargin = eMargin::left;
            break;
        case eMargin::left:
            retMargin = eMargin::top;
            break;
        }
    }

    room.add(
        cPipeline(
            cPipeline::ePipe::hot,
            cPipeline::eLine::door,
            {cnxhot, dc}));

    std::vector<cxy> retloop;

    cxy p1 = cnxret;
    cxy p2 = cornerRet;
    cxy p3 = dret;
    switch (retMargin)
    {
    case eMargin::top:
        p1.y += 0.5 * cRoom::seperation();
        p2.x -= cRoom::seperation() / 2;
        p2.y += cRoom::seperation() / 2;
        p3.x -= cRoom::seperation() / 2;
        break;
    case eMargin::right:
        p1.x += cRoom::seperation() / 2;
        break;
    case eMargin::left:
        p1.x -= 0.5 * cRoom::seperation();
        p2.x += cRoom::seperation() / 2;
        p2.y -= cRoom::seperation() / 2;
        p3.y -= cRoom::seperation() / 2;
        break;
    }
    retloop.push_back(cnxret);
    retloop.push_back(p1);
    retloop.push_back(p2);
    retloop.push_back(p3);
    retloop.push_back(dret);

    room.add(
        cPipeline(
            cPipeline::ePipe::ret,
            cPipeline::eLine::door,
            retloop));
}

cxy topLeft(const cRoom &room)
{
    cxy tl(INT_MAX, INT_MAX);
    for (auto &p : room.getWallPoints())
    {
        if (p.y <= tl.y)
        {
            if (p.x < tl.x)
                tl = p;
        }
    }
    return tl;
}

std::vector<cxy> loopConvex(cRoom &room)
{
    std::vector<cxy> loop;
    const auto &bounds = room.getBounds();
    double topy = bounds.myYmin + room.seperation();
    double boty = bounds.myYmax - room.seperation();
    double rightx = bounds.myXmax - room.seperation();

    // start at top left

    cxy p = topLeft(room);

    p.x += room.seperation();
    p.y = topy;
    loop.push_back(p);

    while (true)
    {
        p.y = boty;
        loop.push_back(p);

        p.x += room.seperation();
        if (p.x > rightx)
            break;
        loop.push_back(p);

        p.y = topy;
        loop.push_back(p);

        p.x += room.seperation();
        if (p.x > rightx)
            break;
        loop.push_back(p);
    }

    return loop;
}

void orderSubroomsByX(
    std::pair<cRoom, cRoom> &subrooms)
{
    double x1, x2;
    x1 = x2 = 0;
    int i = 0;
    while (fabs(x1 - x2) < 1)
    {
        x1 = subrooms.first.getWallPoints()[i].x;
        x2 = subrooms.second.getWallPoints()[i].x;
        i++;
    }
    if (x1 > x2)
    {
        auto temp = subrooms.first;
        subrooms.first = subrooms.second;
        subrooms.second = temp;
    }
}

std::vector<cxy> loopConcave(cRoom &room)
{
    // split concave room into 2 convex rooms

    auto subrooms = concaveSplit(room);

    // order subrooms into increasing x

    orderSubroomsByX(subrooms);

    // layout pipes in subrooms

    auto loop = loopConvex(subrooms.first);
    auto loop2 = loopConvex(subrooms.second);

    // combine loops

    loop.insert(
        loop.end(),
        loop2.begin(), loop2.end());

    return loop;

    // // add to room
}

void cPipeLayer::loop(cRoom &room)
{
    std::vector<cxy> loop;
    
    if (room.isConcave())
    {
        loop = loopConcave(room);
    }
    else
    {
        loop = loopConvex(room);
    }

    room.add(
        cPipeline(
            cPipeline::ePipe::hot,
            cPipeline::eLine::spiral,
            loop));

    connectLoopDoor(room, loop);
}

void cPipeLayer::furnaceRoom(cRoom &room)
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
        cxy d1 = room.getWallPoints()[doorIndex];
        cxy d2 = room.getWallPoints()[doorIndex + 1];
        cxy dc(
            d1.x + (d2.x - d1.x) / 2,
            d1.y + (d2.y - d1.y) / 2);
        cxy p2 = dc;
        eMargin m = cPolygon::margin(d1, d2);
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
            pipe.emplace_back(dc.x - sepret, dc.y + 1.5 * cRoom::seperation());
            pipe.emplace_back(dc.x - sepret, dc.y);
            break;
        case eMargin::left:
            pipe.emplace_back(dc.x + 1.5 * cRoom::seperation(), dc.y - sepret);
            pipe.emplace_back(dc.x, dc.y - sepret);
            break;
        case eMargin::bottom:
            pipe.emplace_back(dc.x - sepret, dc.y - 1.5 * cRoom::seperation());
            pipe.emplace_back(dc.x - sepret, dc.y);
            break;
        case eMargin::right:
            pipe.emplace_back(dc.x - 1.5 * cRoom::seperation(), dc.y + sepret);
            pipe.emplace_back(dc.x, dc.y + sepret);
            break;
        }
        room.add(cPipeline(
            cPipeline::ePipe::ret,
            cPipeline::eLine::door,
            pipe));
    }
}
