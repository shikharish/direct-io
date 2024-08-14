read-write tool exploring O_DIRECT on Linux

compile using gcc:
```
gcc io.c -o io
```

to use:
```
./io <<< "input.txt output.txt"
```
this will simply copy input.txt to output.txt
