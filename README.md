# how to use
1. get the total dataset from https://www.nxcoredata.com/sample-nxcore-data/
2. get the .so file from the same page. Place in repo folder and make sure it is called 'libnx.so'
3. create a config.conf file and add this entry - ```dataset_path=/path/to/downloaded/dataset```
4. make
5. open two terminals in the repo root
6. first ./market_data_processor
7. then ./startegy_processor