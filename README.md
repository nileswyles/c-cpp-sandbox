# Build Instructions

This project (mostly) uses shell scripts for building and executing programs. The scripts are located under the <i>build_scripts</i> directory. Development container configuration files and dependency installation scripts are located under the <i>.devcontainer</i> directory. 

## To run individual programs and tests: 
 ```./build_scripts/<build_script>.sh```

Usage:
```
	-l|--log        Log level.
	-D              Preprocessor Defines.

	*               Positional arguments after are passed as program arguments and are specific to the program.
```

The following example runs the http_server application:

 ```./build_scripts/tests/http_server.sh```

The following example executes the iostream::testReadUntil test with log level 4 and disables logging in the reader task module:

 ```./build_scripts/tests/iostream_test.sh -l 4 -D LOGGER_READER_TASK=0 testReadUntil```

 To build programs that require the google cloud sdk, you must first run build_google.sh to build and install the sdk. The install directory will include a script called `env.sh` which is used to intialize the shell environment for subsequent builds.

## To run all tests: 
```./build_scripts/tests.pl```

Usage:
```
	-p|--path       Path to tests directory. Default to build_scripts/tests.
	-l|--log        Log level.
	-D              Preprocessor Defines.
```

The following example runs the http_server application:

```./build_scripts/tests.pl -l 4```
