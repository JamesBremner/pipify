#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include <cxy.h>
#include "cStarterGUI.h"

// A room composed of walls and doors
class cRoom
{
public:
    static std::vector<cRoom> theHouse; // the house composed of rooms

    std::vector<cxy> myWallPoints; // room walls specified by a clockwise open polygon of 2D points
    std::vector<int> myDoorPoints; // indices in myWallPoints of first point of pairs specifying doors

    cRoom(
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints)
    {
        myWallPoints = wallPoints;
        myDoorPoints = doorPoints;
    }

    std::vector<std::vector<cxy>> wallSegments()
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

    /// @brief get the wall segments of each room in the house
    /// @return a vector of rooms containing a vector of wall segments containing a vector of wall points

    static std::vector<std::vector<std::vector<cxy>>> houseWallSegments()
    {
        std::vector<std::vector<std::vector<cxy>>> ret;
        for (auto &r : theHouse)
        {
            ret.push_back(r.wallSegments());
        }
        return ret;
    }

    static void add(
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints)
    {
        theHouse.emplace_back(wallPoints, doorPoints);
    }
    static void clear()
    {
        theHouse.clear();
    }
};

std::vector<cRoom> cRoom::theHouse;

// generate a one room house
void gen1()
{
    std::vector<cxy> wallPoints = {{0, 0}, {10, 0}, {30, 0}, {100, 0}, {100, 100}, {0, 100}};
    std::vector<int> doorPoints = {1};

    cRoom::clear();
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

        gen1();

        fm.events().draw(
            [](PAINTSTRUCT &ps)
            {
                const int scale = 3;
                const int off = 20;
                wex::shapes S(ps);
                S.penThick(4);

                // auto segs = cRoom::houseWallSegments();

                // loop over rooms
                for (auto &r : cRoom::houseWallSegments())

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
            });

        show();
        run();
    }

private:
};

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
    return true;
}

main()
{
    if (!unitTest())
    {
        std::cout << "unit test failed\n";
        exit(1);
    }

    cGUI theGUI;
    return 0;
}
