from distutils.core import setup, Extension

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
