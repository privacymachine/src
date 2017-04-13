#!/bin/bash
# Helper functions

# detect operating system
set OS=Linux
case "$(uname -s)" in
  Linux)
    OS=Linux
    ;;
  CYGWIN*|MINGW*|MSYS*)
    OS=Windows
    ;;
  *)
    echo 'unknown OS' 
    return 1
    ;;
esac

