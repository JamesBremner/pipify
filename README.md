# Pipify

## Specication of the rooms in a house

Each room is specified by a clockwise open polygon of 2D points ( Every corner is specified by one point, every door is specified by a pair of points. ) and a vector of indices in the polygon of the first point of a pairs specifying a door

For example, a square room with one door is specified as:

```C++
    std::vector<cxy> wallPoints = {{0, 0}, {10, 0}, {30, 0}, {100, 0}, {100, 100}, {0, 100}};
    std::vector<int> doorPoints = {1};
```

which when rendered gives

![image](https://github.com/user-attachments/assets/5d1d233f-919c-4ad8-b202-1d28d04ace88)

