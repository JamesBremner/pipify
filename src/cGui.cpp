#include <wex.h>
#include <inputbox.h>
#include "pipify.h"
#include "cGUI.h"

void cGUI::menus()
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

            // refresh display with contents of opened file
            fm.update();
        }
        catch( std::runtime_error& e )
        {
            wex::msgbox mb(
                           std::string("Error reading file\n")+e.what());
            exit(1);
        } });

    mb.append("File", mf);

    wex::menu mr(fm);
    mr.append("Pipes",
              [&](const std::string &title)
              {
                  try
                  {
                      cRoom::pipeHouse();
                  }
                  catch (std::runtime_error &e)
                  {
                      wex::msgbox(
                          std::string("Error running pipes ") + e.what());

                      exit(1);
                  }
                  fm.update();
              });
    mr.append("Unit Tests",
              [&](const std::string &title)
              {
                  if (!unitTest())
                  {
                      wex::msgbox mb("unit test failed");
                      exit(1);
                  }
                  wex::msgbox mb("unit test passed");
              });

    mb.append("Run", mr);

    wex::menu me(fm);
    me.append("Pipe Separation",
              [&](const std::string &title)
              {
                  // prompt user
                  wex::inputbox ib(fm);
                  ib.text("Pipe Separation");
                  ib.labelWidth(70);
                  ib.add("Separation", std::to_string(cRoom::seperation()));
                  ib.showModal();

                  // set edited value
                  cRoom::set(atoi(ib.value("Separation").c_str()));

                  cRoom::clearHousePipes();

                  fm.update();
              });

    mb.append("Edit", me);
}

void cGUI::drawHousePipes(
    wex::shapes &S)
{
    S.penThick(1);
    for (int ir = 0; ir < cRoom::roomCount(); ir++)
    {
        drawPipes(S, cRoom::getRooms()[ir].pipes());
    }
}

void cGUI::drawWalls(
    wex::shapes &S,
    const std::vector<std::vector<cxy>> &walls)
{
    // loop over wall segments
    for (auto &s : walls)
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
    }
}

void doorReturnLineSegment(
    int &rx1, int &ry1, int &rx2, int &ry2,
    int x1, int y1, int x2, int y2,
    int sep)
{
    switch (cRoom::side(cxy(x1, y1), cxy(x2, y2)))
    {
    case eMargin::top:
        rx1 = x1 - sep;
        ry1 = y1 + sep;
        rx2 = x2 + sep;
        ry2 = y1 + sep;
        break;
    case eMargin::right:
        rx1 = x1 - sep;
        ry1 = y1 - sep;
        rx2 = x2 - sep;
        ry2 = y2 + sep;
        break;
    case eMargin::bottom:
        rx1 = x1 - sep;
        ry1 = y1 - sep;
        rx2 = x2 - sep;
        ry2 = y2 - sep;
        break;
    case eMargin::left:
        rx1 = x1 - sep;
        ry1 = y1 + sep;
        rx2 = x2 - sep;
        ry2 = y2 - sep;
        break;
    }
    // std::cout << "door return\n"
    //           << x1 <<" "<< y1 <<" "<< x2 <<" "<< y2 <<"\n"
    //           << rx1 << " "
    //           << ry1 << " "
    //           << rx2 << " "
    //           << ry2 << " "
    //           << "\n";
}

void cGUI::drawPipes(
    wex::shapes &S,
    const std::vector<cPipeline> &pipes)
{
    cxy p1, p2;

    for (const cPipeline &seg : pipes)
    {
        p2.x = -INT_MAX;

        for (const cxy &p : seg.myLine)
        {
            if (p2.x == -INT_MAX)
            {
                // first point
                p2.x = off + scale * p.x;
                p2.y = off + scale * p.y;
                continue;
            }

            // prev point
            p1 = p2;

            // next point
            p2.x = off + scale * p.x;
            p2.y = off + scale * p.y;

            // set color
            if (seg.myType == cPipeline::ePipe::ret)
                S.color(0xFF0000);
            else
                S.color(0x0000FF);

            // line from prev to next point
            S.line({(int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y});
        }
    }
}
