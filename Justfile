build-demo demo:
  #!/bin/sh
  gcc -o build/main demos/{{demo}}.c lwxgl/main.c -lX11

run:
  #!/bin/sh
  build/main

run-cros:
  #!/bin/sh
  sommelier -X --scale=0.8 sh -c "xset fp+ /usr/share/fonts/X11/misc && build/main"