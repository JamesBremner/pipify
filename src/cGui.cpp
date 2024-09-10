#include <wex.h>
#include "pipify.h"
#include "cGUI.h"

void cGUI::drawHousePipes(
    wex::shapes &S)
{
    S.penThick(1);
    for (int ir = 0; ir < cRoom::roomCount(); ir++)
    {
        if (ir == cRoom::furnaceRoomIndex())
        {
            drawFurnacePipes(S, ir);
            continue;
        }

        // loop over pipe segments
        for (auto &pipesegment : cRoom::getRooms()[ir].pipes())
        {
            drawPipeSegment(S, pipesegment);
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

    cxy p1, p2, p3, p4;
    auto pipes = cRoom::getRooms()[ir].pipes();
    auto seg = pipes[0];
    {
        for (int ip = 0; ip < seg.size(); ip++)
        {
            p1 = p2;
            p2.x = off + scale * seg[ip].x;
            p2.y = off + scale * seg[ip].y;
            if (ip == 0)
                continue;
            S.color(0x0000FF);
            S.line({p1.x, p1.y, p2.x, p2.y});
            switch (cRoom::side(p1, p2))
            {
            case cRoom::eMargin::top:
                p3.x = p1.x + 3;
                p3.y = p1.y + 3;
                p4.x = p2.x - 3;
                p4.y = p2.y + 3;
                break;
            case cRoom::eMargin::right:
                p3.x = p1.x - 3;
                p3.y = p1.y + 3;
                p4.x = p2.x - 3;
                p4.y = p2.y - 3;
                break;
            case cRoom::eMargin::bottom:
                p3.x = p1.x - 3;
                p3.y = p1.y - 3;
                p4.x = p2.x + 3;
                p4.y = p2.y - 3;
                break;
            case cRoom::eMargin::left:
                p3.x = p1.x + 3;
                p3.y = p1.y - 3;
                p4.x = p2.x + 3;
                p4.y = p2.y + 3;
                break;
            }
            S.color(0xFF0000);
            S.line({p3.x, p3.y, p4.x, p4.y});
        }
        p1 = p2;
        p2.x = off + scale * pipes[0][0].x;
        p2.y = off + scale * pipes[0][0].y;
        S.color(0x0000FF);
        S.line({p1.x, p1.y, p2.x, p2.y});
        p3.x = p1.x + 3;
        p3.y = p1.y - 3;
        p4.x = p2.x + 3;
        p4.y = p2.y + 3;
        S.color(0xFF0000);
        S.line({p3.x, p3.y, p4.x, p4.y});
    }

    for (int i = 1; i < pipes.size(); i++)
    {
        p1.x = off + scale * pipes[i][0].x;
        p1.y = off + scale * pipes[i][0].y;
        p2.x = off + scale * pipes[i][1].x;
        p2.y = off + scale * pipes[i][1].y;
        S.color(0x0000FF);
        S.line({p1.x, p1.y, p2.x, p2.y});

        // return pipe
        switch (cRoom::side(p1, p2))
        {
        case cRoom::eMargin::left:
            p1.x += 5;
            p1.y += 4;
            p2.x += 5;
            break;
        case cRoom::eMargin::bottom:
            p1.x += 5;
            p1.y -= 4;
            p2.x += 5;
            break;
        case cRoom::eMargin::right:
            p1.x += 5;
            p1.y -= 4;
            p2.x += 5;
            break;
        case cRoom::eMargin::top:
            p1.y -= 5;
            p2.x += 5;
            p2.y = p1.y;
            break;
        }
        S.color(0xFF0000);
        S.line({p1.x, p1.y, p2.x, p2.y});
    }
}

void cGUI::drawPipeSegment(
    wex::shapes &S,
    const std::vector<cxy> &pipesegment)
{
    const int outInSep = cRoom::seperation() + 2;
    int x1, y1, x2, y2, x3, y3, x4, y4;
    x2 = INT_MAX;
    cxy lastReturn;

    // draw in doorway pipe
    x1 = off + scale * pipesegment[0].x;
    y1 = off + scale * pipesegment[0].y;
    x2 = off + scale * pipesegment[1].x;
    y2 = off + scale * pipesegment[1].y;
    x3 = off + scale * pipesegment[2].x;
    y3 = off + scale * pipesegment[2].y;
    S.color(0x0000FF);
    S.line({x1, y1, x2, y2});
    S.line({x2, y2, x3, y3});
    S.color(0);

    // draw out doorway pipe
    int sep = cRoom::seperation();
    switch (
        cRoom::side(pipesegment[1], pipesegment[2]))
    {
    case cRoom::eMargin::top:
        x1 -= sep;
        x2 -= sep;
        y2 += sep;
        x3 += sep + 1;
        y3 += sep + 1;
        break;
    case cRoom::eMargin::right:
        y1 -= sep;
        x2 -= sep;
        y2 -= sep;
        x3 -= sep;
        y3 += sep + 1;
        break;
    case cRoom::eMargin::bottom:
        x1 += sep;
        x2 += sep;
        y2 -= 2 * sep;
        y3 -= 2 * sep;
        break;
    }
    S.color(0xFF0000);
    S.line({x1, y1, x2, y2});
    S.line({x2, y2, x3, y3});
    S.color(0);

    // loop over pipe bends
    for (int ip = 2; ip < pipesegment.size() - 1; ip++)
    {
        cxy p = pipesegment[ip];

        if (ip == 2)
        {
            x2 = off + scale * p.x;
            y2 = off + scale * p.y;
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
            switch (cRoom::corner(pipesegment[ip - 1], p, pipesegment[ip + 1]))
            {
            case cRoom::eCorner::tl_vex:
                rx1 = x1 + outInSep;
                ry1 = y1 - outInSep;
                rx2 = x2 + outInSep;
                ry2 = y2 + outInSep;
                break;
            case cRoom::eCorner::tr_vex:
                rx1 = x1 + outInSep;
                ry1 = y1 + outInSep;
                rx2 = x2 - outInSep;
                ry2 = y2 + outInSep;
                break;
            case cRoom::eCorner::br_vex:
                rx1 = x1 - outInSep;
                ry1 = y1 + outInSep;
                rx2 = x2 - outInSep;
                ry2 = y2 - outInSep;
                break;
            case cRoom::eCorner::bl_vex:
                rx1 = x1 - outInSep;
                ry1 = y1 - outInSep;
                rx2 = x2 + outInSep;
                ry2 = y2 - outInSep;
                break;
            case cRoom::eCorner::error:
                if (p.y == pipesegment[ip - 1].y)
                {
                    S.line({x1, y1 + outInSep, x2 + outInSep, y2 + outInSep});
                }
                break;
            }
            lastReturn = cxy(rx2, ry2);
            S.color(0xFF0000);
            S.line({rx1, ry1, rx2, ry2});
            S.color(0);
        }
    }
    // connect spiral centers
    int lastbendindex = pipesegment.size() - 2;
    x1 = off + scale * pipesegment[lastbendindex].x;
    y1 = off + scale * pipesegment[lastbendindex].y;
    x2 = lastReturn.x;
    y2 = lastReturn.y;
    S.line({x1, y1, x2, y2});
}
