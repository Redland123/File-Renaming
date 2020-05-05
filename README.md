# File-Renaming
Quick multi-threaded program which renames all files contained within a provided directory.
 
## Notes
 
-Files will be renamed with the following format: "user-entered-prefix" + "A number within the range of [0 - (total number of files in the directory)]". <br />
e.g.: <br />
newFilePrefix0 <br />
newFilePrefix1 <br />
newFilePrefix2 <br />
...
 
-While folders inside the provided directory will be renamed, the contents of said folder will not.
 
## Warning
 
-Using this program can result in file loss due to file naming conflicts. The program will attempt to resolve these conflicts before they happen but its safety is not guaranteed.
