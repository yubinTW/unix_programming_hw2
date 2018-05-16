# unix_programming_hw2

# compile
```
make
```

# run
```
./hw2
```

# run with optional parameter

- ```-a```
list processes from all the users.
```
./hw2 -a
```

- ```-x```
list processes without an associated terminal
```
./hw2 -x
```

> you can combile 2 option like ```./hw2 -a -x```

- ```-p```
sort the listed processes by pid (default)
```
./hw2 -p
```

- ```-q```
sort the listed processes by ppid
```
./hw2 -q
```

- ```-r```
sort the listed processes by pgid
```
./hw2 -r
```

- ```-s```
sort the listed processes by sid
```
./hw2 -s
```

> if you combile servel sort-option, it will only be effect by last one
