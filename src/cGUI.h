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
};