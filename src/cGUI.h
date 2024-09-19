class cStarterGUI
{
public:
    /** CTOR
     * @param[in] title will appear in application window title
     * @param[in] vlocation set location and size of appplication window
     *
     * Usage:
     *
     * <pre>
class appGUI : public cStarterGUI
{
public:
    appGUI()
        : cStarterGUI(
              "The Appliccation",
              {50, 50, 1000, 500})
    {

        // initialize the solution
        ...

        show();
        run();
    }
    </pre>
    */
    cStarterGUI(
        const std::string &title,
        const std::vector<int> &vlocation)
        : fm(wex::maker::make())
    {
        fm.move(vlocation);
        fm.text(title);

        fm.events().draw(
            [&](PAINTSTRUCT &ps)
            {
                wex::shapes S(ps);
                draw(S);
            });
    }
    /** Draw nothing
     *
     * An application should over-ride this method
     * to perform any drawing reuired
     */
    virtual void draw(wex::shapes &S)
    {
    }
    void show()
    {
        fm.show();
    }
    void run()
    {
        fm.run();
    }

protected:
    wex::gui &fm;
};

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "Pipify",
              {50, 50, 1000, 500})
    {
        // set default pipe separation
        cRoom::set( 6 );

        menus();

        fm.events().draw(
            [this](PAINTSTRUCT &ps)
            {
                wex::shapes S(ps);
                S.color(0);
                S.penThick(4);

                retSepPixels = ( scale * cRoom::seperation() ) / 2;

                // loop over rooms
                for (auto &r : cRoom::houseWallSegments())
                {
                    drawWalls(S, r);
                }

                drawHousePipes(S);
            });

        show();
        run();
    }

private:
    const int scale = 3;
    const int off = 20;
    int retSepPixels;       // seperation between hot and return pipes in pixels

    void menus();

    void drawPipes(
        wex::shapes &S,
        const std::vector<cPipeline> &pipes);

    void drawHousePipes(
        wex::shapes &S);

    void drawWalls(
        wex::shapes &S,
        const std::vector<std::vector<cxy>> &walls);
};