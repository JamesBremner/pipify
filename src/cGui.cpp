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
        if (ir == cRoom::furnaceRoomIndex())
        {
            drawFurnacePipes(S, ir);
        }
        else
        {
            drawPipes(S, cRoom::getRooms()[ir].pipes());
        }
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

void cGUI::drawFurnacePipes(
    wex::shapes &S,
    int ir)
{
    const int retSepPixels = scale * cRoom::seperation() / 2;
    cxy p1, p2, p3, p4;
    auto pipes = cRoom::getRooms()[ir].pipes();
    if (!pipes.size())
        return;

    for (auto &seg : pipes)
    {
        p2.x = -INT_MAX;
        for (auto p : seg)
        {
            // start from previous point
            p1 = p2;

            // new point
            p2.x = off + scale * p.x;
            p2.y = off + scale * p.y;

            if (p1.x == -INT_MAX)
            {
                // first time through
                continue;
            }

            S.color(0x0000FF);
            S.line({(int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y});

            switch (cRoom::side(p1, p2))
            {

            case eMargin::top:
                if (seg.myType == cPipeline::ePipe::ring)
                {
                    p3.x = p1.x + retSepPixels;
                    p3.y = p1.y + retSepPixels;
                    p4.x = p2.x - retSepPixels;
                    p4.y = p2.y + retSepPixels;
                }
                else
                {
                    p3 = p1;
                    p4 = p2;
                    p3.y -= retSepPixels;
                    p4.x += retSepPixels;
                    p4.y -= retSepPixels;
                }
                break;

            case eMargin::right:
                if (seg.myType == cPipeline::ePipe::ring)
                {
                    p3.x = p1.x - retSepPixels;
                    p3.y = p1.y + retSepPixels;
                    p4.x = p2.x - retSepPixels;
                    p4.y = p2.y - retSepPixels;
                }
                else
                {
                    p3 = p1;
                    p4 = p2;
                    p3.x -= retSepPixels;
                    p3.y -= retSepPixels;
                    p4.x -= retSepPixels;
                }
                break;

            case eMargin::bottom:
                if (seg.myType == cPipeline::ePipe::ring)
                {
                    p3.x = p1.x - retSepPixels;
                    p3.y = p1.y - retSepPixels;
                    p4.x = p2.x + retSepPixels;
                    p4.y = p2.y - retSepPixels;
                }
                else
                {
                    p3 = p1;
                    p4 = p2;
                    p3.x -= retSepPixels;
                    p3.y += retSepPixels;
                    p4.x -= retSepPixels;
                    p4.y += retSepPixels;
                }
                break;
            case eMargin::left:
                if (seg.myType == cPipeline::ePipe::ring)
                {
                    p3.x = p1.x + retSepPixels;
                    p3.y = p1.y - retSepPixels;
                    p4.x = p2.x + retSepPixels;
                    p4.y = p2.y + retSepPixels;
                }
                else
                {
                    p3 = p1;
                    p4 = p2;
                    p3.x += retSepPixels;
                    p3.y += retSepPixels;
                    p4.x += retSepPixels;
                }
                break;
            }
            S.color(0xFF0000);
            S.line({(int)p3.x, (int)p3.y, (int)p4.x, (int)p4.y});
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
        rx1 = x1 + sep;
        ry1 = y1 - sep;
        rx2 = x2 - sep;
        ry2 = y2 - sep;
        break;
    case eMargin::left:
        rx1 = x1 + sep;
        ry1 = y1 + sep;
        rx2 = x2 + sep;
        ry2 = y2 - sep;
        break;
    }
    // std::cout << "door return "
    //           << rx1 << " "
    //           << ry1 << " "
    //           << rx2 << " "
    //           << ry2 << " "
    //           << "\n";
}

void spiralReturnLineSegment(
    int &rx1, int &ry1, int &rx2, int &ry2,
    int x1, int y1, int x2, int y2,
    eCorner corner,
    int sep)
{
    switch (corner)
    {
    case eCorner::tl_vex:
        rx1 = x1 + sep;
        ry1 = y1 - sep;
        rx2 = x2 + sep;
        ry2 = y2 + sep;
        break;
    case eCorner::tr_vex:
        rx1 = x1 + sep;
        ry1 = y1 + sep;
        rx2 = x2 - sep;
        ry2 = y2 + sep;
        break;
    case eCorner::br_vex:
        rx1 = x1 - sep;
        ry1 = y1 + sep;
        rx2 = x2 - sep;
        ry2 = y2 - sep;
        break;
    case eCorner::bl_vex:
        rx1 = x1 - sep;
        ry1 = y1 - sep;
        rx2 = x2 + sep;
        ry2 = y2 - sep;
        break;
    case eCorner::error:
        // if (p.y == pipesegment[ip - 1].y)
        // {
        //     S.line({x1, y1 + outInSep, x2 + outInSep, y2 + outInSep});
        // }
        break;
    }
}

void cGUI::drawPipes(
    wex::shapes &S,
    const std::vector<cPipeline> &pipes)
{
    int x1, y1, x2, y2, x3, y3, x4, y4;

    for (auto &pipesegment : pipes)
    {
        x2 = INT_MAX;
        cxy lastReturn;
        eCorner corner;
        bool first = true;

        // loop over pipe bends
        for (int ip = 0; ip < pipesegment.size(); ip++)
        {
            cxy p = pipesegment.get(ip);

            if (ip == 0)
            {
                x2 = off + scale * p.x;
                y2 = off + scale * p.y;
                continue;
            }
            else
            {
                x1 = x2;
                y1 = y2;
                x2 = off + (int)(scale * p.x);
                y2 = off + (int)(scale * p.y);
                S.color(0x0000FF);
                S.line({x1, y1, x2, y2});
                S.color(0);

                // draw return pipe

                int rx1, ry1, rx2, ry2;
                if (pipesegment.myType == cPipeline::ePipe::door)
                {
                    doorReturnLineSegment(
                        rx1, ry1, rx2, ry2,
                        x1, y1, x2, y2,
                        retSepPixels);
                }
                else if (pipesegment.myType == cPipeline::ePipe::subroom)
                {
                    std::cout << "subroom spiral return\n";

                    if (ip == pipesegment.size() - 1)
                        corner = cRoom::next(corner);
                    else
                        corner = cRoom::corner(
                            pipesegment.get(ip - 1),
                            p,
                            pipesegment.get(ip + 1));

                    spiralReturnLineSegment(
                        rx1, ry1, rx2, ry2,
                        x1, y1, x2, y2,
                        corner,
                        retSepPixels);

                    if (first)
                    {
                        first = false;
                        rx1 -= 2 * retSepPixels;
                    }
                    lastReturn = cxy(rx2, ry2);
                }
                else
                {
                    if (ip == pipesegment.size() - 1)
                        corner = cRoom::next(corner);
                    else
                        corner = cRoom::corner(
                            pipesegment.get(ip - 1),
                            p,
                            pipesegment.get(ip + 1));
                    spiralReturnLineSegment(
                        rx1, ry1, rx2, ry2,
                        x1, y1, x2, y2,
                        corner,
                        retSepPixels);
                    lastReturn = cxy(rx2, ry2);
                }

                S.color(0xFF0000);
                S.line({rx1, ry1, rx2, ry2});
            }
        }
        if (pipesegment.myType == cPipeline::ePipe::spiral)
        {
            // connect spiral centers
            // int lastbendindex = pipesegment.size() - 2;
            x1 = off + scale * pipesegment.last().x;
            y1 = off + scale * pipesegment.last().y;
            x2 = lastReturn.x;
            y2 = lastReturn.y;
            S.color(0);
            S.line({x1, y1, x2, y2});
        }
    }
}
