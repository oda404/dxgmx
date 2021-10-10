# The dxgmx logging system
The dxgmx logging system is level based, meaning that if a certain log level is set, be it at build or run time, only it and any other levels who'se values are less than it will actually get logged.

## Levels
Below is a list of all the valid log levels in dxgmx:
- **0**: Quiet
- **1**: Fatal
- **2**: Error
- **3**: Warning
- **4**: Info