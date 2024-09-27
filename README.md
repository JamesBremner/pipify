# Pipify

Design the underfloor heating of a house by laying out the pipes in a spiral in each room.

## Specification of the rooms in a house

Each room is specified by a clockwise open polygon of 2D points ( Every corner is specified by one point, every door is specified by a pair of points. ) and a vector of indices in the polygon of the first point of a pair specifying a door

For example, a square room with one door is specified as:

```C++
    std::vector<cxy> wallPoints = {{0, 0}, {10, 0}, {30, 0}, {100, 0}, {100, 100}, {0, 100}};
    std::vector<int> doorPoints = {1};
```

which when rendered gives

![image](https://github.com/user-attachments/assets/5d1d233f-919c-4ad8-b202-1d28d04ace88)

## Pipe layout in  convex rooms

Shows return pipe in blue.

![image](https://github.com/user-attachments/assets/5ef1d03e-c891-4a1e-b6ec-4217baa764ef)


## Pipe layout in a L-shaped rooms

![image](https://github.com/user-attachments/assets/ec500c35-3e6a-4b9d-9588-2f0e8abe298b)


## GUI

![image](https://github.com/user-attachments/assets/f4267a52-29ab-46cf-9d72-7721a6545550)

 ### File  
 
  - Open processed floor plan input file

### Run
 
  - Run the pipes through the room
  - Run the unit tests

### Edit
  
   - Change the pipe separation
   - Change the layout algorithm



