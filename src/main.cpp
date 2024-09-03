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

    static std::vector<cRoom> theHouse; // the house composed of rooms
    static int theSeperation;

    std::vector<cxy> myWallPoints; // room walls specified by a clockwise open polygon of 2D points
    std::vector<int> myDoorPoints; // indices in myWallPoints of first point of pairs specifying doors
    std::vector<cxy> myPipePoints;

public:
    cRoom(
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints)
    {
        myWallPoints = wallPoints;
        myDoorPoints = doorPoints;
        pipe();
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

    std::vector<cxy> pipes()
    {
        return myPipePoints;
    }

    enum class eMargin
    {
        top,
        right,
        bottom,
        left
    };

    /// @brief which side of the room are two points on
    /// @param p1
    /// @param p2
    /// @return margin
    ///
    /// Assumnes room polygon defined clockwise

    eMargin side(const cxy &p1, const cxy &p2)
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

    // layout pipes in room
    void pipe()
    {

        // start at the first door
        cxy d1 = myWallPoints[myDoorPoints[0]];
        cxy d2 = myWallPoints[myDoorPoints[0] + 1];
        cxy p1, p2, p3;
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

        for (int L = 1; true; L++)
        {
            int wallSeperation = L * theSeperation;

            for (int i = myDoorPoints[0] + 2;
                 i < myWallPoints.size();
                 i++)
            {
                p1 = myWallPoints[i - 1];
                p2 = myWallPoints[i];
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
                    p2.x -= wallSeperation;
                    p2.y += wallSeperation;
                    break;
                }
                myPipePoints.push_back(p2);
            }

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

    static void set(int seperation)
    {
        theSeperation = seperation;
    }

    static std::vector<std::vector<cxy>> housePipes()
    {
        std::vector<std::vector<cxy>> ret;
        for (auto &r : theHouse)
        {
            ret.push_back(r.pipes());
        }
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
int cRoom::theSeperation;

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
    // if (!unitTest())
    // {
    //     std::cout << "unit test failed\n";
    //     exit(1);
    // }

    cGUI theGUI;
    return 0;
}
