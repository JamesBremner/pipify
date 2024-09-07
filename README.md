# Pipify

Design the underfloor heating of a house by laying out the pipes in a spiral in each room.

## Specication of the rooms in a house

Each room is specified by a clockwise open polygon of 2D points ( Every corner is specified by one point, every door is specified by a pair of points. ) and a vector of indices in the polygon of the first point of a pair specifying a door

For example, a square room with one door is specified as:

```C++
    std::vector<cxy> wallPoints = {{0, 0}, {10, 0}, {30, 0}, {100, 0}, {100, 100}, {0, 100}};
    std::vector<int> doorPoints = {1};
```

which when rendered gives

![image](https://github.com/user-attachments/assets/5d1d233f-919c-4ad8-b202-1d28d04ace88)

## Pipe layout in a convex room

Shows return pipe in blue.

![image](https://github.com/user-attachments/assets/3fa3fcf3-faa0-4acc-b7ea-fa872df0b8ba)


## Pipe layout in a L-shaped room

![image](https://github.com/user-attachments/assets/0406efd6-f7a7-4ecc-92ba-5bfb3addaf33)




