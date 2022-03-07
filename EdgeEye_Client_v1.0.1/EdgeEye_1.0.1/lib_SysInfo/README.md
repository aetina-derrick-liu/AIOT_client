# lib_SysInfo
## Prerequisites
### Linux
None.
### Windows
#### libraries:
> - **libcpu** (https://192.168.168.57/svn/STSD_SW/trunk/library/libcpu)
>   - cpudisk.dll
>   - libcpu.dll
>- **EAPI** (https://192.168.168.57/svn/STSD_SW/trunk/library/EAPI)
>   - EAPI_Library.dll
#### How to use lib_SysInfo:
1. Build *libcpu*.
2. Execute the following command to install the dependencies. Only need to be executed once when first use this library.
    ```
    $ make dep
    ```
3. Copy all dependent libraries to *libSysInfo/test*.
4. Go back to *libSysInfo/src* folder, and execute the following command.
    ```
    $ make
    ```
    Unit test will be done after successful compilation