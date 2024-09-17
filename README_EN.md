# Simple utility for read/write data from SIMATIC S7 PLC

Application can be used for reading and writing values to/from DB area of S7 PLC.
App use Ethernet connection to PLC, whith S7 protocol. Use [open library Snap7](https://snap7.sourceforge.net/) to connect.  

You need Qt6.7 framework to build it. 

App can be used for simple batch save/restore, debugging, comissioning, saving structured values, making collection of preferencies etc.

Parameters to read/write are configured in text file with simple formatting.

To connect to the PLC you need only address of the PLC and information of data adresses in Data Blocks.

S7-300, -400, -1x500 series are supported. 
It can be used whith S7-1x00 series only with Standard access to Data Block. Option "Optimized data access" should be off for DB that would be read/write.

Data types BOOL, BYTE, WORD, DWORD, S5TIME, TIME, INT, DINT, REAL are supported.
![Application window](https://github.com/jkapter/S7_PLC_simple_DB_rw/blob/master/src/img/sysparams_e.png)
![Parameters file](https://github.com/jkapter/S7_PLC_simple_DB_rw/blob/master/src/img/sysparams_e.png)
