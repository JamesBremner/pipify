

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

    std::vector<std::vector<cxy>> wallSegments();

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
        br_cav
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

    // layout pipes in room
    void pipe();

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
        const std::vector<int> doorPoints);

    static void clear();

};

bool unitTest();
void gen1();
void genL();

