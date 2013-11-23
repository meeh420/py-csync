from distutils.core import setup, Extension

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
