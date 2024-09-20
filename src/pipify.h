#include <vector>
#include <string>
#include <algorithm>
#include "cxy.h"

enum class eCorner
{
    error,
    tl_vex, // top left convex
    tr_vex,
    br_vex,
    bl_vex,
    tr_cav, // top right concave
    tl_cav,
    bl_cav,
    br_cav,
    top, // no corner, along top
    right,
    bottom,
    left,
};
    enum class eMargin
    {
        top,
        right,
        bottom,
        left
    };
class cPipeline
{
public:
    enum class ePipe
    {
        none,
        hot,
        ret,
    };
    ePipe myType;
    std::vector<cxy> myLine;

    cPipeline(
        ePipe type,
        const std::vector<cxy> &bends)
        : myType(type),
          myLine(bends)
    {
    }
    void set(ePipe type)
    {
        myType = type;
    }
    cxy last() const
    {
        return myLine.back();
    }
    int size() const
    {
        return myLine.size();
    }
    cxy get(int index) const
    {
        if (0 > index || index > size() - 1)
            throw std::runtime_error("bad pipeline index");
        return myLine[index];
    }

    std::vector<cxy>::iterator begin()
    {
        return myLine.begin();
    }
    std::vector<cxy>::iterator end()
    {
        return myLine.end();
    }
};

// A room composed of walls and doors
class cRoom
{

    static std::vector<cRoom> theHouse;     // the house composed of rooms
    static int theSeperation;               // separation between hot pipes, user units
    static int thefurnaceRoomIndex;

    std::string myName;
    std::vector<cxy> myWallPoints;                   // room walls specified by a clockwise open polygon of 2D points
    std::vector<int> myDoorPoints;                   // indices in myWallPoints of first point of pairs specifying doors
    cxy myDoorCenter;
    double myXmin, myXmax, myYmin, myYmax, myMaxDim; // bounding rectangle

    std::vector<cPipeline> myPipePoints;

public:
    cRoom(
        const std::string &name,
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints);

    cRoom() {}

    void clearPipes()
    {
        myPipePoints.clear();
    }

    std::string name() const
    {
        return myName;
    }
    
    /// @brief get the wall segments, ready to draw the room wall
    /// @return
    std::vector<std::vector<cxy>> wallSegments();

    std::vector<cPipeline> pipes() 
    {
        return myPipePoints;
    }
    int doorCount() const
    {
        return myDoorPoints.size();
    }
    const std::vector<cxy> &getWallPoints() const
    {
        return myWallPoints;
    }
    const cxy& getWallDoorPoint( int index ) const
    {
        if( index < 0 || index > myWallPoints.size()-1)
            throw std::runtime_error("getWallDoorPoint");
        return myWallPoints[ index ];
    }
    int getWallDoorIndex( const cxy& p ) const
    {
        auto it = std::find(
            myWallPoints.begin(),myWallPoints.end(), p        );
        if( it == myWallPoints.end())
            return -1;
        return it - myWallPoints.begin();
    }
    std::vector<int> getDoorPoints() const
    {
        return myDoorPoints;
    }



    /// @brief which side of the room are two points on
    /// @param p1
    /// @param p2
    /// @return margin
    ///
    /// Assumes room polygon defined clockwise

    static eMargin side(const cxy &p1, const cxy &p2);

    /// @brief identify corner type
    /// @param p1
    /// @param p2
    /// @param p3
    /// @return eCorner

    static eCorner corner(
        const cxy &p1,
        const cxy &p2,
        const cxy &p3);

    /// @brief check for crossed pipe
    /// @param p1 start of pipe segment
    /// @param p2 end of pipe segment
    /// @return true if pipe segment crosses any existing pipe segments

    bool isPipeCrossing(const cxy &p1, const cxy &p2) const;

    /// @brief Check if room is concave
    /// @param[out] index if concave, the wallpoint index of the concave corner
    /// @return concave corner type, or eCorner::error if convex

    eCorner isConcave(int &index) const;

    /// @brief find wall segment on a margin
    /// @param m margin
    /// @return segment start and end

    std::pair<cxy, cxy> find(eMargin m) const;

    /// @brief layout pipes in a room guaranteed to be convex
    /// @param startPoint
    /// for a doored room, the startPoint locates the hot point in the doorway
    /// for an undoored room ( concave subroom ) the nearest pipe point in doored subroom

    void pipeConvex(int x = 0, int y = 0);

    /// @brief layout pipes in a concave room
    /// @param concaveIndex index of wall point at concave corner

    void concavePipe(int concaveIndex);

    /// @brief layout pipes in a doorway
    /// @return location of hot pipe in doorway

    cxy pipeDoor();

    /// @brief layout pipes in room

    void pipe();

    /// @brief layout pipes in furnace room

    void pipefurnaceRoom();

    /// @brief layout pipes in every room of the house

    static void pipeHouse();

    /// @brief set the separation between pipes
    /// @param seperation

    static void set(int seperation)
    {
        theSeperation = seperation;
    }
    static int seperation()
    {
        return theSeperation;
    }

    static int roomCount()
    {
        return theHouse.size();
    }

    static std::vector<cRoom> &getRooms()
    {
        return theHouse;
    }

    static int furnaceRoomIndex()
    {
        return thefurnaceRoomIndex;
    };

    /// @brief check that a valid furnace room has been specified
    /// @param name furnace room name from input file
    ///
    /// throws exception on error

    static void furnaceRoom(const std::string &name);

    /// @brief get pipe locations for the house
    /// @return locations where pipes turn 90 degrees for each room
    /// room vector, containing vector of pipelines

    static std::vector<std::vector<cPipeline>> housePipes()
    {
        std::vector<std::vector<cPipeline>> ret;
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

    /// @brief construct room and add to house
    /// @param name
    /// @param wallPoints
    /// @param doorPoints

    static void add(
        const std::string &name,
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints);

    /// @brief clear rooms from house

    static void clear();

    /// @brief clear pipes from house
    
    static void clearHousePipes();

    /// @brief get next corner type
    /// @param corner current corner type
    /// @return next corner type
    ///
    /// Assumes convex room specified clockwise

    static eCorner next(const eCorner &corner)
    {
        int ret = (int)corner;
        ret++;
        if (ret > (int)eCorner::bl_vex)
            ret = (int)eCorner::tl_vex;
        return (eCorner)ret;
    }

    static void readfile(const std::string &fname);

private:
    void boundingRectangle();

    // std::vector<cxy> pipeSpiral(
    //     int startIndex,
    //     const cxy &startPoint);

    // bool isSpiralComplete(
    //     std::vector<cxy> &spiral,
    //     int wallSeperation,
    //     const cxy &nextbend) const;
};

/// @brief Room corners ( no doors ) as a closed polygon
class cCorners
{
    std::vector<cxy> myCorners;
    std::vector<int> myIndices; // Wallpoint index of each corner

public:
    cCorners(const cRoom &room);

    const std::vector<cxy> &getCorners() const
    {
        return myCorners;
    }

    /// corner index from wallpoint index
    int index(int wp) const;
};

// free functions

bool unitTest();

/// @brief Split concave room into two convex rooms
/// @param[in] ConcaveRoom to be split
/// @param[out] joinPoint where the 2nd room pipe joins the first room pipe
/// @return pair of convex rooms

std::pair<cRoom,cRoom> concaveSplit(
    const cRoom& ConcaveRoom,
    cxy& joinPoint );


