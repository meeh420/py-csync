from distutils.core import setup, Extension

#VERSION = "0.5.0-alpha"
# @see
# https://bitbucket.org/marcusva/py-sdl2/src/8f3bb8448f373a3abcfa4d6429fba6caaaeb60cd/setup.py?at=default

# @todo mark/check required python version. (runtime check?)
#       (need version 2.4 for Py_RETURN_NONE)

setup (name        = 'py-csync',
       version     = '0.50.0-alpha', # csync version -- pycsync patch-level
       description = 'blah',
       ext_modules =
       [
           Extension ('csync',
                      sources   = ['csyncmodule.c'],
                      libraries = ['csync'],
           ),
       ]
)
