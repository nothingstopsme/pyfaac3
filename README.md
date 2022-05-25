# pyfaac3
An extension module providing access to the native library "faac" from a python3 environment.

## Acknowledgements
This project is inspired by [pyfaac](https://github.com/gleero/pyfaac), which offers similiar functionalities but with a different implementation approach. It has served as an important reference for me in learning the usage of libfaac and the mechanism of python extensions; therefore great credit to its author.

## Features
1. Native encoder handles are mapped to python instances, so the use of multiple encoders with different configurations simultanenously in a single process is viable.
2. Encoder instances can be context-managed, i.e., used in the with statement.
3. All settings (except for those related to psychoacoustic modelling) are configurable.
4. All setting values natively defined as C macro are exposed to python as module constants.
5. The final flushing operations (which notifies a native encoder that there is no further input, so that it starts to flush/return the audio frames cached internally) are explicitly defined as calls to flush(), rather than to encode() with an empty buffer (An design choice borrowed from [pyfaac](https://github.com/gleero/pyfaac)).
6. Following feature 3, calling encode() with an empty buffer is repurposed as encoding a fully silent frame (a audio frame in which the value of all samples is 0). This is consistent with the behaviour of faac when encoding an incomplete frame in which the missing samples are treated as of 0 amplitude.


## Prerequisites
The following programs/packages are required for building this module:
* C compiler
* libfaac-dev
* python3-dev

On a debian-based linux OS, they can be acquired via
```Bash
sudo apt install gcc
sudo apt install libfaac-dev
sudo apt install python3-dev
```

## Building/Installation
It is recommended to build and test it first without installation:
```Bash
python3 setup.py build_ext
```
The command above generats the binary file of this module inside the folder "build/lib.*SOME_SYSTEM_PLATFORM_VERSION_SUFFIX*/". That binary can be imported and used as a local module in your python project.

Once you are certain about putting it into your system package reservoir,
```Bash
python3 setup.py install
```
to install this module. Note that using pip to manage such a third-party package might be a better choice, since if you change your mind later, it is much easier to perform uninstallation. Please refer to [this document](https://packaging.python.org/en/latest/guides/distributing-packages-using-setuptools/#wheels) for more information about how you can achieve that.


## Usage
Please check [this script](./test.py) containing the code used to conduct a simple unit test; it should present a basic idea of how to use this module.

You can also find a full list of classes/functions/attributes/constants available in this module, as well as their description, in the module's python document:
```Python
# in a python3 shell
>>> import pyfaac3
>>> help(pyfaac3)
```

## Notes on faac
It is worth mentioning that faac maintains an internal buffer for encoding, and faacEncEncode() will not return any result (returning 0 bytes in fact) until that buffer is filled up.
Due to that behaviour, there is actually a delay in producing encoded data, and this is also why flushing operations are needed after all audio samples have been submitted through faacEncEncode(), in order to make up the delay. 

The presence of delay might not matter for pure pcm-to-aac conversion, but it becomes an issue when each generated aac frame is directly involved in a synchronisation process with other media; in that case, special care should be taken to correct the time discrepancy caused by the delay.









