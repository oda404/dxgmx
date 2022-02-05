# The dxgmx logging system
The dxgmx logging system is level based, meaning that if a certain log level is set, be it at build or run time, only it and any other levels who'se values are less than it will actually get logged.

## Levels
Below is a list of all the valid log levels in dxgmx:
- **0**: QUIET
- **1**: FATAL
- **2**: ERR
- **3**: WARN
- **4**: INFO
- **5**: DEBUG

Note that when setting a log level with the DXGMX_CONFIG_LOG_LEVEL option either the number or the name of the log level can be used.