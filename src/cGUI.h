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

        menus();

        fm.events().draw(
            [this](PAINTSTRUCT &ps)
            {
                wex::shapes S(ps);
                S.color(0);
                S.penThick(4);

                // loop over rooms
                for (auto &r : cRoom::houseWallSegments())
                {
                    drawWalls( S, r );
                }

                S.penThick(1);
                for (auto &r : cRoom::housePipes())
                {
                    // loop over pipe segments
                    for (auto &pipesegment : r)
                    {
                        drawPipeSegment(S, pipesegment);
                    }
                }
            });

        show();
        run();
    }

private:
    const int scale = 3;
    const int off = 20;

    void menus()
    {
        wex::menubar mb(fm);

        wex::menu mf(fm);
        mf.append("Open", [&](const std::string &title)
                  {
        // prompt for file to open
        wex::filebox fb( fm );
        auto paths = fb.open();
        if( paths.empty() )
            return;
        try
        {
            // read the file
            cRoom::readfile( paths );

            fm.text( paths);

            cRoom::pipeHouse();

            // refresh display with contents of opened file
            fm.update();
        }
        catch( std::runtime_error& e )
        {
            wex::msgbox mb(
                           std::string("Error reading file\n")+e.what());
            exit(1);
        } });

        mf.appendSeparator();
        mf.append("Run Unit Tests", [&](const std::string &title)
                  {
            if (!unitTest())
            {
                 wex::msgbox mb("unit test failed");
                exit(1);
            } });

        mb.append("File", mf);
    }

    void drawPipeSegment(
        wex::shapes &S,
        const std::vector<cxy> &pipesegment);

    void drawWalls(
        wex::shapes &S,
        const std::vector<std::vector<cxy>>& walls    );

};