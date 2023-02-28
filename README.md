## Driver for O'man TCP card for the Sinclair QL

### TODO
- [ ] qedrv: use alternate SSP during IOSS calls
- [ ] qedrv: allocate all memory from heap and store pointer in driver linkage block
- [ ] qedrv: implement enough functionality so that Jan Bredenbeek's QLTerm works
- [ ] tftp: add support for file size extension so that binaries can be loaded directly to memory
- [ ] tftp: add support for directly execing a remote file that has the XTcc definition in place

### Dependencies
* Docker
* qdos-devel Docker container


### Compiling
* Pull the code and change into the code directory
* Run build.sh
