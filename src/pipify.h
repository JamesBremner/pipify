

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
        const std::vector<int> doorPoints);

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

    /// @brief check for crossed pipe
    /// @param p1 start of pipe segment
    /// @param p2 end of pipe segment
    /// @return true if pipe segment crosses any existing pipe segments

    bool isPipeCrossing( const cxy& p1, const cxy& p2 ) const;

    /// @brief Check if room is concave
    /// @param[out] index if concave, the wallpoint index of the concave corner
    /// @return concave corner type, or eCorner::error if convex

    eCorner isConcave( int& index ) const;

    /// @brief find wall segment on a margin
    /// @param m margin
    /// @return segment start and end

    std::pair<cxy,cxy> find( eMargin m ) const;

    /// @brief layout pipes in a room guaranteed to be convex

    void pipeConvex();

    /// @brief layout pipes in a concave room
    /// @param concaveIndex index of wall point at concave corner

    void pipeConcave( int concaveIndex );

    /// @brief layout pipes in room

    void pipe();

    static void pipeHouse()
    {
        for (auto &r : theHouse)
            r.pipe();
    }

    /// @brief set the separation between pipes
    /// @param seperation 

    static void set(int seperation)
    {
        theSeperation = seperation;
    }

    /// @brief get pipe locations for the house
    /// @return locations where pipes turn 90 degrees for each room

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

    /// @brief construct room and add to house
    /// @param wallPoints 
    /// @param doorPoints 

    static void add(
        const std::vector<cxy> wallPoints,
        const std::vector<int> doorPoints);

    /// @brief clear rooms from house
    
    static void clear();

    static void readfile( const std::string& fname );

};

// free functions

bool unitTest();

