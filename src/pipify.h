

// A room composed of walls and doors
class cRoom
{

    static std::vector<cRoom> theHouse; // the house composed of rooms
    static int theSeperation;
    static int thefurnaceRoomIndex;

    std::string myName;
    std::vector<cxy> myWallPoints; // room walls specified by a clockwise open polygon of 2D points
    std::vector<int> myDoorPoints; // indices in myWallPoints of first point of pairs specifying doors
    std::vector<std::vector<cxy>> myPipePoints;

public:
    cRoom(
        const std::string &name,
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints);

    /// @brief get the wall segments, ready to draw the room wall
    /// @return
    std::vector<std::vector<cxy>> wallSegments();

    std::vector<std::vector<cxy>> pipes() const
    {
        return myPipePoints;
    }
    int doorCount() const
    {
        return myDoorPoints.size();
    }

    enum class eMargin
    {
        top,
        right,
        bottom,
        left
    };
    enum class eCorner
    {
        error,
        tr_vex, // top right convex
        tl_vex,
        bl_vex,
        br_vex,
        tr_cav, // top right concave
        tl_cav,
        bl_cav,
        br_cav,
        top, // no corner, along top
        right,
        bottom,
        left,
    };

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
    /// @return vector of pipe bend points
    /// for a convex room, the startPoint can be ignored
    /// for any subroom, except the first,
    ///         startPoint should be the nearest pipe point in first subroom

    std::vector<cxy> pipeConvex(int x = 0, int y = 0);

    /// @brief layout pipes in a concave room
    /// @param concaveIndex index of wall point at concave corner

    void pipeConcave(int concaveIndex);

    /// @brief layout pipes in a doorway
    /// @param spiral pipes added to

    void pipeDoor(std::vector<cxy> &spiral);

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

    static void furnaceRoom(const std::string &name);

    /// @brief get pipe locations for the house
    /// @return locations where pipes turn 90 degrees for each room
    /// room vector, containing segment vector, containing bend points

    static std::vector<std::vector<std::vector<cxy>>> housePipes()
    {
        std::vector<std::vector<std::vector<cxy>>> ret;
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

    static void readfile(const std::string &fname);

private:
    std::vector<cxy> pipeSpiral(
        const cxy &startPoint,
        int startIndex);

    bool isSpiralComplete(
        std::vector<cxy> &spiral,
        const cxy &nextbend) const;
};

// free functions

bool unitTest();
